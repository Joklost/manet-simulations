#ifndef MANETSIMS_LINKMODEL_INTERFACE_H
#define MANETSIMS_LINKMODEL_INTERFACE_H

#include <vector>

#include <reachi/link.h>

class lmc {
public:
    void update_model_data(std::vector<reachi::Node>);

    std::vector<double> fetch_model();

    void compute_linkmodel(std::vector<reachi::Node> &nodes);

private:
    std::vector<double> linkmodel;
};

/*
 * notes:
 *
 * needs Nodes information(a list of all nodes) to do the clustering
 *
 *
 */

#endif //MANETSIMS_LINKMODEL_INTERFACE_H
