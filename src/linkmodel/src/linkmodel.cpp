#include <cmath>
#include <algorithm>
#include <linkmodel/linkmodel.h>


#include "linkmodel/linkmodel.h"
#include "linkmodel/math.h"
#include "linkmodel/cholesky.h"

std::vector<double> compute_rx_rssi(double tx_power, int time, std::vector<Link> links) {
    auto l_fading = compute_link_fading(links, time);
    auto l_distance = compute_link_distance(links);

    if (time == 0) {
        return tx_power - (l_distance + l_fading);
    }
}

std::vector<double> compute_link_fading(const std::vector<Link> links, int time) {
    auto corr = generate_correlation_matrix(links);
    auto std_deviation = std::pow(STANDARD_DEVIATION, 2);
    auto sigma = std_deviation * corr;
    auto gaussian_vector = generate_gaussian_vector(0.0, 1.0, links.size());
    return slow_cholesky(sigma) * gaussian_vector;
}

std::vector<double> compute_link_distance(const std::vector<Link> links) {
    std::vector<double> l_distance{};
    std::for_each(links.cbegin(), links.cend(), [&l_distance](auto link) {
        l_distance.emplace_back(distance_pathloss(link));
    });

    return l_distance;
}
