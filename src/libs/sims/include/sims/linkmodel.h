#ifndef MANETSIMS_LINKMODEL_H
#define MANETSIMS_LINKMODEL_H

#include <vector>
#include <sims/link.h>
#include <sims/math.h>

namespace sims {
    namespace linkmodel {
        sims::linalg::vecvec<double> nearest_SPD(const sims::linalg::vecvec<double> &matrix);

        ::std::vector<double> compute(const std::vector<sims::Optics::CLink> &links, double time = 0.0);

        ::std::vector<double>
        compute_spatial_correlation(const std::vector<sims::Optics::CLink> &links, double time);

        ::std::vector<double>
        compute_temporal_correlation(const ::std::vector<Optics::CLink> &links, double time, double delta_time);

        sims::linalg::vecvec<double> near_pd(const sims::linalg::vecvec<double> &matrix);
    }
}

#endif //MANETSIMS_LINKMODEL_H
