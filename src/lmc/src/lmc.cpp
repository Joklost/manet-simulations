#include <reachi/linkmodel.h>
#include <reachi/datagen.h>
#include "lmc.h"

std::vector<double> lmc::fetch_model() {
    return this->linkmodel;
}

void lmc::compute_linkmodel(std::vector<reachi::Node> &nodes) {
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

void lmc::update_model_data(std::vector<reachi::Node>) {

}
