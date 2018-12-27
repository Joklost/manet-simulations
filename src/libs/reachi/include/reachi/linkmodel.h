#ifndef MANETSIMS_LINKMODEL_H
#define MANETSIMS_LINKMODEL_H

#include <vector>
#include <reachi/link.h>
#include <reachi/math.h>

namespace reachi {
    namespace linkmodel {

        ::std::vector<double> compute(const std::vector<reachi::Optics::CLink> &links, double time = 0.0);

        ::std::vector<double>
        compute_spatial_correlation(const std::vector<reachi::Optics::CLink> &links, double time);

        ::std::vector<double>
        compute_temporal_correlation(const ::std::vector<Optics::CLink> &links, double time, double delta_time);

    }
}

#endif //MANETSIMS_LINKMODEL_H
