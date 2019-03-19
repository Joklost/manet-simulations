#include <catch2/catch.hpp>
#include <Eigen/Core>
#include <iostream>
#include <chrono>

#include <geo/geomath.h>
#include <geo/location.h>
#include <reachi2/datagen.h>
#include <reachi2/constants.h>
#include <reachi2/linkmodel.h>

using namespace std::literals::chrono_literals;
using namespace geo::literals;

template<typename T>
using vecvec = std::vector<std::vector<T>>;

reachi2::Node get_common_node(const reachi2::Link &k, const reachi2::Link &l) {
    if (k.node1 == l.node1 || k.node1 == l.node2) {
        return k.node1;
    } else if (k.node2 == l.node1 || k.node2 == l.node2) {
        return k.node2;
    } else {
        throw "Links have no common node.";
    }
}

double autocorrelation(double angle) {
    return 0.595 * std::exp(-0.064 * angle) + 0.092;
}

vecvec<double> generate_correlation_matrix_slow(std::vector<reachi2::Link> links) {
    auto size = links.size();
    vecvec<double> corr{};
    corr.resize(size, std::vector<double>(size));


    for (auto i = 0; i < size; ++i) {
        for (auto j = 0; j < size; ++j) {
            if (links[i] != links[j] && links[i].has_common_node(links[j])) {
                auto common_node = get_common_node(links[i], links[j]);
                auto li_unique = links[i].node1.id == common_node.id ?
                                 links[i].node2 :
                                 links[i].node1;

                auto lj_unique = links[j].node1.id == common_node.id ?
                                 links[j].node2 :
                                 links[j].node1;

                auto angle = angle_between(common_node.location, li_unique.location,
                                           lj_unique.location);
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

vecvec<double> slow_cholesky(const vecvec<double> matrix) {
    auto size = matrix.size();
    vecvec<double> vec{};
    vec.resize(size, std::vector<double>(size));

    for (auto i = 0; i < size; ++i) {
        for (auto j = 0; j <= i; ++j) {
            double sum = 0;
            for (auto k = 0; k < j; ++k) {
                sum += vec[i][k] * vec[j][k];
            }

            if (i == j) {
                auto val = ::std::sqrt(matrix[i][i] - sum);
                vec[i][j] = (::std::isnan(val) ? 0.0 : val);
            } else {
                vec[i][j] = ((double) 1) / vec[j][j] * (matrix[i][j] - sum);
            }

        }
    }

    return vec;
}

std::vector<double> operator*(const vecvec<double> &lhs, const ::std::vector<double> &rhs) {
    std::vector<double> res;
    auto n = lhs[0].size();

    res.resize(n);

    for (auto i = 0; i < lhs.size(); ++i) {
        double calc_res = 0;
        for (auto j = 0; j < rhs.size(); ++j) {
            calc_res += lhs[i][j] * rhs[j];
        }
        res[i] = calc_res;
    }

    return res;
}


TEST_CASE("Compute the distance dependent path loss (double)", "[linkmodel]") {
    REQUIRE(reachi2::Linkmodel::distance_pathloss(100) == Approx(91.2).margin(0.1));
    REQUIRE(reachi2::Linkmodel::distance_pathloss(141.55) == Approx(99.5).margin(0.01));
}

TEST_CASE("Compute the linkmodel", "[linkmodel]") {
    /*std::vector<reachi2::Node> nodes{
            reachi2::Node{1, geo::Location{56.999147, 11.025754}},
            reachi2::Node{2, geo::Location{57.006189, 11.004112}},
            reachi2::Node{3, geo::Location{57.004228, 11.023358}},
            reachi2::Node{4, geo::Location{57.001848, 11.017360}},
            reachi2::Node{5, geo::Location{57.010136, 11.020456}},
            reachi2::Node{6, geo::Location{56.995869, 11.021405}},
            reachi2::Node{7, geo::Location{57.001986, 11.013671}},
            reachi2::Node{8, geo::Location{57.004942, 11.016966}},
            reachi2::Node{9, geo::Location{56.997372, 11.018124}},
            reachi2::Node{10, geo::Location{57.011220, 10.998674}},
            reachi2::Node{11, geo::Location{57.000980, 11.007252}},
            reachi2::Node{12, geo::Location{57.012513, 11.023667}},
            reachi2::Node{13, geo::Location{57.009188, 11.013052}},
            reachi2::Node{14, geo::Location{57.001161, 10.998664}},
            reachi2::Node{15, geo::Location{57.002079, 11.000976}},
            reachi2::Node{16, geo::Location{57.002160, 11.016619}},
            reachi2::Node{17, geo::Location{56.997656, 11.002646}},
            reachi2::Node{18, geo::Location{56.999753, 11.005291}},
            reachi2::Node{19, geo::Location{57.011176, 11.005935}},
            reachi2::Node{20, geo::Location{57.000419, 11.026364}}
    };*/



    geo::Location upper{57.01266813458001, 10.994625734716218};
    auto lower = geo::square(upper, 2_km);

    double high = 0.0;
    double low = 1000000.0;
    long double total = 0.0;

    for (auto i = 0; i < 30; ++i) {
        auto nodes = reachi2::data::generate_nodes(20, upper, lower);
        auto links = reachi2::data::create_links(nodes);

//        cholesky part
        auto corr = generate_correlation_matrix_slow(links);
        auto std_deviation = std::pow(11.4, 2);
        vecvec<double> sigma;

        auto n = corr[0].size();
        auto m = corr.size();
        sigma.resize(m, std::vector<double>(n));

        for (auto i = 0; i < m; ++i) {
            for (auto j = 0; j < n; ++j) {
                sigma[i][j] = corr[i][j] * std_deviation;
            }
        }

        auto l = slow_cholesky(sigma);
        auto t = std::vector<double>(l.size(), 1.0);
        auto chol_res = l * t;


        reachi2::Linkmodel linkmodel{links};
        auto diff = linkmodel.compute(chol_res);
        if (diff > high)
            high = diff;
        else if (diff < low)
            low = diff;

        total += diff;
        std::cout
                << "difference: " << diff
                << ", link count: " << links.size()
                << ", average: " << diff / links.size()
                << std::endl;
//        std::cout << std::endl << "total difference: " << total << std::endl;
    }
    std::cout << std::endl
              << "highest: " << high
              << ", lowest: " << low
              << ", total: " << total
              << std::endl;

//    for (const auto &item : model) {
//        std::cout << item.second << std::endl;
//    }
}