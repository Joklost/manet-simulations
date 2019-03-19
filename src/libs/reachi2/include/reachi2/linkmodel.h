#ifndef MANETSIMS_LINKMODEL_H
#define MANETSIMS_LINKMODEL_H

#include <vector>
#include <unordered_map>

#include "link.h"


#define TX_POWER 26.0


namespace reachi2 {
    class Linkmodel {
    public:
        void find_neighbourhood(const reachi2::Link &link);

        /**
         * Compute the linkmodel at the current object state
         */
        long double compute(std::vector<double> cholesky);

        /**
         * Compute the distance dependent path loss
         * @param distance Distance in meters
         * @return path loss (l_d)
         */
        static double distance_pathloss(double distance);

        const double compute_autocorrelation(double angle) const;

        const double generate_gaussian_value(double mean, double std_deviation) const;

        Linkmodel(std::vector<reachi2::Node> &nodes, double threshold = 0.55);

        Linkmodel(std::vector<reachi2::Link> &links);

        const std::unordered_map<int, double> &get_pep() const;

    private:
        std::unordered_map<int, std::vector<int>> neighbourhoods{};
        std::vector<reachi2::Link> links{};
        std::unordered_map<int, double> pep{};
        std::unordered_map<int, double> prev_rssi{};

    };
}
#endif //MANETSIMS_LINKMODEL_H
