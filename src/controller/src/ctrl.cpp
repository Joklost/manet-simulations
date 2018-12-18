
#include "ctrl.h"

void Controller::run() {
    mpi_init(&this->world_size, &this->world_rank, &this->name_len, this->processor_name);

    this->c = spdlog::stdout_color_st("ctrl");
    this->c->info("ctrl started, running with {} nodes.", this->world_size - 1);

    if (!this->handshake()) {
        mpi_deinit();
        return;
    }

    auto clock_worker = std::thread{&Controller::clock, this};
    auto mpi_worker = std::thread{&Controller::message_handler, this};

    mpi_worker.join();
    clock_worker.join();

    for (auto i = 1; i < world_size; ++i) {
        for (auto &p : this->packets[i]) {
            delete p.data;
        }
    }

    mpi_deinit();
}

void Controller::message_handler() {
    this->c->info("message_handler()");

    while (this->work) {

        Status status = mpi_probe_any();
        c->info("- probe(source={}, tag={})", status.source, status.tag);

        if (status.tag == DIE) {
            this->die(status);
        } else if (status.tag == TX_PKT) {
            this->handle_tx(status);
        } else if (status.tag == RX_PKT) {
            this->handle_rx(status);
        } else if (status.tag == SLEEP) {
            this->handle_sleep(status);
        } else if (status.tag == LOCATION) {
            this->update_location(status);
        }

        if (this->timeslot_complete(this->current_time)) {
            this->cond_.notify_one();
        }
    }

    this->c->info("message_handler() stopped...");
}

void Controller::clock() {
    this->c->info("clock()");

    while (true) {
        std::unique_lock<std::mutex> mlock(mutex_);
        this->cond_.wait(mlock);
        if (!this->work) {
            break;
        }
        auto this_time = this->current_time++;

        this->c->info("Timeslot {} ready to process", this_time);
        this->c->info("Nodes: {}", nodes.size());

        for (const auto &item : this->nodes) {
            const auto &node = item.second;

            if (node.state == sleeping) {
                if (node.time == this_time) {
                    /* Send wake_up message. */
                } else {
                    continue;
                }
            } else if (node.state == transmitting) {
                this->packets[node.rank];
            } else if (node.state == listening) {

            }
        }
    }

    this->c->info("clock() stopped...");
}


bool Controller::handshake() {
    for (auto i = 1; i < world_size; ++i) {
        c->info("Handshaking with {}{}", processor_name, i);
        mpi_send(i, i, HANDSHAKE);
        int response;
        mpi_recv(&response, i, HANDSHAKE);
        if (response == i) {
            c->info("- Handshaking with {}{} successful", processor_name, i);

            Status status = mpi_probe(i, LOCATION);
            if (status.tag == LOCATION) {
                update_location(status);
            }
        } else {
            c->info("- Handshaking with {}{} failed", processor_name, i);
            return false;
        }
    }

    return true;
}

void Controller::die(Status &status) {
    int node;
    mpi_recv(&node, status.source, DIE);
    this->c->info("-- die(source={}{})", processor_name, node);

    dies++;
    if (dies == world_size) {
        this->work = false;
        this->cond_.notify_one();
    }
}

void Controller::handle_tx(const Status &status) {
    this->c->info("-- handle_tx(source={})", status.source);

    auto packet = new std::vector<octet>{};
    mpi_recv(packet, status.source, TX_PKT);

    this->packets[status.source].push_back({this->current_time, packet});
    this->c->info("--- <{} rx packet of size {}", status.source, packet->size());
    log_packet(this->c, packet);

    this->nodes[status.source].time++;
    this->nodes[status.source].state = transmitting;
}

void Controller::handle_rx(const Status &status) {
    this->c->info("-- handle_rx(source={})", status.source);

    unsigned long time_units;
    mpi_recv(&time_units, status.source, RX_PKT);

    this->nodes[status.source].time += time_units;
    this->nodes[status.source].state = listening;

    //unsigned long packet_count = this->packets[status.source].size();
    //this->c->info("--- source listening for {} time units, responding with {} packets",
    //              time_units, packet_count);
    //mpi_send(packet_count, status.source, RX_PKT);

    //for (auto &packet : this->packets[status.source]) {
    //    this->c->info("--- {}> tx packet of size {}", status.source, packet.data->size());
    //    log_packet(this->c, packet.data);

    //    mpi_send(packet.data, status.source, RX_PKT);
    //}
}

void Controller::update_location(const Status &status) {
    this->c->info("-- update_location(source={})", status.source);

    std::vector<octet> buffer{};
    mpi_recv(buffer, status.source, LOCATION);
    auto loc = deserialise<Location>(buffer);

    if (this->nodes.find(status.source) != this->nodes.end()) {
        this->nodes[status.source].loc = loc;
    } else {
        this->nodes.insert({status.source, {status.source, loc}});
    }

    this->c->info("---{}", this->nodes[status.source].loc);
}

void Controller::handle_sleep(const Status &status) {
    this->c->info("-- handle_sleep(source={})", status.source);

    unsigned long time_units;
    mpi_recv(&time_units, status.source, RX_PKT);

    this->nodes[status.source].time += time_units;
    this->nodes[status.source].state = sleeping;
}

bool Controller::timeslot_complete(const unsigned long time) {
    for (const auto &node : this->nodes) {
        if (node.second.time <= time) {
            return false;
        }
    }

    return true;
}
