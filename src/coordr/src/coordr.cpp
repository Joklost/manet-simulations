#include "coordr.h"


void Coordinator::run() {
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

        } else if (status.tag == TX_PKT) {
            this->transmit_actions.emplace_back();
            auto &tx = this->transmit_actions.back();
            tx.rank = status.source;
            tx.start = mpi::recv<unsigned long>(status.source, TX_PKT);
            tx.data = mpi::recv<std::vector<octet>>(status.source, TX_PKT_DATA);
            tx.end = tx.start + mpilib::compute_transmission_time(BAUDRATE, tx.data.size()).count();

        } else if (status.tag == RX_PKT) {
            this->listen_actions.emplace_back();
            auto &rx = this->listen_actions.back();
            rx.rank = status.source;
            rx.start = mpi::recv<unsigned long>(status.source, RX_PKT);
            rx.end = rx.start + mpi::recv<unsigned long>(status.source, RX_PKT_DURATION);

        } else if (status.tag == SLEEP) {
            auto &node = this->nodes[status.source];
            auto localtime = mpi::recv<unsigned long>(status.source, SLEEP);
            auto duration = mpi::recv<unsigned long>(status.source, SLEEP_DURATION);
            node.localtime = localtime + duration;

        } else if (status.tag == SET_LOCATION) {
            auto &node = this->nodes[status.source];
            node.loc = update_location(status.source);
        } else if (status.tag == SET_LOCAL_TIME) {
            auto &node = this->nodes[status.source];
            node.localtime = mpi::recv<unsigned long>(status.source, SET_LOCAL_TIME);
//            this->c->debug("register_localtime(source={}, tag={}, localtime={})", status.source, status.tag, localtime);
        }

        if (status.tag == LINK_MODEL) {
            auto count = mpi::recv<unsigned long>(status.source, LINK_MODEL);
            std::vector<mpilib::Link> link_model{};
            link_model.reserve(count);
            for (auto i = 0; i < count; ++i) {
                auto link_buffer = mpi::recv<std::vector<octet>>(status.source, LINK_MODEL_LINK);
                link_model.push_back(mpilib::deserialise<mpilib::Link>(link_buffer));
            }

        }
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


mpilib::geo::Location Coordinator::update_location(const int rank) {
    auto buffer = mpi::recv<std::vector<octet>>(rank, SET_LOCATION);
    auto loc = mpilib::deserialise<mpilib::geo::Location>(buffer);

    this->c->debug("update_location(source={}, loc={})", rank, loc);
    this->nodes[rank].loc = loc;
    return loc;
}

bool Coordinator::Action::is_within(Coordinator::Action listen) {
    return this->start >= listen.start &&
           this->end <= listen.end;
}
