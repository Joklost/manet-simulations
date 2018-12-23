
#include "ctrlr.h"

void Controller::run() {
    mpi::init(&this->world_size, &this->world_rank, &this->name_len, this->processor_name);
    this->world_size = this->world_size - 2;
    this->lmc_node = this->world_size + 1;
    this->c = spdlog::stdout_color_mt("ctrlr");
    if (this->debug) {
        this->c->set_level(spdlog::level::debug);
    }
    this->c->debug("started, running with {} nodes", this->world_size);

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

        for (auto &listen : this->listen_actions) {
            /* We check if we have received an action/time update from all other nodes. */
            if (listen.start + listen.duration > min_localtime || listen.is_processed) {
                continue;
            }

            std::vector<std::vector<octet>> packets{};

            /* we can process this listen action. */
            for (auto &transmission : this->transmission_actions) {
                if (listen.rank == transmission.rank) {
                    continue;
                }

                if (transmission.is_within(listen)) {
                    /* TODO: Ask linkmodel if message should be received. */

                    if (transmission.rank + 1 == listen.rank || transmission.rank - 1 == listen.rank) {
                        packets.emplace_back(transmission.data);
                    }
                }
            }

            this->c->debug("process_rx(rank={}, localtime={}, duration={}, packets={})",
                           act.rank, act.localtime, act.duration, packets.size());

            mpi::send(packets.size(), listen.rank, RX_PKT_COUNT);

            for (auto &packet : packets) {
                mpi::send(packet, listen.rank, RX_PKT_DATA);
            }

            mpi::send(listen.start + listen.duration, listen.rank, RX_PKT_ACK);
            listen.is_processed = true;
        }

        /* Remove processed actions. */
        //this->listen_actions.erase(std::remove_if(this->listen_actions.begin(), this->listen_actions.end(),
        //                                          [](ListenAction &action) -> bool { return action.processed; }));
    }
}


bool Controller::handshake() {
    mpi::send(this->lmc_node, this->lmc_node, HANDSHAKE);
    auto response = mpi::recv<int>(this->lmc_node, HANDSHAKE);

    if (response != this->lmc_node) {
        this->c->debug("handshake(target=lmc, status=fail)");
        return false;
    } else {
        this->c->debug("handshake(target=lmc, status=success)");
    }

    for (auto i = 1; i <= world_size; ++i) {
        auto node_name = mpilib::processor_name(this->processor_name, i);
        mpi::send(i, i, HANDSHAKE);
        response = mpi::recv<int>(i, HANDSHAKE);
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

    return true;
}

mpilib::geo::Location Controller::update_location(const mpi::Status &status) {
    auto buffer = mpi::recv<std::vector<octet>>(status.source, SET_LOCATION);
    auto loc = mpilib::deserialise<mpilib::geo::Location>(buffer);

    this->c->debug("update_location(source={}, loc={})", status.source, loc);
    this->nodes[status.source].loc = loc;
    return loc;
}

bool TransmissionAction::is_within(ListenAction listen) {
    return this->start >= listen.start &&
           (this->start + this->duration) <= (listen.start + listen.duration);
}
