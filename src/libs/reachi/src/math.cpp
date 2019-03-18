#include <algorithm>
#include <limits>

#include <geo/geo.h>
#include <reachi/math.h>
#include <reachi/svd.h>

double reachi::math::distance_pathloss(const double distance) {
    return (10 * PATHLOSS_EXPONENT) * std::log10(distance) + PATHLOSS_CONSTANT_OFFSET;
}

double reachi::math::distance_pathloss(const geo::Location to, const geo::Location from) {
    return distance_pathloss(distance_between(from, to) * KM);
}

double reachi::math::distance_pathloss(const Link &link) {
    return distance_pathloss(link.get_distance() * KM);
}

double reachi::math::distance_pathloss(const Optics::CLink &link) {
    return distance_pathloss(link.get_distance() * KM);
}

double reachi::math::autocorrelation(const double angle) {
    /* TODO: #define the constants */
    return 0.595 * std::exp(-0.064 * angle) + 0.092;
}

bool has_common_node_optics(const reachi::Optics::CLink &k, const reachi::Optics::CLink &l) {
    const auto &k_clusters = k.get_clusters();
    const auto &l_clusters = l.get_clusters();

    return k_clusters.first == l_clusters.first || k_clusters.first == l_clusters.second ||
           k_clusters.second == l_clusters.first || k_clusters.second == l_clusters.second;
}

reachi::Optics::Cluster get_common_node_optics(const reachi::Optics::CLink &k, const reachi::Optics::CLink &l) {
    const auto &k_clusters = k.get_clusters();
    const auto &l_clusters = l.get_clusters();

    if (k_clusters.first == l_clusters.first || k_clusters.first == l_clusters.second) {
        return k_clusters.first;
    } else if (k_clusters.second == l_clusters.first || k_clusters.second == l_clusters.second) {
        return k_clusters.second;
    } else {
        throw "Links have no common node.";
    }
}

bool has_common_node(const reachi::Link &k, const reachi::Link &l) {
    auto k_nodes = k.get_nodes();
    auto l_nodes = l.get_nodes();

    return k_nodes.first == l_nodes.first || k_nodes.first == l_nodes.second ||
           k_nodes.second == l_nodes.first || k_nodes.second == l_nodes.second;
}

reachi::Node get_common_node(const reachi::Link &k, const reachi::Link &l) {
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

reachi::linalg::vecvec<double> reachi::math::generate_correlation_matrix(std::vector<Optics::CLink> links) {
    auto size = links.size();
    reachi::linalg::vecvec<double> corr{};
    corr.resize(size, std::vector<double>(size));

    std::sort(links.begin(), links.end());

    for (auto i = 0; i < size; ++i) {
        for (auto j = 0; j < i + 2; ++j) {
            if (j == size) continue;

            if (links[i] == links[j]) {
                corr[i][j] = 1.0;
            } else if (links[i] != links[j] && has_common_node_optics(links[i], links[j])) {
                auto common_node = get_common_node_optics(links[i], links[j]);
                auto li_unique = links[i].get_clusters().first.get_id() == common_node.get_id() ?
                                 links[i].get_clusters().second :
                                 links[i].get_clusters().first;

                auto lj_unique = links[j].get_clusters().first.get_id() == common_node.get_id() ?
                                 links[j].get_clusters().second :
                                 links[j].get_clusters().first;

                auto angle = geo::angle_between(common_node.centroid(), li_unique.centroid(),
                                                        lj_unique.centroid());
                auto value = autocorrelation(angle);

                //if (value >= CORRELATION_COEFFICIENT_THRESHOLD)
                corr[i][j] = value;
            } else if (j > i)
                corr[i][j] = 0.000001;
        }
    }

    return corr;
}

reachi::linalg::vecvec<double> reachi::math::generate_correlation_matrix_slow(std::vector<Link> links) {
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
}

reachi::linalg::vecvec<double> reachi::math::generate_correlation_matrix(std::vector<Link> links) {
    auto size = links.size();
    reachi::linalg::vecvec<double> corr{};
    corr.resize(size, std::vector<double>(size));

    std::sort(links.begin(), links.end());

    for (auto i = 0; i < size; ++i) {
        for (auto j = 0; j < i + 1; ++j) {
            // if (j == size) continue;

            if (links[i] == links[j]) {
                corr[i][j] = 1.0;
            } else if (links[i] != links[j] && has_common_node(links[i], links[j])) {
                auto common_node = get_common_node(links[i], links[j]);
                auto li_unique = links[i].get_nodes().first.get_id() == common_node.get_id() ?
                                 links[i].get_nodes().second :
                                 links[i].get_nodes().first;

                auto lj_unique = links[j].get_nodes().first.get_id() == common_node.get_id() ?
                                 links[j].get_nodes().second :
                                 links[j].get_nodes().first;

                auto angle = angle_between(common_node.get_location(), li_unique.get_location(),
                                           lj_unique.get_location());
                auto value = autocorrelation(angle);
                corr[i][j] = value >= CORRELATION_COEFFICIENT_THRESHOLD ? value : 0.0;
            }
        }
    }

    return corr;
}

uint64_t reachi::math::next_power_of_2(uint32_t n) {
    uint64_t res = 1;

    if (n && !(n & (n - 1)))
        return n;

    while (res < n)
        res <<= 1;

    return res;
}


