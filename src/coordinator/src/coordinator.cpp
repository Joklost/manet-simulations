#include <common/all.h>

#include "gpslog.h"
#include "coordinator.h"
#include "models/radiomodel.h"

void Coordinator::run() {
    int size{};
    int rank{};
    int name_length{};
    char proc_name[MPI_MAX_PROCESSOR_NAME]{};

    mpi::init(&size, &rank, &name_length, proc_name);
    this->world_size = size - 1;  /* Coordinator. */
    this->world_rank = rank;
    this->processor_name = std::string{proc_name};

    if (this->world_rank != 0ul) {
        mpi::deinit();
        throw std::runtime_error("coordr rank != 0");
    }

    this->c = spdlog::stdout_color_mt("coordr");
    if (this->debug) {
        this->c->set_level(spdlog::level::debug);
    } else {
        this->c->set_pattern("%v");
    }

    this->c->debug("init(nodes={})", this->world_size);

    if (!this->handshake()) {
        this->c->debug("deinit()");
        mpi::deinit();
        return;
    }

    if (this->nodes.size() != this->world_size) {
        std::string err{"mismatch in log node count and world size "};
        err.append(std::to_string(this->nodes.size()));
        err.append(" != ");
        err.append(std::to_string(this->world_size));
        throw std::runtime_error(err.c_str());
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

    if (this->plots) {
        this->stats.save();
    }
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

        this->actions.emplace(act);
        this->transmissions.push_back(act);
        this->nodes[status.source].action_count += 1;
    } else if (status.tag == RX_PKT) {
        Action act{Listen, status.source};
        act.start = mpi::recv<unsigned long>(status.source, RX_PKT);
        act.end = act.start + mpi::recv<unsigned long>(status.source, RX_PKT_DURATION);

        this->actions.emplace(act);
        this->nodes[status.source].action_count += 1;
    } else if (status.tag == SLEEP) {
        Action act{Sleep, status.source};
        act.start = mpi::recv<unsigned long>(status.source, SLEEP);
        act.end = act.start + mpi::recv<unsigned long>(status.source, SLEEP_DURATION);

        this->actions.emplace(act);
        this->nodes[status.source].action_count += 1;
    } else if (status.tag == INFORM) {
        Action act{Inform, status.source};
        act.start = act.end = mpi::recv<unsigned long>(status.source, INFORM);

        this->actions.emplace(act);
        this->nodes[status.source].action_count += 1;
    }

    return CSUCCESS;
}

static bool has_actions(const std::pair<unsigned long, Node> &rank_node_pair) {
    auto &node = rank_node_pair.second;
    return node.action_count > 0 || (node.action_count == 0 && node.dead);
}

void Coordinator::process_actions(std::mt19937 &gen) {
    const auto action_count = this->actions.size();

    while (std::all_of(this->nodes.cbegin(), this->nodes.cend(), has_actions)) {
        auto head = this->actions.begin();
        auto act = *head; /* Copy. */
        this->actions.erase(head);
        this->nodes[act.rank].action_count--;

        if (act.type == Transmission) {
            auto &tx = act;

            auto &topology = this->get_topology(tx.start);
            if (topology.locations.find(tx.rank) == topology.locations.end()) {
                /* Node is not present in this topology. */
                continue;
            }

            /* Get interference from other transmitters. */
            std::vector<double> interference{};
            for (auto &tx_i : this->transmissions) {
                if (tx_i.rank == tx.rank) {
                    continue;
                }

                if (tx.end < tx_i.start || tx.start > tx_i.end) {
                    continue;
                }

                auto rssi = topology.get_link(tx.rank, tx_i.rank);
                if (common::is_zero(rssi)) {

                    continue;
                }

                interference.push_back(rssi);
            }

            /* Go through possible receivers. */
            for (auto it = this->actions.begin(); it != this->actions.end();) { // NOLINT(modernize-loop-convert)
                if (it->type != Listen) {
                    it++;
                    continue;
                }

                if (!tx.is_within(*it)) {
                    /* Transmit is not within receive interval. */
                    it++;
                    continue;
                }

                auto rssi = topology.get_link(it->rank, tx.rank);

                if (common::is_zero(rssi)) {
                    it++;
                    continue;
                }

                auto pep = RadioModel::pep(rssi, tx.packet.data.size(), interference);
                std::bernoulli_distribution d{1.0 - pep};
                auto should_receive = d(gen);

                if (this->plots) {
                    /* Log RSSI and interference for plots. */
                    auto interfering_power = 0.0;
                    for (auto &RSSI_interference_dB : interference) {
                        interfering_power += RadioModel::lin(RSSI_interference_dB);
                    }

                    if (!common::is_zero(interfering_power)) {
                        interfering_power = RadioModel::log(interfering_power);
                    }

                    this->stats.packetloss.emplace_back(
                            should_receive,
                            this->nodes[tx.rank].id,
                            this->nodes[it->rank].id,
                            tx.packet.data.size(),
                            rssi,
                            pep,
                            interfering_power,
                            interference.size(),
                            tx.start / 1000.0,
                            tx.end / 1000.0
                    );
                }

                if (should_receive) {
                    //this->c->debug("tx.end:{}", tx.end);
                    mpi::send(tx.end, it->rank, RX_PKT_END);
                    mpi::send(tx.packet.data, it->rank, RX_PKT_DATA);
                    this->c->debug("receive={}", *it);
                    this->nodes[it->rank].action_count--;
                    it = this->actions.erase(it);
                } else {
                    it++;
                }
            }
        }

        if (act.type == Listen) {
            auto &rx = act;
            mpi::send(rx.end, rx.rank, RX_PKT_END);
            mpi::send(std::vector<octet>{}, rx.rank, RX_PKT_DATA);
        }
    }

//    this->c->debug(" ");
//    for (const auto &node : this->nodes) {
//        this->c->debug("node={}, action_count={}, dead={}", node.second.rank, node.second.action_count,
//                       node.second.dead);
//    }
//    for (auto &action : this->actions) {
//        this->c->debug("action={}", action);
//    }
//    this->c->debug(" ");
/*
    if (action_count != this->action_queue.size()) {
        const auto &acts = this->action_queue.get_container();
        auto it = std::min_element(acts.cbegin(), acts.cend(), [](const Action &left, const Action &right) {
            return left.start < right.start;
        });

        auto &start_time = it->start;
        auto &txs = this->transmissions;
        txs.erase(std::remove_if(txs.begin(), txs.end(), [start_time](const Action &tx) {
            return tx.end < start_time;
        }), txs.end());

        if (this->plots) {
            const auto &act = this->action_queue.top();
            this->stats.queue_sizes.emplace_back(act.end, this->action_queue.size());
            this->stats.transmissions_sizes.emplace_back(act.end, this->transmissions.size());
        }
    }
    */
}

Coordinator::Coordinator(const char *logpath, bool debug, bool plots) : debug(debug), plots(plots) {
    auto time_nodes_pair = load_log(logpath);
    auto max_time = time_nodes_pair.first;
    this->nodelist = time_nodes_pair.second;

    /* Generate topologies */
    auto t = 0.0;
    while (t < max_time) {
        this->topologies[t] = {t};
        t += TIME_GAP;
    }

    std::unordered_map<unsigned long, unsigned long> ranks{};
    for (const auto &node : this->nodelist) {
        ranks[node.id] = node.rank;
    }

    for (auto &node : this->nodelist) {
        for (auto &time_location_pair : node.locations) {
            const auto time = time_location_pair.first;
            auto &location = time_location_pair.second;
            auto &topology = this->get_topology(time);
            topology.locations[node.rank] = location;

            for (auto &id_rssi_pair : location.links) {
                const auto id = id_rssi_pair.first;
                const auto rssi = id_rssi_pair.second;

                /* Precompute the conversion from id to rank. */
                topology.links[ranks[node.id]][ranks[id]] = rssi;
            }
        }
    }
}

bool Coordinator::handshake() {
    /* Handshake and assign rank to all nodes. */
    for (auto &node : this->nodelist) {
        auto i = node.rank;
        node.name = mpilib::processor_name(this->processor_name.c_str(), i);
        mpi::send(node.id, i, HANDSHAKE);
        auto response = mpi::recv<unsigned long>(i, HANDSHAKE);
        if (response == i) {
            this->nodes[node.rank] = node;
            this->c->debug("handshake(target={}, status=success)", node.name);
        } else {
            this->c->debug("handshake(target={}, status=fail)", node.name);
            return false;
        }
    }

    /* Sets the clock on all nodes. */
    for (auto &node : this->nodelist) {
        mpi::send(node.rank, node.rank, READY);
    }

    return true;
}

Topology &Coordinator::get_topology(unsigned long time) {
    return this->get_topology(time / 1000.0); /* Convert to double. */
}

Topology &Coordinator::get_topology(double time) {
    return this->topologies[std::round(time - std::fmod(time, TIME_GAP))];
}
