
#include "ctlr.h"

void Controller::run() {
    mpi::init(&this->world_size, &this->world_rank, &this->name_len, this->processor_name);

    this->c = spdlog::stdout_color_mt("ctrl");
    if (this->debug) {
        this->c->set_level(spdlog::level::debug);
    }
    this->c->debug("ctrl started, running with {} nodes.", this->world_size - 1);

    /* Ensure that the clock worker is ready before using MPI. */
    auto clock_worker = std::thread{&Controller::clock, this};

    if (!this->handshake()) {
        this->work = false;
        this->cond_.notify_one();
        clock_worker.join();

        mpi::deinit();
        return;
    }

    auto mpi_worker = std::thread{&Controller::message_handler, this};

    mpi_worker.join();
    clock_worker.join();

    for (auto i = 1; i < world_size; ++i) {
        for (auto &p : this->packets) {
            delete p.second.data;
        }
    }

    mpi::deinit();
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

            this->nodes[status.source].time += 1;
            this->nodes[status.source].state = transmitting;

        } else if (status.tag == RX_PKT) {
            this->c->debug("\e[96m--\e[39m handle_rx(source={})", status.source);

            unsigned long time_units;
            mpi::recv(&time_units, status.source, RX_PKT);

            this->nodes[status.source].time += time_units;
            this->nodes[status.source].state = listening;

        } else if (status.tag == SLEEP) {
            unsigned long time_units;
            mpi::recv(&time_units, status.source, SLEEP);

            this->c->debug("\e[96m--\e[39m handle_sleep(source={}, time={})", status.source, time_units);

            this->nodes[status.source].time += time_units;
            this->nodes[status.source].state = sleeping;

        } else if (status.tag == LOCATION) {
            update_location(status);

        } else if (status.tag == LOCAL_TIME_REQ) {
            unsigned long id;
            mpi::recv(&id, status.source, LOCAL_TIME_REQ);

            mpi::send(this->nodes[id].time, status.source, LOCAL_TIME_RSP);
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
                this->c->debug("\e[92m++\e[39m process(node={}, time={}, state=sleeping)", node.rank, node.time);
                if (node.time == this_time) {
                    /* Send wake up message. */
                    mpi::send(node.time, node.rank, SLEEP_ACK);
                    this->c->debug("\e[92m+++\e[39m wakeup(node={}, time={})", node.rank, node.time);
                }
            } else if (node.state == transmitting) {
                this->c->debug("\e[92m++\e[39m process(node={}, time={}, state=transmitting)", node.rank, node.time);
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

                mpi::send(node.time, node.rank, TX_PKT_ACK);
                this->c->debug("\e[92m+++\e[39m send_ack_tx(node={})", node.rank);
            }
        }

        for (auto &item : nodes) {
            auto &node = item.second;

            if (node.state == listening) {
                if (node.time == this_time) {
                    this->c->debug("\e[92m++\e[39m process(node={}, time={}, state=listening)", node.rank, node.time);
                    /* Send and empty packet vector. */

                    auto packet_count = node.packets.size();
                    mpi::send(packet_count, node.rank, RX_PKT_COUNT);

                    for (auto &packet : node.packets) {
                        this->c->debug("\e[92m+++\e[39m tx_packet(node={}, size={}):", node.rank, packet.size());
                        mpilib::log_packet(this->c, "\e[92m+++\e[39m", packet);
                        mpi::send(packet, node.rank, RX_PKT);
                    }

                    node.packets.clear();
                    mpi::send(node.time, node.rank, RX_PKT_ACK);
                    this->c->debug("\e[92m+++\e[39m send_ack_rx(node={})", node.rank);
                }
            }
        }
    }

    this->c->debug("clock() stopped...");
}


bool Controller::handshake() {
    for (auto i = 1; i < world_size; ++i) {
        this->c->debug("handshake(target={}{})", processor_name, i);
        mpi::send(i, i, HANDSHAKE);
        int response;
        mpi::recv(&response, i, HANDSHAKE);
        if (response == i) {
            this->c->debug("\e[96m-\e[39m handshake(target={}{}, status=success)", processor_name, i);

            mpi::Status status = mpi::probe(i, LOCATION);
            if (status.tag == LOCATION) {
                update_location(status);
            }
        } else {
            this->c->debug("\e[96m-\e[39m handshake(target={}{}, status=fail)", processor_name, i);
            return false;
        }
    }

    return true;
}

void Controller::update_location(const mpi::Status &status) {
    this->c->debug("\e[96m--\e[39m update_location(source={})", status.source);

    std::vector<octet> buffer{};
    mpi::recv(buffer, status.source, LOCATION);
    auto loc = mpilib::deserialise<mpilib::geo::Location>(buffer);

    if (this->nodes.find(status.source) != this->nodes.end()) {
        this->nodes[status.source].loc = loc;
    } else {
        this->nodes.insert({status.source, {status.source, loc}});
    }

    this->c->debug("\e[96m---\e[39m{}", this->nodes[status.source].loc);
}

bool Controller::timeslot_complete(const unsigned long time) {
    for (const auto &node : this->nodes) {
        if (node.second.time <= time) {
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
