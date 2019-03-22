#include <cmath>
#include <algorithm>

#include <sims/linkmodel.h>
#include <sims/cholesky.h>
#include <sims/svd.h>
#include <sims/qr.h>
#include <sims/ostr.h>
#include <Eigen/Eigenvalues>


std::vector<double> compute_link_distance(const std::vector<sims::Optics::CLink> &links) {
    std::vector<double> l_distance{};

    for (const auto &link : links) {
        l_distance.emplace_back(sims::math::distance_pathloss(link));
    }

    return l_distance;
}


sims::linalg::vecvec<double>
sims::linkmodel::nearest_SPD(const sims::linalg::vecvec<double> &matrix) {
    auto svd_res = sims::svd::svd(matrix, 20);

    auto h = std::get<2>(svd_res) * std::get<0>(svd_res) * sims::linalg::transpose(std::get<2>(svd_res));
    auto spd = (matrix + h) / 2.0;

    uint32_t scalar = 1;
    uint32_t multiplier = 1;
    while (!sims::cholesky::is_positive_definite(spd)) {
        auto eigen = sims::linalg::eig(spd, 30);
        auto min_eig = eigen.values.back();

        spd = spd + (-min_eig * sims::math::next_power_of_2(scalar) +
                     (std::numeric_limits<double>::epsilon() * std::abs(min_eig))) *
                    sims::linalg::identity(spd.size());

        scalar++;

    }

    return spd;
}


sims::linalg::Eigen calc_eigen(const sims::linalg::vecvec<double> &matrix) {
    auto s = matrix.size();
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> x(s, s);
    ::std::vector<double> values(s);
    sims::linalg::vecvec<double> vectors;
    vectors.resize(s, ::std::vector<double>(s));

    for (auto row = 0; row < s; ++row) {
        for (auto column = 0; column < s; ++column) {
            x(row, column) = matrix[row][column];
        }
    }

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>> es;
    es.compute(x);

    auto ve = es.eigenvectors();
    auto va = es.eigenvalues();

    for (auto row = 0; row < s; ++row) {
        for (auto column = 0; column < s; ++column) {
            vectors[row][column] = ve(row, column);
        }
    }
    for (auto row = 0; row < s; ++row) {
        values[row] = va(row);
    }

    std::sort(std::begin(values), std::end(values));
    return sims::linalg::Eigen{vectors, values, 0, 0};
}

sims::linalg::vecvec<double> sims::linkmodel::near_pd(const sims::linalg::vecvec<double> &matrix) {
    auto x = matrix;

    while (true) {
        auto y = x;
        //auto eigen = sims::linalg::eig(x, 10);
        //auto eigen = sims::qr::qr_algorithm(x);
        auto eigen = calc_eigen(x);
        auto q = eigen.vectors;
        auto d = eigen.values;

        std::vector<int> p;
        auto eigen_tolerance = 1e-06 * d.front();
        for (auto i = 0; i < d.size(); ++i) {
            if (d[i] > eigen_tolerance)
                p.emplace_back(i);
        }

        std::sort(p.begin(), p.end());
        q = sims::linalg::slice_column_from_index_list(q, p);
        auto q_t = q;

        for (auto i = 0; i < p.size(); ++i) {
            for (auto &row : q)
                row[i] = row[i] * d[i];
        }

        x = sims::linalg::crossprod(sims::linalg::transpose(q), q_t);
        x = sims::linalg::diag(x, 1.0);

        auto diff = y - x;
        auto conv = sims::linalg::infinity_norm(diff) / sims::linalg::infinity_norm(y);

        if (conv <= 1e-07) {
            return x;
        }
    }
}


std::vector<double> compute_link_fading(const std::vector<sims::Optics::CLink> &links, double time = 0.0) {
    auto corr = sims::math::generate_correlation_matrix(links);
    /*if (!sims::cholesky::is_positive_definite(corr)) {
        std::cout << "ensuring spd" << std::endl;
        corr = sims::linkmodel::near_pd(corr);
    }

    if (!sims::cholesky::is_positive_definite(corr)) {
        std::cout << "near PD failed" << std::endl;
    }*/

    auto std_deviation = std::pow(STANDARD_DEVIATION, 2);
    auto sigma = std_deviation * corr;

    /*if (!sims::cholesky::is_positive_definite(sigma)) {
        std::cout << "ensuring spd" << std::endl;
        sigma = sims::linkmodel::nearest_SPD(sigma);
    }*/

    auto autocorrelation_matrix = sims::cholesky::cholesky(sigma);

    auto l_fading = autocorrelation_matrix * sims::math::generate_gaussian_vector(0.0, 1.0, links.size());
    return common::is_equal(time, 0.0) ? l_fading : l_fading * time;
}


std::vector<double>
sims::linkmodel::compute_spatial_correlation(const std::vector<sims::Optics::CLink> &links, double time) {
    return compute_link_fading(links, time);

}

std::vector<double> sims::linkmodel::compute_temporal_correlation(const std::vector<sims::Optics::CLink> &links,
                                                                    const double time, const double delta_time) {
    /* TODO: Compute the temporal coefficient */
    auto d_t = 1_km;
    auto d_r = 1_km;

    auto temporal_coefficient = std::exp(-(std::log(2) * ((d_t + d_r) / 20)));

    /* compute l_fading */
    auto l_fading = compute_link_fading(links, time);

    return sqrt(1 - temporal_coefficient) + l_fading * temporal_coefficient;
}

::std::vector<double> sims::linkmodel::compute(const std::vector<sims::Optics::CLink> &links, double time) {
    return compute_link_distance(links);// + compute_link_fading(links, time); /* TODO: + temporal*/
}