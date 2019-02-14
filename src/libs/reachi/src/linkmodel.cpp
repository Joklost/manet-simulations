#include <cmath>
#include <algorithm>

#include <reachi/linkmodel.h>
#include <reachi/cholesky.h>
#include <reachi/svd.h>
#include <reachi/qr.h>
#include <reachi/ostr.h>
#include <Eigen/Eigenvalues>


std::vector<double> compute_link_distance(const std::vector<reachi::Optics::CLink> &links) {
    std::vector<double> l_distance{};

    for (const auto &link : links) {
        l_distance.emplace_back(reachi::math::distance_pathloss(link));
    }

    return l_distance;
}


reachi::linalg::vecvec<double>
reachi::linkmodel::nearest_SPD(const reachi::linalg::vecvec<double> &matrix) {
    auto svd_res = reachi::svd::svd(matrix, 20);

    auto h = std::get<2>(svd_res) * std::get<0>(svd_res) * reachi::linalg::transpose(std::get<2>(svd_res));
    auto spd = (matrix + h) / 2.0;

    uint32_t scalar = 1;
    uint32_t multiplier = 1;
    while (!reachi::cholesky::is_positive_definite(spd)) {
        auto eigen = reachi::linalg::eig(spd, 30);
        auto min_eig = eigen.values.back();

        spd = spd + (-min_eig * reachi::math::next_power_of_2(scalar) +
                     (std::numeric_limits<double>::epsilon() * std::abs(min_eig))) *
                    reachi::linalg::identity(spd.size());

        std::cout << spd[0][0] << std::endl;
        scalar++;

    }

    return spd;
}


reachi::linalg::Eigen calc_eigen(const reachi::linalg::vecvec<double> &matrix) {
    auto s = matrix.size();
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> x(s, s);
    ::std::vector<double> values(s);
    reachi::linalg::vecvec<double> vectors;
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
    return reachi::linalg::Eigen{vectors, values, 0, 0};
}

reachi::linalg::vecvec<double> reachi::linkmodel::near_pd(const reachi::linalg::vecvec<double> &matrix) {
    auto x = matrix;

    while (true) {
        auto y = x;
        //auto eigen = reachi::linalg::eig(x, 10);
        //auto eigen = reachi::qr::qr_algorithm(x);
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
        q = reachi::linalg::slice_column_from_index_list(q, p);
        auto q_t = q;

        for (auto i = 0; i < p.size(); ++i) {
            for (auto &row : q)
                row[i] = row[i] * d[i];
        }

        x = reachi::linalg::crossprod(reachi::linalg::transpose(q), q_t);
        x = reachi::linalg::diag(x, 1.0);

        auto diff = y - x;
        auto conv = reachi::linalg::infinity_norm(diff) / reachi::linalg::infinity_norm(y);

        if (conv <= 1e-07) {
            return x;
        }
    }
}


std::vector<double> compute_link_fading(const std::vector<reachi::Optics::CLink> &links, double time = 0.0) {
    auto corr = reachi::math::generate_correlation_matrix(links);
    /*if (!reachi::cholesky::is_positive_definite(corr)) {
        std::cout << "ensuring spd" << std::endl;
        corr = reachi::linkmodel::near_pd(corr);
    }

    if (!reachi::cholesky::is_positive_definite(corr)) {
        std::cout << "near PD failed" << std::endl;
    }*/

    auto std_deviation = std::pow(STANDARD_DEVIATION, 2);
    auto sigma = std_deviation * corr;

    /*if (!reachi::cholesky::is_positive_definite(sigma)) {
        std::cout << "ensuring spd" << std::endl;
        sigma = reachi::linkmodel::nearest_SPD(sigma);
    }*/

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

    auto temporal_coefficient = std::exp(-(std::log(2) * ((d_t + d_r) / 20)));

    /* compute l_fading */
    auto l_fading = compute_link_fading(links, time);

    return sqrt(1 - temporal_coefficient) + l_fading * temporal_coefficient;
}

::std::vector<double> reachi::linkmodel::compute(const std::vector<reachi::Optics::CLink> &links, double time) {
    return compute_link_distance(links);// + compute_link_fading(links, time); /* TODO: + temporal*/
}
