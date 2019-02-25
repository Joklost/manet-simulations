#include <random>

#include <mpilib/geomath.h>
#include <reachi/radiomodel.h>

#include "coordr.h"

#define TX_POWER 26.0
#define THRESHOLD -110.0 /* RSSI in dBm */

#ifndef THERMAL_NOISE
#define THERMAL_NOISE -119.66
#endif

#ifndef NOISE_FIGURE
#define NOISE_FIGURE 4.2
#endif

/*
bool Coordinator::has_link(Coordinator::Listen &rx, Coordinator::Transmission &tx) {
    if (rx.rank == tx.rank || !tx.is_within(rx)) {
        return false;
    }

    auto pathloss = this->link_model[rx.rank][tx.rank];
    auto rssi = TX_POWER - pathloss;

    return !(rssi > THRESHOLD || mpilib::is_zero(rssi));
}
*/

void Coordinator::run() {
    std::random_device rd;
    std::mt19937 gen(rd());

    mpi::init(&this->world_size, &this->world_rank, &this->name_len, this->processor_name);
    this->world_size = this->world_size - 2;
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

    while (true) {

        auto status = mpi::probe();

        if (status.tag == DIE) {
            auto localtime = mpi::recv<unsigned long>(status.source, DIE);
            this->c->debug("die(source={}, tag={}, localtime={})", status.source, status.tag, localtime);

            this->nodes[status.source].localtime = std::numeric_limits<unsigned long>::max() - 1ul;
            this->nodes[status.source].dead = true;

            auto all_dead = true;
            for (const auto &node : this->nodes) {
                if (!node.second.dead) {
                    all_dead = false;
                    break;
                }
            }

            if (all_dead) {
                /* Break the loop to deinit mpi. */
                break;
            }

        }

        if (status.tag == TX_PKT) {
            Action act{Transmission, status.source};
            act.start = mpi::recv<unsigned long>(status.source, TX_PKT);
            auto data = mpi::recv<std::vector<octet>>(status.source, TX_PKT_DATA);
            act.packets.push_back(data);
            act.end = act.start + mpilib::compute_transmission_time(BAUDRATE, data.size()).count();
            //this->action_queue.push(act);
            this->transmissions.push_back(act);

            this->nodes[status.source].localtime = act.end;
        }

        if (status.tag == RX_PKT) {
            Action act{Listen, status.source};
            act.start = mpi::recv<unsigned long>(status.source, RX_PKT);
            act.end = act.start + mpi::recv<unsigned long>(status.source, RX_PKT_DURATION);
            this->action_queue.push(act);

            this->nodes[status.source].localtime = act.end;
        }

        if (status.tag == NOOP) {
            Action act{NoOp, status.source};
            act.start = mpi::recv<unsigned long>(status.source, NOOP);
            act.end = act.start + mpi::recv<unsigned long>(status.source, NOOP_DURATION);
            //this->action_queue.push(act);

            this->nodes[status.source].localtime = act.end;
        }

        if (status.tag == SET_LOCATION) {
            auto &node = this->nodes[status.source];
            node.loc = update_location(status.source);
        }

        if (status.tag == LINK_MODEL) {
            auto count = mpi::recv<unsigned long>(status.source, LINK_MODEL);
            std::vector<mpilib::Link> links{};
            links.reserve(count);
            for (auto i = 0; i < count; ++i) {
                auto link_buffer = mpi::recv<std::vector<octet>>(status.source, LINK_MODEL_LINK);
                links.push_back(mpilib::deserialise<mpilib::Link>(link_buffer));
            }

            set_linkmodel(links);
        }

        unsigned long mintime = std::numeric_limits<unsigned long>::max();
        for (const auto &item : this->nodes) {
            auto &node = item.second;
            if (node.localtime < mintime) {
                mintime = node.localtime;
            }
        }

        if (mintime == std::numeric_limits<unsigned long>::max()) {
            continue;
        }

        while (true) {
            if (this->action_queue.empty()) {
                break;
            }

            auto act = this->action_queue.top();

            if (act.end > mintime) {
                break;
            }

            if (act.type == Listen) {
                this->action_queue.pop();

                for (auto &tx : this->transmissions) {
                    if (!tx.is_within(act)) {
                        continue;
                    }

                    auto pathloss = this->link_model[tx.rank][act.rank];
                    if (mpilib::is_zero(pathloss)) {
                        /* No link. */
                        continue;
                    }

                    std::vector<double> interference{};
                    auto rssi = TX_POWER - pathloss;

                    for (auto &tx_i : this->transmissions) {
                        if (tx_i.rank == tx.rank) {
                            continue;
                        }

                        if (act.end <= tx_i.start || act.start >= tx_i.end) {
                            /* Time interval does not intersect. */
                            continue;
                        }

                        auto interfering_pathloss = this->link_model[act.rank][tx_i.rank];
                        if (mpilib::is_zero(interfering_pathloss)) {
                            /* No link. */
                            continue;
                        }

                        interference.push_back(TX_POWER - interfering_pathloss);
                    }

                    auto pep = reachi::radiomodel::pep(rssi, tx.packets.back().size(), interference);
                    std::bernoulli_distribution d(1.0 - pep);
                    auto should_receive = d(gen);

                    if (should_receive) {
                        act.packets.push_back(tx.packets.back());
                    } else {
                        auto interfering_power = 0.0;
                        for (auto &RSSI_interference_dB : interference) {
                            interfering_power += reachi::radiomodel::linearize(RSSI_interference_dB);
                        }

                        if (!mpilib::is_zero(interfering_power)) {
                            interfering_power = reachi::radiomodel::logarithmicize(interfering_power);

                        }
                        this->c->info("packet_dropped(src: {}, dst: {}, rssi: {} dBm, pep: {}, p_i: {} dBm, p_ic: {})", act.rank, tx.rank, rssi, pep, interfering_power, interference.size());
                    }
                }

                mpi::send(act.packets.size(), act.rank, RX_PKT_COUNT);

                for (auto &packet : act.packets) {
                    mpi::send(packet, act.rank, RX_PKT_DATA);
                }
            }
        }

        /* Clean transmission vector. */
        //this->c->debug("transmissions before: {}", this->transmissions.size());
        /*
        this->transmissions.erase(
                std::remove_if(this->transmissions.begin(), this->transmissions.end(), [&mintime](Action act) {
                    return act.end < mintime;
                }), this->transmissions.end()
        );
        this->c->debug("transmissions after : {}", this->transmissions.size());
        */
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
            this->nodes[i].localtime = 0;
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

    /* Set the clocks on all nodes. */
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

mpilib::geo::Location Coordinator::update_location(const int rank) {
    auto buffer = mpi::recv<std::vector<octet>>(rank, SET_LOCATION);
    auto loc = mpilib::deserialise<mpilib::geo::Location>(buffer);

    this->nodes[rank].loc = loc;
    //mpi::send(rank, this->lmc_node, UPDATE_LOCATION);
    //mpi::send(buffer, this->lmc_node, UPDATE_LOCATION_DATA);

    return loc;
}

bool Coordinator::Action::is_within(const Coordinator::Action &action) const {
    return this->start >= action.start && this->end <= action.end;
}
