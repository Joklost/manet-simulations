#include <common/equality.h>
#include <sims/radiomodel.h>
#include <geo/geo.h>

#include "coordinator.h"

#define TX_POWER 26.0

#ifndef THERMAL_NOISE
#define THERMAL_NOISE -119.66
#endif

#ifndef NOISE_FIGURE
#define NOISE_FIGURE 4.2
#endif

static bool has_actions(const std::pair<unsigned long, mpilib::Node> &item) {
    auto &node = item.second;
    return node.action_count > 0 || (node.action_count == 0 && node.dead);
}

void Coordinator::run() {
    mpi::init(&this->world_size, &this->world_rank, &this->name_len, this->processor_name);
    this->world_size = this->world_size - 2;  /* Coordinator and LMC */
    this->lmc_node = this->world_size + 1;

    this->c = spdlog::stdout_color_mt("coordr");
    if (this->debug) {
        this->c->set_level(spdlog::level::debug);
    }

    this->c->debug("init(nodes={})", this->world_size);

    if (!this->handshake()) {
        this->c->debug("deinit()");
        mpi::send(DIE, this->lmc_node, DIE);
        mpi::deinit();
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    while (true) {
        auto status = mpi::probe();
        if (this->process_message(status) != CSUCCESS) {
            break;
        }
        this->process_queue(gen);
    }

    this->c->debug("deinit()");
    mpi::send(DIE, this->lmc_node, DIE);
    mpi::deinit();
}


bool Coordinator::handshake() {
    mpi::send(this->lmc_node, this->lmc_node, HANDSHAKE);

    for (auto i = 1; i <= world_size; ++i) {
        auto node_name = mpilib::processor_name(this->processor_name, i);
        mpi::send(i, i, HANDSHAKE);
        auto response = mpi::recv<int>(i, HANDSHAKE);
        if (response == i) {
            this->nodes.insert({i, {i}});
            this->c->debug("handshake(target={}, status=success)", node_name);

            mpi::Status status = mpi::probe(i, SET_LOCATION);
            update_location(status.source);
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
    std::vector<mpilib::Link> links{};
    links.reserve(count);
    for (auto j = 0; j < count; ++j) {
        auto link_buffer = mpi::recv<std::vector<octet>>(this->lmc_node, LINK_MODEL_LINK);
        links.push_back(mpilib::deserialise<mpilib::Link>(link_buffer));
    }

    set_linkmodel(links);

    /* Sets the clock on all nodes. */
    for (auto i = 1; i <= world_size; ++i) {
        mpi::send(i, i, READY);
    }

    return true;
}

void Coordinator::set_linkmodel(std::vector<mpilib::Link> &links) {
    auto node_count = this->nodes.size() + 1;
    this->c->debug("set_linkmodel(links={})", links.size());

    this->link_model.resize(node_count, std::vector<double>(node_count));

    for (auto &link : links) {
        this->link_model[link.first][link.second] = link.pathloss;
        this->link_model[link.second][link.first] = link.pathloss;
    }
}

geo::Location Coordinator::update_location(const int rank) {
    auto buffer = mpi::recv<std::vector<octet>>(rank, SET_LOCATION);
    auto loc = mpilib::deserialise<geo::Location>(buffer);

    this->nodes[rank].loc = loc;
    //mpi::send(rank, this->lmc_node, UPDATE_LOCATION);
    //mpi::send(buffer, this->lmc_node, UPDATE_LOCATION_DATA);

    return loc;
}

int Coordinator::process_message(const mpi::Status &status) {
    if (status.tag == DIE) {
        auto localtime = mpi::recv<unsigned long>(status.source, DIE);
        this->c->debug("die(source={}, localtime={})", status.source, localtime);

        this->nodes[status.source].dead = true;
        this->dead_nodes += 1;

        if (this->dead_nodes == this->world_size) {
            /* Break the loop to deinit the MPI. */
            return CERROR;
        }
    }

    if (status.tag == TX_PKT) {
        Action act{Transmission, status.source};
        act.start = mpi::recv<unsigned long>(status.source, TX_PKT);
        auto data = mpi::recv<std::vector<octet>>(status.source, TX_PKT_DATA);
        act.end = act.start + mpilib::compute_transmission_time(BAUDRATE, data.size()).count();
        act.packet = {act.end, data};

        this->action_queue.push(act);
        this->transmissions[status.source].push_back(act);
        this->nodes[status.source].action_count += 1;
    }

    if (status.tag == RX_PKT) {
        Action act{Listen, status.source};
        act.start = mpi::recv<unsigned long>(status.source, RX_PKT);
        act.end = act.start + mpi::recv<unsigned long>(status.source, RX_PKT_DURATION);

        this->action_queue.push(act);
        this->nodes[status.source].action_count += 1;
    }

    if (status.tag == SLEEP) {
        Action act{Sleep, status.source};
        act.start = mpi::recv<unsigned long>(status.source, SLEEP);
        act.end = act.start + mpi::recv<unsigned long>(status.source, SLEEP_DURATION);

        this->action_queue.push(act);
        this->nodes[status.source].action_count += 1;
    }

    if (status.tag == INFORM) {
        Action act{Inform, status.source};
        act.start = act.end = mpi::recv<unsigned long>(status.source, INFORM);

        this->action_queue.push(act);
        this->nodes[status.source].action_count += 1;
    }

    if (status.tag == SET_LOCATION) {
        this->nodes[status.source].loc = update_location(status.source);
    }

    return CSUCCESS;
}

void Coordinator::process_queue(std::mt19937 &gen) {
    while (std::all_of(this->nodes.cbegin(), this->nodes.cend(), has_actions)) {
        auto act = this->action_queue.top(); /* Copy. */
        this->action_queue.pop();
        this->nodes[act.rank].action_count -= 1;

        if (act.type == Transmission) {
            auto &tx = act;

            /* Get interference from other transmitters. */
            std::vector<double> interference{};
            for (auto &item : this->transmissions) {
                auto rank = item.first;
                if (rank == tx.rank) {
                    continue;
                }

                for (auto &tx_i : item.second) {
                    if (tx.end <= tx_i.start || tx.start >= tx_i.end) {
                        continue;
                    }

                    auto pathloss_i = this->link_model[tx.rank][tx_i.rank];
                    if (common::is_zero(pathloss_i)) {
                        /* No link. */
                        continue;
                    }

                    interference.push_back(TX_POWER - pathloss_i);
                    break;
                }
            }

            /* Go through possible receivers. */
            auto &actions = this->action_queue.get_container();
            for (auto &action : actions) {
                if (action.type != Listen) {
                    continue;
                }

                auto &rx = action;

                if (!tx.is_within(rx)) {
                    /* Time interval does not intersect. */
                    continue;
                }

                if (rx.packet.time != 0ul) {
                    /* Listen action already have a packet. */
                    continue;
                }

                auto pathloss = this->link_model[rx.rank][tx.rank];

                if (common::is_zero(pathloss)) {
                    /* No link. */
                    continue;
                }

                auto rssi = TX_POWER - pathloss;
                auto pep = sims::radiomodel::pep(rssi, tx.packet.data.size(), interference);
                std::bernoulli_distribution d(1.0 - pep);
                auto should_receive = d(gen);

                /* Print RSSI and interference for debugging purposes. */
                if (this->debug) {
                    auto interfering_power = 0.0;
                    for (auto &RSSI_interference_dB : interference) {
                        interfering_power += sims::radiomodel::linearize(RSSI_interference_dB);
                    }

                    if (!common::is_zero(interfering_power)) {
                        interfering_power = sims::radiomodel::logarithmicize(interfering_power);
                    }

                    this->c->debug("{}(src: {}, dst: {}, rssi: {} dBm, pep: {}, p_i: {} dBm, p_ic: {})",
                                   should_receive ? "recv" : "drop",
                                   tx.rank, rx.rank, rssi, pep,
                                   interfering_power, interference.size());
                }

                if (should_receive) {
                    rx.packet = tx.packet;
                }
            }
        }

        if (act.type == Listen) {
            auto &rx = act;

            /* Hardware module will check to see if anything has been received. */
            mpi::send(rx.packet.time, rx.rank, RX_PKT_END);
            mpi::send(rx.packet.data, rx.rank, RX_PKT_DATA);
        }

        if (act.type == Sleep) {
        }

        if (act.type == Inform) {
        }
    }
}

bool Coordinator::Action::is_within(const Coordinator::Action &action) const {
    return this->start >= action.start && this->end <= action.end;
}

bool Coordinator::Packet::operator==(const Coordinator::Packet &rhs) const {
    return time == rhs.time &&
           data == rhs.data;
}

bool Coordinator::Packet::operator!=(const Coordinator::Packet &rhs) const {
    return !(rhs == *this);
}
