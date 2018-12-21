#ifndef MANETSIMS_LINKMODEL_INTERFACE_H
#define MANETSIMS_LINKMODEL_INTERFACE_H

#include <vector>

#include <reachi/link.h>

class linkmodel_interface {
public:
    void update_model_data(std::vector<Link>);

    std::vector<double> fetch_model();

    void compute_linkmodel();

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
