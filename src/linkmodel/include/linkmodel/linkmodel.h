#ifndef MANETSIMS_LINKMODEL_H
#define MANETSIMS_LINKMODEL_H

#include <vector>
#include <mpilib/link.h>

std::vector<double> compute_link_fading(std::vector<Link> links, int time);

std::vector<double> compute_link_distance(std::vector<Link> links);

std::vector<double> compute_rx_rssi(std::vector<Link> links, double tx_power, int time);

#endif //MANETSIMS_LINKMODEL_H
