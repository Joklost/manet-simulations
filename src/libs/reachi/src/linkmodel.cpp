#include <cmath>
#include <algorithm>

#include <reachi/linkmodel.h>
#include <reachi/cholesky.h>
#include <reachi/svd.h>
#include <reachi/qr.h>

std::vector<double> compute_link_distance(const std::vector<reachi::Optics::CLink> &links) {
    std::vector<double> l_distance{};

    for (const auto &link : links) {
        l_distance.emplace_back(reachi::math::distance_pathloss(link));
    }

    return l_distance;
}

reachi::linalg::vecvec<double>
reachi::linkmodel::ensure_positive_definiteness(const reachi::linalg::vecvec<double> &matrix) {
    auto b = (matrix + reachi::linalg::transpose(matrix)) / 2.0;
    auto svd_res = reachi::svd::svd(b, 5);

    auto h = std::get<2>(svd_res) * std::get<0>(svd_res) * reachi::linalg::transpose(std::get<2>(svd_res));
    auto spd = (b + h) / 2.0;
    spd = (spd + reachi::linalg::transpose(spd)) / 2.0;

    auto scalar = 0.0;
    while (!reachi::cholesky::is_positive_definite(spd)) {
        auto eigen = reachi::linalg::eig(spd, 10);
        auto min_eig = eigen.values.back();

        scalar++;
        spd = spd + (((-min_eig * std::pow(scalar, 2)) +
                      (std::numeric_limits<double>::epsilon() * std::abs(min_eig))) *
                     reachi::linalg::identity(spd.size()));
    }

    return spd;
}

std::vector<double> compute_link_fading(const std::vector<reachi::Optics::CLink> &links, double time = 0.0) {
    auto corr = reachi::math::generate_correlation_matrix(links);

    auto std_deviation = std::pow(STANDARD_DEVIATION, 2);
    auto sigma = std_deviation * corr;

    if (!reachi::cholesky::is_positive_definite(sigma)) {
        std::cout << "ensuring psd" << std::endl;
        sigma = reachi::linkmodel::ensure_positive_definiteness(sigma);
    }

    auto autocorrelation_matrix = reachi::cholesky::cholesky(sigma);

    auto l_fading = autocorrelation_matrix * reachi::math::generate_gaussian_vector(0.0, 1.0, links.size());
    return mpilib::is_equal(time, 0.0) ? l_fading : l_fading * time;
}


std::vector<double>
reachi::linkmodel::compute_spatial_correlation(const std::vector<reachi::Optics::CLink> &links, double time) {
    return compute_link_fading(links, time);

}

std::vector<double> reachi::linkmodel::compute_temporal_correlation(const std::vector<reachi::Optics::CLink> &links,
                                                                    const double time, const double delta_time) {
    /* TODO: Compute the temporal coefficient */
    auto d_t = 1_km;
    auto d_r = 1_km;

    auto temporal_coefficient = std::exp(-(std::log(2) * (20 / (d_t + d_r))));

    /* compute l_fading */
    auto l_fading = compute_link_fading(links, time);

    return sqrt(1 - temporal_coefficient) + l_fading * temporal_coefficient;
}

::std::vector<double> reachi::linkmodel::compute(const std::vector<reachi::Optics::CLink> &links, double time) {
    return compute_link_distance(links) + compute_link_fading(links, time); /* TODO: + temporal*/
}
