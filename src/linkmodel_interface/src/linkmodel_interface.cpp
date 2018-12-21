#include <reachi/linkmodel.h>
#include "linkmodel_interface.h"

std::vector<double> linkmodel_interface::fetch_model() {
    return this->linkmodel;
}

void linkmodel_interface::compute_linkmodel() {
    /*
     * step 1: check if node information has been updated
     * step 2: do the clustering
     * step 3: calculate the temporal correlation with current data
     * step 4: update the public accessable link model so that the controller can fetch the newest model
     */

    //compute_temporal_correlation(cluster_links, time, time_delta);
}
