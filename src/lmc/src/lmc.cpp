#include <reachi/linkmodel.h>
#include <reachi/datagen.h>
#include <mpilib/node.h>
#include <ostream>
#include "lmc.h"
//
//std::vector<double> LinkModelComputer::fetch_model() {
//    return this->linkmodel;
//}
//
//void LinkModelComputer::compute_linkmodel(std::vector<reachi::Node> &nodes) {
//    /*
//     * step 1: check if node information has been updated
//     * step 2: do the clustering
//     * step 3: calculate the temporal correlation with current data
//     * step 4: update the public accessable link model so that the controller can fetch the newest model
//     */
//
//    reachi::Optics optics{};
//
//    auto eps = 0.01;
//    auto minpts = 2;
//    auto link_threshold = 0.150;
//    auto time = 0.0, time_delta = 0.0;
//
//    auto ordering = optics.compute_ordering(nodes, eps, minpts);
//    auto clusters = optics.cluster(ordering);
//    auto links = reachi::data::create_link_vector(clusters, link_threshold);
//
//
//    reachi::linkmodel::compute_spatial_correlation(links, time, time_delta);
//}
//
//void LinkModelComputer::update_model_data(std::vector<reachi::Node>) {
//
//}

void LinkModelComputer::run() {

    mpi::init(&this->world_size, &this->world_rank, &this->name_len, this->processor_name);
    this->world_size = this->world_size - 2;
    this->c = spdlog::stdout_color_mt("lmc");
    if (this->debug) {
        this->c->set_level(spdlog::level::debug);
    }
    this->c->debug("init()");

    /* Ensure that the workers are ready before using MPI. */
    auto compute = std::thread{&LinkModelComputer::compute, this};
    auto control = std::thread{&LinkModelComputer::control, this};

    if (!this->handshake()) {
        this->work = false;
        this->cond_.notify_one();
        this->queue.push({poison_t});

        mpi::deinit();
        return;
    }

    auto receiver = std::thread{&LinkModelComputer::recv, this};

    receiver.join();
    control.join();
    compute.join();

    this->c->debug("deinit()");

    mpi::deinit();
}

void LinkModelComputer::recv() {

    while (this->work) {
        auto status = mpi::probe(CTRLR);

        if (status.tag == DIE) {
            mpi::recv<int>(status.source, status.tag);
            this->c->debug("die(source={}, tag={})", status.source, status.tag);
            this->queue.push({poison_t});
            break;
        }

        if (status.tag == UPDATE_LOCATION) {
            int rank = mpi::recv<int>(status.source, status.tag);
            auto loc = mpi::recv<std::vector<octet>>(status.source, UPDATE_LOCATION_DATA);
            this->queue.push({update_location_t, rank, loc});
        }
    }
}

void LinkModelComputer::control() {

    while (this->work) {
        auto act = this->queue.pop();

        if (act.type == poison_t) {
            this->work = false;
            this->cond_.notify_one();
            break;
        }

        if (act.type == update_location_t) {
            auto loc = mpilib::deserialise<mpilib::geo::Location>(act.data);
            this->c->debug("update_location(rank={}, loc={})", act.rank, loc);
            this->nodes[act.rank].loc = loc;

            this->is_valid = false;
            this->cond_.notify_one();
        }

        if (act.type == link_model_t) {
            this->c->debug("update_linkmodel(links={})", act.link_model.size());

            mpi::send(this->link_model.size(), CTRLR, LINK_MODEL);
            for (auto &link : this->link_model) {
                auto link_buffer = mpilib::serialise(link);
                mpi::send(link_buffer, CTRLR, LINK_MODEL_LINK);
            }
        }
    }
}

bool LinkModelComputer::handshake() {
    auto magic = mpi::recv<int>(CTRLR, HANDSHAKE);

    if (magic != this->world_rank) {
        return false;
    }

    for (auto i = 1; i <= this->world_size; ++i) {
        auto node_buffer = mpi::recv<std::vector<octet>>(CTRLR, NODE_INFO);
        auto node = mpilib::deserialise<mpilib::Node>(node_buffer);
        this->nodes.insert(std::make_pair(node.rank, node));
    }

    /* Wait for first Link Model to compute, and send ready signal to controller. */
    compute_link_model();
    mpi::send(this->world_rank, CTRLR, HANDSHAKE);

    /* Send link model to controller. */
    mpi::send(this->link_model.size(), CTRLR, LINK_MODEL);
    for (auto &link : this->link_model) {
        auto link_buffer = mpilib::serialise(link);
        mpi::send(link_buffer, CTRLR, LINK_MODEL_LINK);
    }

    return true;
}

void LinkModelComputer::compute() {

    while (this->work) {
        std::unique_lock<std::mutex> mlock(mutex_);
        this->cond_.wait(mlock);
        if (!this->work) {
            break;
        }

        if (this->is_valid) {
            continue;
        }

        compute_link_model();
    }

}

void LinkModelComputer::compute_link_model() {
    reachi::Optics optics{};

    auto eps = 0.01;
    auto minpts = 2;

    auto link_threshold = 350_m;

    auto time = 0.0, time_delta = 0.0;
    std::vector<reachi::Node> model_nodes{};
    for (auto &node : this->nodes) {
        model_nodes.emplace_back(node.second.rank, node.second.loc);
    }

    auto ordering = optics.compute_ordering(model_nodes, eps, minpts);
    auto clusters = optics.cluster(ordering);
    auto links = reachi::data::create_link_vector(clusters, link_threshold);

    this->c->debug("compute_link_model(clusters={}, links={})", clusters.size(), links.size());

    auto start = std::chrono::steady_clock::now();
    auto link_model = reachi::linkmodel::compute(links);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    this->c->debug("compute_done(execution={})", duration.count());

    std::vector<mpilib::Link> model{};

    /* Expand clusters to get the actual model. */
    for (auto i = 0; i < links.size(); ++i) {
        auto &link = links[i];
        auto &c1 = link.get_clusters().first;
        auto &c2 = link.get_clusters().second;
        auto rssi = link_model[i];

        for (auto &n1 : c1.get_nodes()) {
            for (auto &n2 : c2.get_nodes()) {
                model.push_back({static_cast<int>(n1.get_id()), static_cast<int>(n2.get_id()),
                                 rssi, link.get_distance()});
            }
        }
    }

    this->link_model = model;
    this->is_valid = true;

    Action act{link_model_t};
    act.link_model = model;
}
