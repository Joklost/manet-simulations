#include <catch.hpp>
#include <iostream>
#include <mpilib/helpers.h>
#include <chrono>

#include "reachi/cholesky.h"
#include "reachi/radiomodel.h"
#include "reachi/linkmodel.h"

#include "../src/datagen.h"

TEST_CASE("Compute the Cholesky decomposition (slow)", "[math]") {
    vecvec<double> matrix{{25.0, 15.0, -5.0},
                          {15.0, 18.0, 0.0},
                          {-5.0, 0.0,  11.0}};

    vecvec<double> result{{5.0,  0.0, 0.0},
                          {3.0,  3.0, 0.0},
                          {-1.0, 1.0, 3.0}};
    REQUIRE(result == slow_cholesky(matrix));
}

TEST_CASE("Generate a Gaussian Vector with 1 million elements", "[math]") {
    auto size = 1000000u;
    auto vec = generate_gaussian_vector(0.0, 1.0, size);
    REQUIRE(vec.size() == size);
}

TEST_CASE("Multiplication operator for 4x3 matrix and scalar value", "[math]") {
    vecvec<int> m1{{-1, 1,  4},
                   {6,  -4, 2},
                   {-3, 5,  0},
                   {3,  7,  -2}};

    vecvec<int> m1_expected{{-11, 11,  44},
                            {66,  -44, 22},
                            {-33, 55,  0},
                            {33,  77,  -22}};

    REQUIRE((m1 * 11) == m1_expected);

    vecvec<double> m2{{-1, 1,  4},
                      {6,  -4, 2},
                      {-3, 5,  0},
                      {3,  7,  -2}};

    vecvec<double> m2_expected{{-129.96, 129.96,  519.84},
                               {779.76,  -519.84, 259.92},
                               {-389.88, 649.8,   0.0},
                               {389.88,  909.72,  -259.92}};

    REQUIRE(compare_vectors((m2 * std::pow(11.4, 2)), m2_expected, 0.00000001));

}

TEST_CASE("Multiplication operator for 4x3 and 3x4 matrices", "[math]") {
    vecvec<int> m1{{-1, 1,  4},
                   {6,  -4, 2},
                   {-3, 5,  0},
                   {3,  7,  -2}};

    vecvec<int> m2{{-1, 1,  4,  8},
                   {6,  9,  10, 2},
                   {11, -4, 5,  -3}};

    vecvec<int> expected{{51, -8,  26, -18},
                         {-8, -38, -6, 34},
                         {33, 42,  38, -14},
                         {17, 74,  72, 44}};

    REQUIRE((m1 * m2) == expected);
}

TEST_CASE("Multiplication operator for 4x4 and 1x4 matrices", "[math]") {
    vecvec<int> m1{{1,  -1, -4,  -8},
                   {-6, 6,  24,  48},
                   {3,  -3, -12, -24},
                   {-3, 3,  12,  24}};

    std::vector<int> m2{-1, 1, 4, 8};

    std::vector<int> expected{-82, 492, -246, 246};

    REQUIRE((m1 * m2) == expected);
}

TEST_CASE("Multiplication operator for 4x1 and 1x4 matrices", "[math]") {
    vecvec<int> m1{{-1},
                   {6},
                   {-3},
                   {3}};

    vecvec<int> m2{{-1, 1, 4, 8}};

    vecvec<int> result{{1,  -1, -4,  -8},
                       {-6, 6,  24,  48},
                       {3,  -3, -12, -24},
                       {-3, 3,  12,  24}};

    REQUIRE((m1 * m2) == result);
}

TEST_CASE("Compute the distance dependent path loss (double)", "[math]") {
    REQUIRE(distance_pathloss(100) == Approx(91.2).margin(0.01));
    REQUIRE(distance_pathloss(141.55) == Approx(99.5).margin(0.01));
}

TEST_CASE("Compute the distance dependent path loss (Location)", "[math]") {
    Location l1{57.01266813458001, 9.994625734716218};
    Location l2{57.01266813458001, 9.9929758};
    REQUIRE(distance_pathloss(l1, l2) == Approx(91.2).margin(0.1));
}

TEST_CASE("Compute the autocorrelation for an angle (double)", "[math]") {
    REQUIRE(autocorrelation(45.0) == Approx(0.1254).margin(0.0001));
    REQUIRE(autocorrelation(90.0) == Approx(0.0939).margin(0.0001));
}


TEST_CASE("Compute distance dependent path loss", "[linkmodel]") {
    Node n1{1, {57.01266813458001, 9.994625734716218}};
    Node n2{2, {57.01266813458001, 9.9929758}};
    Node n3{3, {57.0117698, 9.9929758}};
    Node n4{4, {57.0117698, 9.994625734716218}};

    Link l6{6, n3, n4};
    Link l1{1, n1, n2};
    Link l2{2, n1, n3};
    Link l5{5, n2, n4};
    Link l3{3, n1, n4};
    Link l4{4, n2, n3};
    std::vector links{l1, l2, l3, l4, l5, l6};

    std::vector<double> l_distance{};
    std::vector<double> l_distance_expected{91.2, 99.5, 91.2, 91.2, 99.5, 91.2};
    std::for_each(links.cbegin(), links.cend(), [&l_distance](auto link) {
        l_distance.emplace_back(distance_pathloss(link));
    });

    REQUIRE(compare_vectors(l_distance, l_distance_expected, 0.1));
}

TEST_CASE("Compute the correlation matrix", "[linkmodel]") {
    Node n1{1, {57.01266813458001, 9.994625734716218}};
    Node n2{2, {57.01266813458001, 9.9929758}};
    Node n3{3, {57.0117698, 9.9929758}};
    Node n4{4, {57.0117698, 9.994625734716218}};

    Link l6{6, n3, n4};
    Link l1{1, n1, n2};
    Link l2{2, n1, n3};
    Link l5{5, n2, n4};
    Link l3{3, n1, n4};
    Link l4{4, n2, n3};
    std::vector links{l1, l2, l3, l4, l5, l6};

    vecvec<double> corr = generate_correlation_matrix_slow(links);
    vecvec<double> corr_expected{{1.0,   0.125, 0.094, 0.094, 0.125, 0.0},
                                 {0.125, 1.0,   0.125, 0.125, 0.0,   0.125},
                                 {0.094, 0.125, 1.0,   0.0,   0.125, 0.094},
                                 {0.094, 0.125, 0.0,   1.0,   0.125, 0.094},
                                 {0.125, 0.0,   0.125, 0.125, 1.0,   0.125},
                                 {0.0,   0.125, 0.094, 0.094, 0.125, 1.0}};

    REQUIRE(compare_vectors(corr, corr_expected, 0.001));
}

TEST_CASE("Compute stochastic fading path loss", "[linkmodel]") {
    Node n1{1, {57.01266813458001, 9.994625734716218}};
    Node n2{2, {57.01266813458001, 9.9929758}};
    Node n3{3, {57.0117698, 9.9929758}};
    Node n4{4, {57.0117698, 9.994625734716218}};

    Link l6{6, n3, n4};
    Link l1{1, n1, n2};
    Link l2{2, n1, n3};
    Link l5{5, n2, n4};
    Link l3{3, n1, n4};
    Link l4{4, n2, n3};
    std::vector links{l1, l2, l3, l4, l5, l6};

    vecvec<double> corr = generate_correlation_matrix_slow(links);

    /* Compute link fading   */
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = std_deviation * corr;
    vecvec<double> sigma_expected{{129.96,   16.245, 12.21624, 12.21624, 16.245, 0.0},
                                  {16.245,   129.96, 16.245,   16.245,   0.0,    16.245},
                                  {12.21624, 16.245, 129.96,   0.0,      16.245, 12.21624},
                                  {12.21624, 16.245, 0.0,      129.96,   16.245, 12.21624},
                                  {16.245,   0.0,    16.245,   16.245,   129.96, 16.245},
                                  {0.0,      16.245, 12.21624, 12.21624, 16.245, 129.96}};


    REQUIRE(compare_vectors(sigma, sigma_expected, 0.1));

    std::vector<double> gaussian_vector{-0.121966, -1.08682, 0.68429, -1.07519, 0.0332695, 0.744836};

    std::cout << slow_cholesky(sigma) << std::endl;

    auto l_fading = slow_cholesky(sigma) * gaussian_vector;
    std::vector<double> l_fading_expected{-1.39041, -12.4664, 6.17022, -13.8368, -0.158379, 6.41379};

    REQUIRE(compare_vectors(l_fading, l_fading_expected, 0.01));
}

TEST_CASE("Compute RSSI using spatial correlation", "[linkmodel]") {
    Node n1{1, {57.01266813458001, 9.994625734716218}};
    Node n2{2, {57.01266813458001, 9.9929758}};
    Node n3{3, {57.0117698, 9.9929758}};
    Node n4{4, {57.0117698, 9.994625734716218}};

    Link l6{6, n3, n4};
    Link l1{1, n1, n2};
    Link l2{2, n1, n3};
    Link l5{5, n2, n4};
    Link l3{3, n1, n4};
    Link l4{4, n2, n3};
    std::vector links{l1, l2, l3, l4, l5, l6};

    /* Compute link fading   */
    auto corr = generate_correlation_matrix_slow(links);
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = std_deviation * corr;
    std::vector<double> gaussian_vector{-0.121966, -1.08682, 0.68429, -1.07519, 0.0332695, 0.744836};
    auto l_fading = slow_cholesky(sigma) * gaussian_vector;

    /* Compute distance part */
    std::vector<double> l_distance{};
    std::for_each(links.cbegin(), links.cend(), [&l_distance](auto link) {
        l_distance.emplace_back(distance_pathloss(link));
    });

    auto tx_dbm = 26.0;
    auto rssi = tx_dbm - (l_distance + l_fading);
    std::vector<double> l_distance_expected{91.2, 99.5, 91.2, 91.2, 99.5, 91.2};
    std::vector<double> l_fading_expected{-1.39041, -12.4664, 6.17022, -13.8368, -0.158379, 6.41379};
    auto rssi_expected = tx_dbm - (l_distance_expected + l_fading_expected);

    REQUIRE(compare_vectors(rssi, rssi_expected, 0.1));
}

TEST_CASE("Compute RSSI using spatial and temporal correlation", "[linkmodel]") {
    Node n1{1, {57.01266813458001, 9.994625734716218}};
    Node n2{2, {57.01266813458001, 9.9929758}};
    Node n3{3, {57.0117698, 9.9929758}};
    Node n4{4, {57.0117698, 9.994625734716218}};

    Link l6{6, n3, n4};
    Link l1{1, n1, n2};
    Link l2{2, n1, n3};
    Link l5{5, n2, n4};
    Link l3{3, n1, n4};
    Link l4{4, n2, n3};
    std::vector links{l1, l2, l3, l4, l5, l6};

    /* Compute link fading   */
    auto corr = generate_correlation_matrix_slow(links);
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = std_deviation * corr;
    std::vector<double> gaussian_vector{-0.121966, -1.08682, 0.68429, -1.07519, 0.0332695, 0.744836};
    auto l_fading = slow_cholesky(sigma) * gaussian_vector;

    /* Compute distance part */
    std::vector<double> l_distance{};
    std::for_each(links.cbegin(), links.cend(), [&l_distance](auto link) {
        l_distance.emplace_back(distance_pathloss(link));
    });

    auto tx_dbm = 26.0;
    auto rssi = tx_dbm - (l_distance + l_fading);
    std::vector<double> l_distance_expected{91.2, 99.5, 91.2, 91.2, 99.5, 91.2};
    std::vector<double> l_fading_expected{-1.39041, -12.4664, 6.17022, -13.8368, -0.158379, 6.41379};
    auto rssi_expected = tx_dbm - (l_distance_expected + l_fading_expected);

    REQUIRE(compare_vectors(rssi, rssi_expected, 0.1));
}

TEST_CASE("Compute packet probability error", "[radiomodel]") {
    REQUIRE(packet_error_probability(-105.3, 160) == Approx(0.097154).margin(0.00001));
    REQUIRE(packet_error_probability(-104.0, 160) == Approx(0.014551).margin(0.00001));
}

TEST_CASE("Correlation matrix generation performance measure", "[linkmodel]") {
    /*Node n1{0, {57.01266813458001, 9.994625734716218}};
    Node n2{1, {57.01266813458001, 9.9929758}};
    Node n3{2, {57.0117698, 9.9929758}};
    Node n4{3, {57.0117698, 9.994625734716218}};

    Link l1{0, n1, n2};
    Link l2{1, n1, n3};
    Link l3{2, n1, n4};
    Link l4{3, n2, n3};
    Link l5{4, n2, n4};
    Link l6{5, n3, n4};
    std::vector links{l1, l2, l3, l4, l5, l6};*/

    // std::cout << measure<>::execution(generate_correlation_matrix_slow, links) << std::endl;
    /* vecvec<double> corr_expected{{1.0,   0.125, 0.094, 0.094, 0.125, 0.0},
                                 {0.125, 1.0,   0.125, 0.125, 0.0,   0.125},
                                 {0.094, 0.125, 1.0,   0.0,   0.125, 0.094},
                                 {0.094, 0.125, 0.0,   1.0,   0.125, 0.094},
                                 {0.125, 0.0,   0.125, 0.125, 1.0,   0.125},
                                 {0.0,   0.125, 0.094, 0.094, 0.125, 1.0}}; */// std::cout << measure<>::execution(generate_correlation_matrix, links) << std::endl;
    auto upper = Location{57.01266813458001, 9.994625734716218};
    auto lower = Location{57.0117698, 9.9929758};
    auto nodes = generate_nodes(50, upper, lower);
    auto links = generate_links(nodes);

    auto corr = generate_correlation_matrix(links);
    /*for (const auto &item : corr) {
        std::cout << "id: " << item.first.first.get_id() << ", " << item.first.second.get_id() << "\tvalue: " << item.second << std::endl;
    }*/
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = corr * std_deviation;

    //auto temp = cholesky(sigma);

    std::cout << measure<>::execution(cholesky, sigma) << std::endl;
/*    std::cout << "" << std::endl;
    for (const auto &item : temp) {
        std::cout << "id: " << item.first.first.get_id() << ", " << item.first.second.get_id() << "\tvalue: " << std::to_string(item.second) << std::endl;
    }*/

}