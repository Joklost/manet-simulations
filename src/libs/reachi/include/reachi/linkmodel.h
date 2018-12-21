#ifndef MANETSIMS_LINKMODEL_H
#define MANETSIMS_LINKMODEL_H

#include <vector>
#include <reachi/link.h>
#include <reachi/math.h>

std::vector<double> compute_temporal_correlation(const std::vector<Optics::CLink> &links, double time, double delta_time);

std::vector<double> compute_spatial_correlation();

#endif //MANETSIMS_LINKMODEL_H
