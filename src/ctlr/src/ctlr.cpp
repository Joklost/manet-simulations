
#include "ctlr.h"

void Controller::run() {
    mpi::init(&this->world_size, &this->world_rank, &this->name_len, this->processor_name);
    this->world_size = this->world_size - 1; /* TODO: Set to 2 when running with LMC */
    this->c = spdlog::stdout_color_mt("ctrl");
    if (this->debug) {
        this->c->set_level(spdlog::level::debug);
    }
    this->c->debug("ctrl started, running with {} nodes.", this->world_size);

    /* Ensure that the clock worker is ready before using MPI. */
    //auto clock_worker = std::thread{&Controller::clock, this};
    auto control_worker = std::thread{&Controller::control, this};

    if (!this->handshake()) {
        this->work = false;
        this->cond_.notify_one();
        //clock_worker.join();

        mpi::deinit();
        return;
    }

    auto receiver = std::thread{&Controller::recv, this};
    //auto mpi_worker = std::thread{&Controller::message_handler, this};

    //mpi_worker.join();
    receiver.join();
    control_worker.join();

    for (auto i = 1; i < world_size; ++i) {
        for (auto &p : this->packets) {
            delete p.second.data;
        }
    }

    mpi::deinit();
}

void Controller::clock() {
    this->c->debug("clock()");

    while (true) {
        std::unique_lock<std::mutex> mlock(mutex_);
        this->cond_.wait(mlock);
        if (!this->work) {
            break;
        }
        auto this_time = ++this->current_time;

        this->c->debug("\e[92m+\e[39m clock(time={})", this_time);
        for (auto &item : this->nodes) {
            auto &node = item.second;

            if (node.state == sleeping) {
                this->c->debug("\e[92m++\e[39m process(node={}, time={}, state=sleeping)", node.rank, node.localtime);
                if (node.localtime == this_time) {
                    /* Send wake up message. */
                    mpi::send(node.localtime, node.rank, SLEEP_ACK);
                    this->c->debug("\e[92m+++\e[39m wakeup(node={}, time={})", node.rank, node.localtime);
                }
            } else if (node.state == transmitting) {
                this->c->debug("\e[92m++\e[39m process(node={}, time={}, state=transmitting)", node.rank,
                               node.localtime);
                /* Get packet sent by node. */
                auto p = this->packets[node.rank];

                this->c->debug("\e[92m+++\e[39m sort_packet(size={}):", p.data->size());
                mpilib::log_packet(this->c, "\e[92m+++\e[39m", p.data);

                /* Add copy of data to neighbouring nodes. */
                for (auto &prospect : this->nodes) {
                    auto &pnode = prospect.second;

                    if (node == pnode or pnode.state != listening) {
                        /* Don't send to itself */
                        continue;
                    }

                    /* TODO: Include link model and neighbourhoods */
                    if (/*reachable or neighbour*/true) {
                        this->c->debug("\e[92m+++\e[39m add_to_neighbour(node={})", pnode.rank);
                        pnode.packets.emplace_back(p.data->begin(), p.data->end());
                    }
                }

                /* Cleanup */
                delete p.data;
                this->packets.erase(node.rank);

                mpi::send(node.localtime, node.rank, TX_PKT_ACK);
                this->c->debug("\e[92m+++\e[39m send_ack_tx(node={})", node.rank);
            }
        }

        for (auto &item : nodes) {
            auto &node = item.second;

            if (node.state == listening) {
                if (node.localtime == this_time) {
                    this->c->debug("\e[92m++\e[39m process(node={}, time={}, state=listening)", node.rank,
                                   node.localtime);
                    /* Send and empty packet vector. */

                    auto packet_count = node.packets.size();
                    mpi::send(packet_count, node.rank, RX_PKT_COUNT);

                    for (auto &packet : node.packets) {
                        this->c->debug("\e[92m+++\e[39m tx_packet(node={}, size={}):", node.rank, packet.size());
                        mpilib::log_packet(this->c, "\e[92m+++\e[39m", packet);
                        mpi::send(packet, node.rank, RX_PKT);
                    }

                    node.packets.clear();
                    mpi::send(node.localtime, node.rank, RX_PKT_ACK);
                    this->c->debug("\e[92m+++\e[39m send_ack_rx(node={})", node.rank);
                }
            }
        }
    }

    this->c->debug("clock() stopped...");
}


void Controller::recv() {
    while (this->work) {
        auto status = mpi::probe();
        this->c->debug("probe(source={}, tag={})", status.source, status.tag);

        if (status.tag == DIE) {
            auto localtime = mpi::recv<unsigned long>(status.source, DIE);

            this->nodes[status.source].localtime = localtime;
            this->nodes[status.source].dead = true;

            auto all_dead = true;
            for (const auto &node : this->nodes) {
                if (!node.second.dead) {
                    all_dead = false;
                    break;
                }
            }

            if (all_dead) {
                /* Send poison pill to controller queue to gracefully shut down. */
                this->work = false;
                this->queue.push({poison_t});
            }
        }

        if (status.tag == TX_PKT) {
            auto localtime = mpi::recv<unsigned long>(status.source, TX_PKT);
            auto packet = mpi::recv<std::vector<octet>>(status.source, TX_PKT_DATA);
            mpilib::log_packet(this->c, "", packet);
            this->queue.push({transmit_t, status.source, localtime, 11ul, packet});
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
            update_location(status);
        }

        if (status.tag == SET_LOCAL_TIME) {
            auto localtime = mpi::recv<unsigned long>(status.source, SET_LOCAL_TIME);
            this->nodes[status.source].localtime = localtime;
        }

    }
}


void Controller::message_handler() {
    this->c->debug("message_handler()");

    while (this->work) {

        mpi::Status status = mpi::probe();
        this->c->debug("\e[96m-\e[39m probe(source={}, tag={})", status.source, status.tag);

        if (status.tag == DIE) {
            int node;
            mpi::recv(&node, status.source, DIE);
            this->c->debug("\e[96m--\e[39m die(source={}{})", processor_name, node);

            this->dies++;
            if (this->dies == world_size) {
                this->work = false;
                this->cond_.notify_one();
            } else {
                /* Make sure that the controller can continue, even if a node will disappear. */
                if (node == status.source) {
                    this->nodes.erase(node);

                    if (this->timeslot_complete(this->current_time)) {
                        this->c->debug("\e[96m---\e[39m notify(time={})", this->current_time);
                        this->cond_.notify_one();
                    }
                }
            }

        } else if (status.tag == TX_PKT) {
            this->c->debug("\e[96m--\e[39m handle_tx(source={})", status.source);

            auto packet = new std::vector<octet>{};
            mpi::recv(packet, status.source, TX_PKT);

            this->packets[status.source] = {this->current_time, packet};
            //this->packets[status.source].push_back({this->current_time, packet});
            this->c->debug("\e[96m---\e[39m rx_packet(node={}, size={}):", status.source, packet->size());
            mpilib::log_packet(this->c, "\e[96m---\e[39m", packet);

            this->nodes[status.source].localtime += 1;
            this->nodes[status.source].state = transmitting;

        } else if (status.tag == RX_PKT) {
            this->c->debug("\e[96m--\e[39m handle_rx(source={})", status.source);

            unsigned long time_units;
            mpi::recv(&time_units, status.source, RX_PKT);

            this->nodes[status.source].localtime += time_units;
            this->nodes[status.source].state = listening;

        } else if (status.tag == SLEEP) {
            unsigned long time_units;
            mpi::recv(&time_units, status.source, SLEEP);

            this->c->debug("\e[96m--\e[39m handle_sleep(source={}, time={})", status.source, time_units);

            this->nodes[status.source].localtime += time_units;
            this->nodes[status.source].state = sleeping;

        } else if (status.tag == LOCATION) {
            update_location(status);

        } else if (status.tag == LOCAL_TIME_REQ) {
            unsigned long id;
            mpi::recv(&id, status.source, LOCAL_TIME_REQ);

            mpi::send(this->nodes[id].localtime, status.source, LOCAL_TIME_RSP);
        } else if (status.tag == WORLD_SIZE_REQ) {
            unsigned long id;
            mpi::recv(&id, status.source, WORLD_SIZE_REQ);

            mpi::send(this->nodes.size(), status.source, WORLD_SIZE_RSP);
        }

        if (this->timeslot_complete(this->current_time)) {
            this->c->debug("\e[96m-\e[39m notify(time={})", this->current_time);
            this->cond_.notify_one();
        }
    }

    this->c->debug("message_handler() stopped...");
}

void Controller::control() {
    while (this->work) {
        auto msg = this->queue.pop();

        if (msg.action == poison_t) {
            break;
        }

        if (msg.action == transmit_t) {
        }

        if (msg.action == listen_t) {
            //auto packet_count = 1ul;
            //mpi::send(packet_count, msg.rank, RX_PKT_COUNT);
            //mpi::send(msg.data, msg.rank, RX_PKT_DATA);
            //mpi::send(msg.localtime, msg.rank, RX_PKT_ACK);
        }

        if (msg.action == sleep_t) {

        }
    }
}


bool Controller::handshake() {
    for (auto i = 1; i <= world_size; ++i) {
        auto node_name =  mpilib::processor_name(this->processor_name, i);
        mpi::send(i, i, HANDSHAKE);
        auto response = mpi::recv<int>(i, HANDSHAKE);
        if (response == i) {
            this->nodes.insert({i, {i, node_name}});
            this->c->debug("handshake(target={}, status=success)", node_name);

            mpi::Status status = mpi::probe(i, SET_LOCATION);
            if (status.tag == SET_LOCATION) {
                update_location(status);
            }
        } else {
            this->c->debug("handshake(target={}, status=fail)", node_name);
            return false;
        }
    }

    return true;
}

void Controller::update_location(const mpi::Status &status) {
    auto buffer = mpi::recv<std::vector<octet>>(status.source, SET_LOCATION);
    auto loc = mpilib::deserialise<mpilib::geo::Location>(buffer);

    this->c->debug("update_location(source={}, loc={})", status.source, loc);
    this->nodes[status.source].loc = loc;
}

bool Controller::timeslot_complete(const unsigned long time) {
    for (const auto &node : this->nodes) {
        if (node.second.localtime <= time) {
            return false;
        }
    }

    return true;
}


bool Node::operator==(const Node &rhs) const {
    return rank == rhs.rank;
}

bool Node::operator!=(const Node &rhs) const {
    return !(rhs == *this);
}
