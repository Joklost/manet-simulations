#include <geo/geo.h>
#include <common/all.h>

#include "gpslog.h"
#include "coordinator.h"

static bool has_actions(const std::pair<unsigned long, Node> &item) {
    auto &node = item.second;
    return node.action_count > 0 || (node.action_count == 0 && node.dead);
}

void Coordinator::run() {
    int node_count{};
    int coordinator_rank{};
    int name_length{};
    char proc_name[MPI_MAX_PROCESSOR_NAME]{};

    mpi::init(&node_count, &coordinator_rank, &name_length, proc_name);
    this->world_size = node_count - 1;  /* Coordinator */
    this->world_rank = coordinator_rank;
    this->processor_name = std::string{proc_name};
    assert(this->world_rank == 0ul);

    this->c = spdlog::stdout_color_mt("coordr");
    if (this->debug) {
        this->c->set_level(spdlog::level::debug);
    }

    this->c->debug("init(nodes={})", this->world_size);

    if (!this->handshake()) {
        this->c->debug("deinit()");
        mpi::deinit();
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    while (true) {
        auto status = mpi::probe();
        if (this->enqueue_message(status) != CSUCCESS) {
            break;
        }
        this->process_actions(gen);
    }

    this->c->debug("deinit()");
    mpi::deinit();
}

int Coordinator::enqueue_message(const mpi::Status &status) {
    if (status.tag == DIE) {
        auto localtime = mpi::recv<unsigned long>(status.source, DIE);
        this->c->debug("die(source={}, localtime={})", status.source, localtime);

        this->nodes[status.source].dead = true;
        this->dead_nodes += 1;

        if (this->dead_nodes == this->world_size) {
            /* Break the loop to deinit the MPI. */
            return CERROR;
        }
    } else if (status.tag == TX_PKT) {
        Action act{Transmission, status.source};
        act.start = mpi::recv<unsigned long>(status.source, TX_PKT);
        auto data = mpi::recv<std::vector<octet>>(status.source, TX_PKT_DATA);
        act.end = act.start + mpilib::compute_transmission_time(BAUDRATE, data.size()).count();
        act.packet = {act.end, data};

        this->action_queue.push(act);
        this->transmissions[status.source].push_back(act);
        this->nodes[status.source].action_count += 1;
    } else if (status.tag == RX_PKT) {
        Action act{Listen, status.source};
        act.start = mpi::recv<unsigned long>(status.source, RX_PKT);
        act.end = act.start + mpi::recv<unsigned long>(status.source, RX_PKT_DURATION);

        this->action_queue.push(act);
        this->nodes[status.source].action_count += 1;
    } else if (status.tag == SLEEP) {
        Action act{Sleep, status.source};
        act.start = mpi::recv<unsigned long>(status.source, SLEEP);
        act.end = act.start + mpi::recv<unsigned long>(status.source, SLEEP_DURATION);

        this->action_queue.push(act);
        this->nodes[status.source].action_count += 1;
    } else if (status.tag == INFORM) {
        Action act{Inform, status.source};
        act.start = act.end = mpi::recv<unsigned long>(status.source, INFORM);

        this->action_queue.push(act);
        this->nodes[status.source].action_count += 1;
    }

    return CSUCCESS;
}

void Coordinator::process_actions(std::mt19937 &gen) {
    while (std::all_of(this->nodes.cbegin(), this->nodes.cend(), has_actions)) {
        auto act = this->action_queue.top(); /* Copy. */
        this->action_queue.pop();
        this->nodes[act.rank].action_count -= 1;

        if (act.type == Transmission) {
            auto &tx = act;

//            /* Get interference from other transmitters. */
//            std::vector<double> interference{};
//            for (auto &item : this->transmissions) {
//                auto rank = item.first;
//                if (rank == tx.rank) {
//                    continue;
//                }
//
//                for (auto &tx_i : item.second) {
//                    if (tx.end <= tx_i.start || tx.start >= tx_i.end) {
//                        continue;
//                    }
//
////                    auto pathloss_i = this->link_model[tx.rank][tx_i.rank];
//                    auto pathloss_i = 0.0;
//                    if (common::is_zero(pathloss_i)) {
//                        /* No link. */
//                        continue;
//                    }
//
//                    interference.push_back(TX_POWER - pathloss_i);
//                    break;
//                }
//            }
//
//            /* Go through possible receivers. */
//            auto &actions = this->action_queue.get_container();
//            for (auto &action : actions) {
//                if (action.type != Listen) {
//                    continue;
//                }
//
//                auto &rx = action;
//
//                if (!tx.is_within(rx)) {
//                    /* Time interval does not intersect. */
//                    continue;
//                }
//
//                if (rx.packet.time != 0ul) {
//                    /* Listen action already have a packet. */
//                    continue;
//                }
//
//                auto pathloss = 0.0;
////                auto pathloss = this->link_model[rx.rank][tx.rank];
//
//                if (common::is_zero(pathloss)) {
//                    /* No link. */
//                    continue;
//                }
//
//                auto rssi = TX_POWER - pathloss;
//                auto pep = sims::radiomodel::pep(rssi, tx.packet.data.size(), interference);
//                std::bernoulli_distribution d(1.0 - pep);
//                auto should_receive = d(gen);
//
//                /* Print RSSI and interference for debugging purposes. */
//                if (this->debug) {
//                    auto interfering_power = 0.0;
//                    for (auto &RSSI_interference_dB : interference) {
//                        interfering_power += sims::radiomodel::linearize(RSSI_interference_dB);
//                    }
//
//                    if (!common::is_zero(interfering_power)) {
//                        interfering_power = sims::radiomodel::logarithmicize(interfering_power);
//                    }
//
//                    this->c->debug("{}(src: {}, dst: {}, rssi: {} dBm, pep: {}, p_i: {} dBm, p_ic: {})",
//                                   should_receive ? "recv" : "drop",
//                                   tx.rank, rx.rank, rssi, pep,
//                                   interfering_power, interference.size());
//                }
//
//                if (should_receive) {
//                    rx.packet = tx.packet;
//                }
//            }
        }

        if (act.type == Listen) {
            auto &rx = act;

            /* Hardware module will check to see if anything has been received. */
            mpi::send(rx.packet.time, rx.rank, RX_PKT_END);
            mpi::send(rx.packet.data, rx.rank, RX_PKT_DATA);
        }
    }
}

Coordinator::Coordinator(const char *logpath, bool debug) : debug(debug) {
    this->nodes = load_log(logpath);

    if (this->nodes.size() != this->world_size) {
        throw std::runtime_error("mismatch in log node count and world size!");
    }

    for (auto &item : this->nodes) {
        auto &node = item.second;
        for (auto &location : node.location_history) {
            /* TODO: generate topology for every 20 seconds time jump. */
            if (this->topologies.find(location.time) == this->topologies.end()) {
                this->topologies[location.time] = {location.time};
            }
        }
    }
}

bool Coordinator::handshake() {
    /* Handshake and assign rank to all nodes. */
    common::enumerate(this->nodes.begin(), this->nodes.end(), 1ul, [this](auto i, auto &item) {
        Node &node = item.second;
        node.name = mpilib::processor_name(this->processor_name.c_str(), i);
        mpi::send(node.id, i, HANDSHAKE);
        auto response = mpi::recv<unsigned long>(i, HANDSHAKE);
        if (response == i) {
            node.rank = i;
            this->c->debug("handshake(target={}, status=success)", node.name);
        }
    });

    /* Sets the clock on all nodes. */
    for (auto &item : this->nodes) {
        auto &node = item.second;
        if (node.rank == 0) {
            this->c->debug("handshake(target={}, status=fail)", node.name);
            return false;
        }

        mpi::send(node.rank, node.rank, READY);
    }

    return true;
}
