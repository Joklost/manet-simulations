
#include <random>

#include <reachi/radiomodel.h>

#include "ctrlr.h"

void Controller::run() {
    mpi::init(&this->world_size, &this->world_rank, &this->name_len, this->processor_name);
    this->world_size = this->world_size - 2;
    this->lmc_node = this->world_size + 1;

    this->c = spdlog::stdout_color_mt("ctrlr");
    if (this->debug) {
        this->c->set_level(spdlog::level::debug);
    }

    this->c->debug("init(nodes={})", this->world_size);

    /* Ensure that the control worker is ready before using MPI. */
    auto control_worker = std::thread{&Controller::control, this};

    if (!this->handshake()) {
        this->work = false;
        this->queue.push({poison_t});

        mpi::deinit();
        return;
    }

    auto receiver = std::thread{&Controller::recv, this};

    receiver.join();
    control_worker.join();

    this->c->debug("deinit()");

    mpi::deinit();
}

void Controller::recv() {
    while (this->work) {
        auto status = mpi::probe();

        if (status.tag == DIE) {
            auto localtime = mpi::recv<unsigned long>(status.source, DIE);
            this->c->debug("die(source={}, tag={}, localtime={})", status.source, status.tag, localtime);

            this->nodes[status.source].localtime = std::numeric_limits<unsigned long>::max() - 1ul;
            this->nodes[status.source].dead = true;
            this->queue.push({set_time_t, status.source, this->nodes[status.source].localtime});

            auto all_dead = true;
            for (const auto &node : this->nodes) {
                if (!node.second.dead) {
                    all_dead = false;
                    break;
                }
            }

            if (all_dead) {
                /* Send poison pill to controller queue to gracefully shut down. */
                this->queue.push({poison_t});
                break;
            }
        }

        if (status.tag == TX_PKT) {
            auto localtime = mpi::recv<unsigned long>(status.source, TX_PKT);
            auto packet = mpi::recv<std::vector<octet>>(status.source, TX_PKT_DATA);
            auto duration = static_cast<unsigned long>(mpilib::transmission_time(BAUDRATE, packet.size()).count());
            this->queue.push({transmit_t, status.source, localtime, duration, packet});
        }

        if (status.tag == RX_PKT) {
            auto localtime = mpi::recv<unsigned long>(status.source, RX_PKT);
            auto duration = mpi::recv<unsigned long>(status.source, RX_PKT_DURATION);
            this->queue.push({listen_t, status.source, localtime, duration});
        }

        if (status.tag == SLEEP) {
            auto localtime = mpi::recv<unsigned long>(status.source, SLEEP);
            auto duration = mpi::recv<unsigned long>(status.source, SLEEP_DURATION);
            this->queue.push({sleep_t, status.source, localtime, duration});
        }

        if (status.tag == SET_LOCATION) {
            auto location = update_location(status);
            this->queue.push({update_location_t, status.source, 0ul, 0ul, mpilib::serialise(location)});
        }

        if (status.tag == SET_LOCAL_TIME) {
            auto localtime = mpi::recv<unsigned long>(status.source, SET_LOCAL_TIME);
            this->c->debug("register_localtime(source={}, tag={}, localtime={})", status.source, status.tag, localtime);
            this->queue.push({set_time_t, status.source, localtime});
        }

        if (status.tag == LINK_MODEL) {
            auto count = mpi::recv<unsigned long>(status.source, LINK_MODEL);
            std::vector<mpilib::Link> link_model{};
            link_model.reserve(count);
            for (auto i = 0; i < count; ++i) {
                auto link_buffer = mpi::recv<std::vector<octet>>(status.source, LINK_MODEL_LINK);
                link_model.push_back(mpilib::deserialise<mpilib::Link>(link_buffer));
            }

            Action act{link_model_t, status.source};
            act.link_model = link_model;
            this->queue.push(act);
        }

    }
}

void Controller::control() {
    std::random_device rd;
    std::mt19937 gen(rd());

    while (this->work) {
        auto act = this->queue.pop();

        if (act.type == poison_t) {
            mpi::send(DIE, this->lmc_node, DIE);
            break;
        }

        if (act.type == transmit_t) {
            this->c->debug("process_tx(rank={}, localtime={}, duration={}, size={})",
                           act.rank, act.localtime, act.duration, act.data.size());
            this->nodes[act.rank].localtime = act.localtime + act.duration;
            this->transmit_actions.push_back({act.localtime, act.localtime + act.duration, act.rank, act.data});
        }

        if (act.type == listen_t) {
            this->nodes[act.rank].localtime = act.localtime + act.duration;
            this->listen_actions.push_back({act.localtime, act.localtime + act.duration, act.rank});
        }

        if (act.type == sleep_t) {
            //this->c->debug("process_sleep(rank={}, localtime={}, duration={})", act.rank, act.localtime, act.duration);
            this->nodes[act.rank].localtime = act.localtime + act.duration;
        }

        if (act.type == set_time_t) {
            this->nodes[act.rank].localtime = act.localtime;
        }

        if (act.type == update_location_t) {
            mpi::send(act.rank, this->lmc_node, UPDATE_LOCATION);
            mpi::send(act.data, this->lmc_node, UPDATE_LOCATION_DATA);
        }

        if (act.type == link_model_t) {
            auto links = act.link_model.size();
            this->c->debug("update_linkmodel(links={})", links);

            std::vector<std::vector<double>> new_link_model{};
            new_link_model.resize(links, std::vector<double>(links));

            for (auto &item : act.link_model) {
                new_link_model[item.first][item.second] = item.pathloss;
                new_link_model[item.second][item.first] = item.pathloss;
            }
            this->link_model = std::move(new_link_model);
        }


        if (this->transmit_actions.empty() && this->listen_actions.empty()) {
            continue;
        }

        unsigned long ctrlr_time = std::numeric_limits<unsigned long>::max();
        for (const auto &item : this->nodes) {
            auto &node = item.second;
            if (node.localtime < ctrlr_time) {
                ctrlr_time = node.localtime;
            }
        }

        if (ctrlr_time == std::numeric_limits<unsigned long>::max()) {
            continue;
        }

        /* Remove processed transmission and listen actions. */
        this->listen_actions.erase(
                std::remove_if(
                        this->listen_actions.begin(), this->listen_actions.end(),
                        [](const Listen &l) -> bool {
                            return l.is_processed;
                        }
                ), this->listen_actions.end()
        );

        for (auto &rx : this->listen_actions) {
            /* We check if we have received an action/time update from all other nodes. */
            if (rx.end > ctrlr_time || rx.is_processed) {
                continue;
            }

            std::vector<std::vector<octet>> packets{};

            /* We can process this listen action. */
            for (auto &tx : this->transmit_actions) {
                if (rx.rank == tx.rank || !tx.is_within(rx)) {
                    continue;
                }

                auto tx_power = 26.0;
                std::vector<double> interference{};
                auto pathloss = this->link_model[rx.rank][tx.rank];

                if (mpilib::is_equal(pathloss, 0.0)) {
                    this->c->debug("should_receive(first={}, second={}, status=no_link)", tx.rank, rx.rank);
                    continue;
                }

                for (auto &tx_inner : this->transmit_actions) {
                    if (tx.rank == tx_inner.rank)  {
                        continue;
                    }

                    /* If tx intervals don't intersect. */
                    if (tx.end > tx_inner.start && tx.start < tx_inner.end) {
                        interference.push_back(tx_power - this->link_model[rx.rank][tx_inner.rank]);
                    }
                }

                auto rssi = tx_power - pathloss;
                auto pep = reachi::radiomodel::pep(rssi, tx.data.size(), interference);

                std::bernoulli_distribution d(1.0 - pep);
                auto should_receive = d(gen);

                this->c->debug("should_receive(first={}, second={}, status={}, pep={}, rssi={}, interference={})",
                               tx.rank, rx.rank, should_receive, pep, rssi, interference.size());

                if (should_receive) {
                    packets.emplace_back(tx.data);
                }
            }

            mpi::send(packets.size(), rx.rank, RX_PKT_COUNT);

            for (auto &packet : packets) {
                mpi::send(packet, rx.rank, RX_PKT_DATA);
            }

            rx.is_processed = true;
        }

        /* Remove processed transmission actions. */
        /*
        this->transmit_actions.erase(
                std::remove_if(
                        this->transmit_actions.begin(), this->transmit_actions.end(),
                        [&ctrlr_time](const Transmission &t) -> bool {
                            return t.end < ctrlr_time;
                        }
                ), this->transmit_actions.end()
        );
         */
    }
}


bool Controller::handshake() {
    mpi::send(this->lmc_node, this->lmc_node, HANDSHAKE);

    for (auto i = 1; i <= world_size; ++i) {
        auto node_name = mpilib::processor_name(this->processor_name, i);
        mpi::send(i, i, HANDSHAKE);
        auto response = mpi::recv<int>(i, HANDSHAKE);
        if (response == i) {
            this->nodes.insert({i, {i}});
            this->c->debug("handshake(target={}, status=success)", node_name);

            mpi::Status status = mpi::probe(i, SET_LOCATION);
            update_location(status);
            auto node_buffer = mpilib::serialise(this->nodes[i]);
            mpi::send(node_buffer, this->lmc_node, NODE_INFO);

        } else {
            this->c->debug("handshake(target={}, status=fail)", node_name);
            return false;
        }
    }

    auto response = mpi::recv<int>(this->lmc_node, HANDSHAKE);

    if (response != this->lmc_node) {
        this->c->debug("handshake(target=lmc, status=fail)");
        return false;
    } else {
        this->c->debug("handshake(target=lmc, status=success)");
    }

    auto count = mpi::recv<unsigned long>(this->lmc_node, LINK_MODEL);
    std::vector<mpilib::Link> link_model{};
    link_model.reserve(count);
    for (auto j = 0; j < count; ++j) {
        auto link_buffer = mpi::recv<std::vector<octet>>(this->lmc_node, LINK_MODEL_LINK);
        link_model.push_back(mpilib::deserialise<mpilib::Link>(link_buffer));
    }

    auto links = link_model.size();
    this->c->debug("set_linkmodel(links={})", link_model.size());

    std::vector<std::vector<double>> new_link_model{};
    new_link_model.resize(links, std::vector<double>(links));

    for (auto &item : link_model) {
        new_link_model[item.first][item.second] = item.pathloss;
        new_link_model[item.second][item.first] = item.pathloss;
    }
    this->link_model = std::move(new_link_model);

    /* Set the clocks on all nodes. */
    for (auto i = 1; i <= world_size; ++i) {
        mpi::send(i, i, READY);
    }

    return true;
}

mpilib::geo::Location Controller::update_location(const mpi::Status &status) {
    auto buffer = mpi::recv<std::vector<octet>>(status.source, SET_LOCATION);
    auto loc = mpilib::deserialise<mpilib::geo::Location>(buffer);

    this->c->debug("update_location(source={}, loc={})", status.source, loc);
    this->nodes[status.source].loc = loc;
    return loc;
}

bool Transmission::is_within(Listen listen) {
    return this->start >= listen.start &&
           this->end <= listen.end;
}
