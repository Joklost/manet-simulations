#ifndef MANETSIMS_LINKMODEL_H
#define MANETSIMS_LINKMODEL_H

#include <vector>
#include <mpilib/link.h>

#include "reachi/math.h"

std::vector<double> compute_link_fading(std::vector<Link> &links, double time);

std::vector<double> compute_link_distance(std::vector<Link> &links);

std::vector<double> compute_link_rssi(std::vector<Link> &links, double tx_power, double time);

vecvec<double> correlation_matrix(std::vector<Link> &links);


#endif //MANETSIMS_LINKMODEL_H
