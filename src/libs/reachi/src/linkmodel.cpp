#include <cmath>
#include <algorithm>

#include <reachi/linkmodel.h>
#include <reachi/cholesky.h>
#include <reachi/svd.h>
#include <reachi/qr.h>


double temporal_correlation_coefficient(const double d_transmitter, const double d_receiver) {
    auto d_t = d_transmitter * KM;
    auto d_r = d_receiver * KM;

    return std::exp(-(std::log(2) * (20 / (d_t + d_r))));
}

std::vector<double> compute_link_distance(const std::vector<Link> &links) {
    std::vector<double> l_distance{};

    for (const auto &link : links) {
        l_distance.emplace_back(distance_pathloss(link));
    }

    return l_distance;
}

vecvec<double> ensure_positive_definiteness(const vecvec<double> &matrix) {
    auto svd_res = svd(matrix, 2);

    auto h = std::get<2>(svd_res) * std::get<0>(svd_res) * transpose(std::get<2>(svd_res));
    auto spd = (matrix + h) / 2.0;

    auto scalar = 0.0;
    while (!is_positive_definite(spd)) {
        std::cout << "not positive definite" << std::endl;
        auto eigen = eig(spd, 10);
        auto min_eig = eigen.values.back();

        scalar++;
        spd = spd + ((-min_eig * std::pow(scalar, 2) +
                      (std::numeric_limits<double>::epsilon() * std::abs(min_eig))) * identity(spd.size()));
    }

    return spd;
}

std::vector<double> compute_autocorrelation_matrix(const std::vector<Link> &links) {
    auto corr = generate_correlation_matrix(links);
    if (!is_positive_definite(corr))
        corr = ensure_positive_definiteness(corr);

    auto std_deviation = std::pow(STANDARD_DEVIATION, 2);
    auto sigma = std_deviation * corr;
    auto autocorrelation_matrix = cholesky(sigma);
}

std::vector<double> compute_link_fading(const std::vector<Link> &links, double time) {
    auto corr = generate_correlation_matrix(links);
    auto std_deviation = std::pow(STANDARD_DEVIATION, 2);
    auto sigma = std_deviation * corr;
    auto correlation_matrix = cholesky(sigma);

    auto l_fading = correlation_matrix * generate_gaussian_vector(0.0, 1.0, links.size());
    return time == 0.0 ? l_fading : l_fading * time;
}


std::vector<double> compute_link_rssi(std::vector<Link> &links, double tx_power, double time) {
    return tx_power - (compute_link_fading(links, time) + compute_link_distance(links));
}

std::vector<double> temporal_correlation(const std::vector<Link> &links, const double time, const double delta_time) {
    auto temporal_coefficient = temporal_correlation_coefficient(1.0, 1.0);
    auto l_fading = compute_link_fading(links, time);

    return sqrt(1 - temporal_coefficient) + l_fading * temporal_coefficient;
}

void spatial_correlation() {

}