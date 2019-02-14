#include <mpilib/geomath.h>

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

Eigen::MatrixXd reachi2::math::generate_correlation_matrix(std::vector<reachi2::Link> &links) {
    Eigen::MatrixXd corr{};
    auto link_size = links.size();

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

double reachi2::math::distance_pathloss(const double distance) {

}

double reachi2::math::distance_pathloss(mpilib::geo::Location to, mpilib::geo::Location from) {

}

double reachi2::math::distance_pathloss(reachi2::Link link) {

}




/*reachi::linalg::vecvec<double> reachi::math::generate_correlation_matrix_slow(std::vector<Link> links) {
    auto size = links.size();
    reachi::linalg::vecvec<double> corr{};
    corr.resize(size, std::vector<double>(size));

    std::sort(links.begin(), links.end());

    for (auto i = 0; i < size; ++i) {
        for (auto j = 0; j < size; ++j) {
            if (links[i] != links[j] && has_common_node(links[i], links[j])) {
                auto common_node = get_common_node(links[i], links[j]);
                auto li_unique = links[i].get_nodes().first.get_id() == common_node.get_id() ?
                                 links[i].get_nodes().second :
                                 links[i].get_nodes().first;

                auto lj_unique = links[j].get_nodes().first.get_id() == common_node.get_id() ?
                                 links[j].get_nodes().second :
                                 links[j].get_nodes().first;

                auto angle = angle_between(common_node.get_location(), li_unique.get_location(),
                                           lj_unique.get_location());
                corr[i][j] = autocorrelation(angle);

            } else if (links[i] == links[j]) {
                corr[i][j] = 1.0;
            } else {
                corr[i][j] = 0.0;
            }
        }
    }

    return corr;
}*/