#include <algorithm>
#include <mpilib/geomath.h>
#include <mpilib/helpers.h>

#include "reachi/math.h"

double distance_pathloss(const double distance) {
    return (10 * PATHLOSS_EXPONENT) * std::log10(distance) + PATHLOSS_CONSTANT_OFFSET;
}

double distance_pathloss(const Location to, const Location from) {
    return distance_pathloss(distance_between(to, from) * KM);
}

double distance_pathloss(Link link) {
    return distance_pathloss(link.get_distance() * KM);
}

double autocorrelation(const double angle) {
    /* TODO: #define the constants */
    return 0.595 * std::exp(-0.064 * angle) + 0.092;
}

double autocorrelation(const Location to, const Location from) {
    /* TODO: Implement using angle_between */
    return 0;
}

double autocorrelation(Link link) {
    /* TODO: Implement using angle_between */
    return 0;
}

bool has_common_node(const Link &k, const Link &l) {
    auto k_nodes = k.get_nodes();
    auto l_nodes = l.get_nodes();

    return k_nodes.first == l_nodes.first || k_nodes.first == l_nodes.second ||
           k_nodes.second == l_nodes.first || k_nodes.second == l_nodes.second;
}

Node get_common_node(const Link &k, const Link &l) {
    auto k_nodes = k.get_nodes();
    auto l_nodes = l.get_nodes();

    if (k_nodes.first == l_nodes.first || k_nodes.first == l_nodes.second) {
        return k_nodes.first;
    } else if (k_nodes.second == l_nodes.first || k_nodes.second == l_nodes.second) {
        return k_nodes.second;
    } else {
        throw "Links have no common node.";
    }
}

vecvec<double> generate_correlation_matrix_slow(std::vector<Link> links) {
    auto size = links.size();
    vecvec<double> corr{};
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

                auto angle = angle_between(common_node, li_unique, lj_unique);
                corr[i][j] = autocorrelation(angle);

            } else if (links[i] == links[j]) {
                corr[i][j] = 1.0;
            } else {
                corr[i][j] = 0.0;
            }
        }
    }

    return corr;
}

LinkMap generate_correlation_matrix(std::vector<Link> links) {
    auto size = links.size();
    LinkMap res;

    for (auto i = 0; i < size; ++i) {
        if (links[i].get_distance() <= CORRELATION_COEFFICIENT_THRESHOLD) {
            continue;
        }

        for (auto j = 0; j < size; ++j) {
            double value = 0.0;

            if (res.contains(links[i], links[j])) {
                continue;
            } else if (links[i] == links[j]) {
                value = 1.0;
            } else if (links[i] != links[j] && has_common_node(links[i], links[j])) {
                auto common_node = get_common_node(links[i], links[j]);
                auto li_unique = links[i].get_nodes().first.get_id() == common_node.get_id() ?
                                 links[i].get_nodes().second :
                                 links[i].get_nodes().first;

                auto lj_unique = links[j].get_nodes().first.get_id() == common_node.get_id() ?
                                 links[j].get_nodes().second :
                                 links[j].get_nodes().first;

                auto angle = angle_between(common_node, li_unique, lj_unique);
                value = autocorrelation(angle);
            }

            if (value >= CORRELATION_COEFFICIENT_THRESHOLD) {
                res.emplace(links[i], links[j], value);
            }
        }

    }
    return res;
}


