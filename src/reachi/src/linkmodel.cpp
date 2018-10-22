#include <cmath>
#include <algorithm>
#include <reachi/linkmodel.h>


#include "reachi/linkmodel.h"
#include "reachi/math.h"
#include "reachi/cholesky.h"


vecvec<double> correlation_matrix(const std::vector<Link> &links) {
    auto corr = generate_correlation_matrix(links);
    auto std_deviation = std::pow(STANDARD_DEVIATION, 2);
    auto sigma = std_deviation * corr;
    return cholesky(sigma);
}


double temporal_correlation_coefficient(const double d_transmitter, const double d_receiver) {
    auto d_t = d_transmitter * KM;
    auto d_r = d_receiver * KM;

    return std::exp(-(std::log(2) * (20 / (d_t + d_r))));
}

std::vector<double> compute_link_distance(const std::vector<Link> &links) {
    std::vector<double> l_distance{};
    std::for_each(links.cbegin(), links.cend(), [&l_distance](auto link) {
        l_distance.emplace_back(distance_pathloss(link));
    });

    return l_distance;
}


std::vector<double> compute_link_fading(const std::vector<Link> &links, double time) {
    auto l_fading = correlation_matrix(links) * generate_gaussian_vector(0.0, 1.0, links.size());
    return time == 0.0 ? l_fading : l_fading * time;
}


std::vector<double> compute_link_rssi(std::vector<Link> &links, double tx_power, double time) {
    auto l_fading = compute_link_fading(links, time);
    auto l_distance = compute_link_distance(links);

    return tx_power - (l_distance + l_fading);
}

std::vector<double> temporal_correlation(const std::vector<Link> &links, const double time, const double delta_time) {
    auto temporal_coefficient = temporal_correlation_coefficient(1.0, 1.0);
    auto l_fading = compute_link_fading(links, time);

    return sqrt(1 - temporal_coefficient) + l_fading * temporal_coefficient;
}

void spatial_correlation() {

}