#ifndef MANETSIMS_LINKMODEL_H
#define MANETSIMS_LINKMODEL_H

#include <vector>
#include <unordered_map>

#include "reachi2/link.h"

namespace reachi2 {
    class Linkmodel {
    public:
        void find_neighbourhood(const reachi2::Link &link);

        void compute();

        const double generate_fading_value(const reachi2::Link &link);

        double distance_pathloss(double distance) const;

        const double generate_gaussian_value(double mean, double std_deviation) const;

        Linkmodel(const std::vector<reachi2::Node> &nodes, double threshold);

        Linkmodel(std::vector<reachi2::Link> &links);

    private:
        std::unordered_map<int, std::vector<int>> neighbourhoods{};
        std::vector<reachi2::Link> graph{};
        std::vector<double> pathloss_values{};
        std::unordered_map<int, double> model{};
    };
}
#endif //MANETSIMS_LINKMODEL_H
