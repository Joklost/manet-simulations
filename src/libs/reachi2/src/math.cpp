#include <mpilib/geomath.h>
#include "reachi2/constants.h"
#include "reachi2/math.h"

reachi2::Node reachi2::math::get_common_node(const reachi2::Link &lhs, const reachi2::Link &rhs) {
    if (lhs.node1 == rhs.node1 || lhs.node1 == rhs.node2) {
        return lhs.node1;
    } else if (lhs.node2 == rhs.node1 || lhs.node2 == rhs.node2) {
        return lhs.node2;
    } else {
        auto node = reachi2::Node{};
        node.id = -1;
        return node;
    }
}

double reachi2::math::autocorrelation(const double angle) {
    return 0.595 * std::exp(-0.064 * angle) + 0.092;
}

double reachi2::math::distance_pathloss(const double distance) {
    return (10 * PATHLOSS_EXPONENT) * std::log10(distance) + PATHLOSS_CONSTANT_OFFSET;
}

double reachi2::math::distance_pathloss(mpilib::geo::Location &to, mpilib::geo::Location &from) {
    return distance_pathloss(distance_between(from, to) * KM);
}

Eigen::MatrixXd reachi2::math::generate_correlation_matrix(std::vector<reachi2::Link> &links) {
    Eigen::MatrixXd corr{};
    auto link_size = links.size();

    std::sort(std::begin(links), std::end(links));
    for (auto i = 0; i < link_size; ++i) {
        for (auto j = 0; j < i + 1; ++j) {
            reachi2::Node common_node;
            if (links[i] != links[j] && (common_node = reachi2::math::get_common_node(links[i], links[j])) != -1) {
                auto li_unique = links[i].node1 == common_node ? links[i].node1 : links[i].node2;
                auto lj_unique = links[j].node1 == common_node ? links[j].node1 : links[j].node2;

                auto angle = mpilib::geo::angle_between(common_node.location, li_unique.location, lj_unique.location);
                corr(i, j) = reachi2::math::autocorrelation(angle);
            } else if (links[i] == links[j]) {
                corr(i, j) = 1.0;
            } else {
                corr(i, j) = 0.0;
            }
        }
    }

    return corr;
}
