
#include <random>

#include <reachi/radiomodel.h>

#include "ctrlr.h"

void Controller::run() {
    mpi::init(&this->world_size, &this->world_rank, &this->name_len, this->processor_name);
    this->world_size = this->world_size - 2;
    this->lmc_node = this->world_size + 1;
    this->c = spdlog::basic_logger_mt("ctrlr", "logs/ctrlr.log");
    //this->c = spdlog::stdout_color_mt("ctrlr");
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
            auto duration = mpi::recv<unsigned long>(status.source, TX_PKT_DURATION);
            auto packet = mpi::recv<std::vector<octet>>(status.source, TX_PKT_DATA);
            this->c->debug("register_tx(source={}, tag={}, localtime={}, duration={}, size={})",
                           status.source, status.tag, localtime, duration, packet.size());
            this->nodes[status.source].localtime = localtime + duration;
            this->queue.push({transmit_t, status.source, localtime, duration, packet});
        }

        if (status.tag == RX_PKT) {
            auto localtime = mpi::recv<unsigned long>(status.source, RX_PKT);
            auto duration = mpi::recv<unsigned long>(status.source, RX_PKT_DURATION);
            this->c->debug("register_rx(source={}, tag={}, localtime={}, duration={})",
                           status.source, status.tag, localtime, duration);
            this->nodes[status.source].localtime = localtime + duration;
            this->queue.push({listen_t, status.source, localtime, duration});
        }

        if (status.tag == SLEEP) {
            auto localtime = mpi::recv<unsigned long>(status.source, SLEEP);
            auto duration = mpi::recv<unsigned long>(status.source, SLEEP_DURATION);
            this->c->debug("register_sleep(source={}, tag={}, localtime={}, duration={})",
                           status.source, status.tag, localtime, duration);
            this->nodes[status.source].localtime = localtime + duration;
            this->queue.push({sleep_t, status.source, localtime, duration});
        }

        if (status.tag == SET_LOCATION) {
            auto location = update_location(status);
            this->queue.push({update_location_t, status.source, 0ul, 0ul, mpilib::serialise(location)});
        }

        if (status.tag == SET_LOCAL_TIME) {
            auto localtime = mpi::recv<unsigned long>(status.source, SET_LOCAL_TIME);
            this->c->debug("register_localtime(source={}, tag={}, localtime={})", status.source, status.tag, localtime);
            this->nodes[status.source].localtime = localtime;
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
    while (this->work) {
        auto act = this->queue.pop();

        if (act.type == poison_t) {
            mpi::send(DIE, this->lmc_node, DIE);
            break;
        }

        if (act.type == transmit_t) {
            this->c->debug("process_tx(rank={}, localtime={}, duration={}, size={})",
                           act.rank, act.localtime, act.duration, act.data.size());
            this->transmission_actions.push_back({act.localtime, act.duration, act.rank, act.data});
            mpi::send(act.localtime + act.duration, act.rank, TX_PKT_ACK);
        }

        if (act.type == listen_t) {
            this->listen_actions.push_back({act.localtime, act.duration, act.rank});
        }

        if (act.type == sleep_t) {
            this->c->debug("process_sleep(rank={}, localtime={}, duration={})", act.rank, act.localtime, act.duration);
            mpi::send(act.localtime + act.duration, act.rank, SLEEP_ACK);
        }

        if (act.type == update_location_t) {
            mpi::send(act.rank, this->lmc_node, UPDATE_LOCATION);
            mpi::send(act.data, this->lmc_node, UPDATE_LOCATION_DATA);
        }

        if (act.type == link_model_t) {
            this->c->debug("update_linkmodel(links={})", act.link_model.size());
            this->link_model = act.link_model;
        }

        if (act.type == set_time_t) {
            /* Do nothing. */
        }


        if (this->transmission_actions.empty() && this->listen_actions.empty()) {
            continue;
        }

        unsigned long min_localtime = std::numeric_limits<unsigned long>::max();
        for (const auto &item : this->nodes) {
            auto &node = item.second;
            if (node.localtime < min_localtime) {
                min_localtime = node.localtime;
            }
        }

        if (min_localtime == std::numeric_limits<unsigned long>::max()) {
            continue;
        }

        for (auto &rx : this->listen_actions) {
            /* We check if we have received an action/time update from all other nodes. */
            if (rx.start + rx.duration > min_localtime || rx.is_processed) {
                continue;
            }

            std::vector<std::vector<octet>> packets{};

            /* we can process this listen action. */
            for (auto &tx : this->transmission_actions) {
                if (rx.rank == tx.rank || !tx.is_within(rx)) {
                    continue;
                }

                /* TODO: Compute interference. */
                auto tx_power = 1.0;
                auto interference = 0.0;

                auto it = std::find_if(this->link_model.begin(), this->link_model.end(), [&tx, &rx](auto link) {
                    return (tx.rank == link.first && rx.rank == link.second) ||
                           (tx.rank == link.second && rx.rank == link.first);
                });

                if (it == this->link_model.end()) {
                    this->c->debug("should_receive(first={}, second={}, status=no_link)", tx.rank, rx.rank);
                    continue;
                }

                auto rssi = tx_power - it.base()->pathloss;
                auto pep = reachi::radiomodel::pep(rssi, tx.data.size(), interference);

                std::random_device rd;
                std::mt19937 gen(rd());
                std::bernoulli_distribution d(1.0 - pep);
                auto should_receive = d(gen);

                this->c->debug("should_receive(first={}, second={}, status={}, rssi={}, pep={})",
                               tx.rank, rx.rank, should_receive, rssi, pep);

                if (should_receive) {
                    packets.emplace_back(tx.data);
                }
            }

            this->c->debug("process_rx(rank={}, localtime={}, duration={}, packets={})",
                           rx.rank, rx.start, rx.duration, packets.size());

            mpi::send(packets.size(), rx.rank, RX_PKT_COUNT);

            for (auto &packet : packets) {
                mpi::send(packet, rx.rank, RX_PKT_DATA);
            }

            mpi::send(rx.start + rx.duration, rx.rank, RX_PKT_ACK);
            rx.is_processed = true;
        }

        /* Remove processed actions. */
        //this->listen_actions.erase(std::remove_if(this->listen_actions.begin(), this->listen_actions.end(),
        //                                          [](Listen &action) -> bool { return action.processed; }));
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
    this->c->debug("set_linkmodel(links={})", link_model.size());
    this->link_model = link_model;

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
           (this->start + this->duration) <= (listen.start + listen.duration);
}
