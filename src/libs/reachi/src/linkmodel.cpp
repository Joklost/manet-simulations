#include <cmath>
#include <algorithm>

#include <reachi/linkmodel.h>
#include <reachi/cholesky.h>
#include <reachi/svd.h>
#include <reachi/qr.h>

std::vector<double> compute_link_distance(const std::vector<Optics::CLink> &links) {
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
        auto eigen = eig(spd, 10);
        auto min_eig = eigen.values.back();

        scalar++;
        spd = spd + ((-min_eig * std::pow(scalar, 2) +
                      (std::numeric_limits<double>::epsilon() * std::abs(min_eig))) * identity(spd.size()));
    }

    return spd;
}

std::vector<double> compute_link_fading(const std::vector<Optics::CLink> &links, double time) {
    auto corr = generate_correlation_matrix(links);
    if (!is_positive_definite(corr))
        corr = ensure_positive_definiteness(corr);

    auto std_deviation = std::pow(STANDARD_DEVIATION, 2);
    auto sigma = std_deviation * corr;
    auto autocorrelation_matrix = cholesky(sigma);

    auto l_fading = autocorrelation_matrix * generate_gaussian_vector(0.0, 1.0, links.size());
    return time == 0.0 ? l_fading : l_fading * time;
}


std::vector<double> compute_temporal_correlation(const std::vector<Optics::CLink> &links, const double time, const double delta_time) {
    /* compute the temporal_coefficient */
    auto d_t = 1.0 * KM;
    auto d_r = 1.0 * KM;

    auto temporal_coefficient = std::exp(-(std::log(2) * (20 / (d_t + d_r))));

    /* compute l_fading */
    auto l_fading = compute_link_fading(links, time);

    return sqrt(1 - temporal_coefficient) + l_fading * temporal_coefficient;
}

std::vector<double> compute_spatial_correlation() {
    return std::vector<double>();
}
