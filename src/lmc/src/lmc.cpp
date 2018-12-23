#include <reachi/linkmodel.h>
#include <reachi/datagen.h>
#include <mpilib/node.h>
#include "lmc.h"

std::vector<double> LinkModelComputer::fetch_model() {
    return this->linkmodel;
}

void LinkModelComputer::compute_linkmodel(std::vector<reachi::Node> &nodes) {
    /*
     * step 1: check if node information has been updated
     * step 2: do the clustering
     * step 3: calculate the temporal correlation with current data
     * step 4: update the public accessable link model so that the controller can fetch the newest model
     */

    reachi::Optics optics{};

    auto eps = 0.01;
    auto minpts = 2;
    auto link_threshold = 0.150;
    auto time = 0.0, time_delta = 0.0;

    auto ordering = optics.compute_ordering(nodes, eps, minpts);
    auto clusters = optics.cluster(ordering);
    auto links = reachi::data::create_link_vector(clusters, link_threshold);


    reachi::linkmodel::compute_spatial_correlation(links, time, time_delta);
}

void LinkModelComputer::update_model_data(std::vector<reachi::Node>) {

}

void LinkModelComputer::run() {

    mpi::init(&this->world_size, &this->world_rank, &this->name_len, this->processor_name);
    this->world_size = this->world_size - 2;
    this->c = spdlog::stdout_color_mt("lmc");
    if (this->debug) {
        this->c->set_level(spdlog::level::debug);
    }
    this->c->debug("started");

    /* Ensure that the compute worker is ready before using MPI. */
    auto compute_worker = std::thread{&LinkModelComputer::compute, this};

    if (!this->handshake()) {
        this->work = false;
        this->queue.push({poison_t});

        mpi::deinit();
        return;
    }

    auto receiver = std::thread{&LinkModelComputer::recv, this};

    receiver.join();
    compute_worker.join();

    mpi::deinit();
}

void LinkModelComputer::recv() {

    while (this->work) {
        auto status = mpi::probe(CTRLR);

        if (status.tag == DIE) {
            auto die = mpi::recv<int>(status.source, status.tag);
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

void LinkModelComputer::compute() {

    while (this->work) {
        auto act = this->queue.pop();

        if (act.type == poison_t) {
            break;
        }

        if (act.type == update_location_t) {
            auto loc = mpilib::deserialise<mpilib::geo::Location>(act.data);
            this->c->debug("update_location(rank={}, loc={})", act.rank, loc);
            this->nodes[act.rank].loc = loc;
        }
    }
}

bool LinkModelComputer::handshake() {
    auto magic = mpi::recv<int>(CTRLR, HANDSHAKE);
    mpi::send(this->world_rank, CTRLR, HANDSHAKE);

    if (magic != this->world_rank) {
        return false;
    }

    for (auto i = 1; i <= this->world_size; ++i) {
        auto node_buffer = mpi::recv<std::vector<octet>>(CTRLR, NODE_INFO);
        auto node = mpilib::deserialise<mpilib::Node>(node_buffer);
        this->nodes.insert(std::make_pair(node.rank, node));
    }

    return true;
}
