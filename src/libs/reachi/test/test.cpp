#include <catch2/catch.hpp>
#include <iostream>
#include <cmath>
#include <chrono>

#include <reachi/cholesky.h>
#include <reachi/radiomodel.h>
#include <reachi/linkmodel.h>
#include <reachi/svd.h>
#include <reachi/qr.h>
#include <reachi/datagen.h>
#include <reachi/ostr.h>
#include <mpilib/helpers.h>
#include <mpilib/ostr.h>
#include <Eigen/Eigenvalues>


TEST_CASE("Compute the Cholesky decomposition (slow)", "[math]") {
    reachi::linalg::vecvec<double> matrix{{25.0, 15.0, -5.0},
                                          {15.0, 18.0, 0.0},
                                          {-5.0, 0.0,  11.0}};

    reachi::linalg::vecvec<double> result{{5.0,  0.0, 0.0},
                                          {3.0,  3.0, 0.0},
                                          {-1.0, 1.0, 3.0}};
    REQUIRE(result == reachi::cholesky::slow_cholesky(matrix));
}

TEST_CASE("Comparing cholesky implementations for correct results", "[math]") {
    auto upper = mpilib::geo::Location{57.01266813458001, 10.994625734716218};
    auto lower = mpilib::geo::Location{57.0117698, 10.9929758};
    auto nodes = reachi::data::generate_nodes(25, upper, lower);
    auto links = reachi::data::create_link_vector(nodes, 1);


    // testing original implementation
    auto begin = std::chrono::steady_clock::now();

    auto corr_org = reachi::math::generate_correlation_matrix_slow(links);
    auto std_deviation_org = std::pow(11.4, 2);
    auto sigma_org = corr_org * std_deviation_org;
    auto cholesky_res_org = reachi::cholesky::slow_cholesky(sigma_org);

    auto end = std::chrono::steady_clock::now();
    std::cout << "Original code: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << std::endl;


    // testing our implementation
    auto begin_1 = std::chrono::steady_clock::now();

    auto corr_our = reachi::math::generate_correlation_matrix(links);
    auto std_deviation_our = std::pow(11.4, 2);
    auto sigma_our = corr_our * std_deviation_our;
    auto cholesky_res_our = reachi::cholesky::cholesky(sigma_our);

    auto end_1 = std::chrono::steady_clock::now();
    std::cout << "Our code: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_1 - begin_1).count()
              << std::endl;


    REQUIRE(reachi::linalg::compare_vectors(cholesky_res_org, cholesky_res_our, 0.00001));
}

TEST_CASE("Generate a Gaussian Vector with 1 million elements", "[math]") {
    auto size = 1000000u;
    auto vec = reachi::math::generate_gaussian_vector(0.0, 1.0, size);
    REQUIRE(vec.size() == size);
}

TEST_CASE("Multiplication operator for 4x3 matrix and scalar value", "[math]") {
    reachi::linalg::vecvec<int> m1{{-1, 1,  4},
                                   {6,  -4, 2},
                                   {-3, 5,  0},
                                   {3,  7,  -2}};

    reachi::linalg::vecvec<int> m1_expected{{-11, 11,  44},
                                            {66,  -44, 22},
                                            {-33, 55,  0},
                                            {33,  77,  -22}};

    REQUIRE((m1 * 11) == m1_expected);

    reachi::linalg::vecvec<double> m2{{-1, 1,  4},
                                      {6,  -4, 2},
                                      {-3, 5,  0},
                                      {3,  7,  -2}};

    reachi::linalg::vecvec<double> m2_expected{{-129.96, 129.96,  519.84},
                                               {779.76,  -519.84, 259.92},
                                               {-389.88, 649.8,   0.0},
                                               {389.88,  909.72,  -259.92}};

    REQUIRE(reachi::linalg::compare_vectors((m2 * std::pow(11.4, 2)), m2_expected, 0.00000001));

}

TEST_CASE("Multiplication operator for 4x3 and 3x4 matrices", "[math]") {
    reachi::linalg::vecvec<int> m1{{-1, 1,  4},
                                   {6,  -4, 2},
                                   {-3, 5,  0},
                                   {3,  7,  -2}};

    reachi::linalg::vecvec<int> m2{{-1, 1,  4,  8},
                                   {6,  9,  10, 2},
                                   {11, -4, 5,  -3}};

    reachi::linalg::vecvec<int> expected{{51, -8,  26, -18},
                                         {-8, -38, -6, 34},
                                         {33, 42,  38, -14},
                                         {17, 74,  72, 44}};

    REQUIRE((m1 * m2) == expected);
}

TEST_CASE("Multiplication operator for 4x4 and 1x4 matrices", "[math]") {
    reachi::linalg::vecvec<int> m1{{1,  -1, -4,  -8},
                                   {-6, 6,  24,  48},
                                   {3,  -3, -12, -24},
                                   {-3, 3,  12,  24}};

    std::vector<int> m2{-1, 1, 4, 8};

    std::vector<int> expected{-82, 492, -246, 246};

    REQUIRE((m1 * m2) == expected);
}

TEST_CASE("Multiplication operator for 4x1 and 1x4 matrices", "[math]") {
    reachi::linalg::vecvec<int> m1{{-1},
                                   {6},
                                   {-3},
                                   {3}};

    reachi::linalg::vecvec<int> m2{{-1, 1, 4, 8}};

    reachi::linalg::vecvec<int> result{{1,  -1, -4,  -8},
                                       {-6, 6,  24,  48},
                                       {3,  -3, -12, -24},
                                       {-3, 3,  12,  24}};

    REQUIRE((m1 * m2) == result);
}

TEST_CASE("Compute the distance dependent path loss (double)", "[math]") {
    REQUIRE(reachi::math::distance_pathloss(100) == Approx(91.2).margin(0.01));
    REQUIRE(reachi::math::distance_pathloss(141.55) == Approx(99.5).margin(0.01));
}

TEST_CASE("Compute the distance dependent path loss (Location)", "[math]") {
    mpilib::geo::Location l1{57.01266813458001, 9.994625734716218};
    mpilib::geo::Location l2{57.01266813458001, 9.9929758};
    REQUIRE(reachi::math::distance_pathloss(l1, l2) == Approx(91.2).margin(0.1));
}

TEST_CASE("Compute the autocorrelation for an angle (double)", "[math]") {
    REQUIRE(reachi::math::autocorrelation(45.0) == Approx(0.1254).margin(0.0001));
    REQUIRE(reachi::math::autocorrelation(90.0) == Approx(0.0939).margin(0.0001));
}

TEST_CASE("Dot product", "[math]") {
    reachi::linalg::vecvec<double> v1{{0, 3, 5},
                                      {5, 5, 2}};

    reachi::linalg::vecvec<double> v2{{3, 4},
                                      {3, -2},
                                      {4, -2}};

    reachi::linalg::vecvec<double> expected{{29, -16},
                                            {38, 6}};

    auto res_1 = reachi::linalg::dot(v1, v2);
    REQUIRE(reachi::linalg::compare_vectors(res_1, expected, 0.01));
}

TEST_CASE("Slicing vecvecs and vectors", "[math]") {
    std::vector<int> v1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    reachi::linalg::vecvec<int> v2{{0, 1, 2, 3},
                                   {0, 1, 2, 3},
                                   {0, 1, 2, 3},
                                   {0, 1, 2, 3},
                                   {0, 1, 2, 3}};

    std::vector<int> expected_v1_1{1, 2, 3, 4};
    std::vector<int> expected_v1_2{1, 2, 3, 4, 5, 6, 7, 8, 9};

    reachi::linalg::vecvec<int> expected_v2_1{{0, 1, 2, 3},
                                              {0, 1, 2, 3},
                                              {0, 1, 2, 3},
                                              {0, 1, 2, 3}};

    reachi::linalg::vecvec<int> expected_v2_2{{1},
                                              {1}};


    auto res_v1_1 = reachi::linalg::slice(v1, 1, 5);
    auto res_v1_2 = reachi::linalg::slice(v1, 1);

    auto res_v2_1 = reachi::linalg::slice(v2, 1);
    auto res_v2_2 = reachi::linalg::slice(v2, 1, 2, 1, 2);

    REQUIRE(reachi::linalg::compare_vectors(res_v1_1, expected_v1_1, 0));
    REQUIRE(reachi::linalg::compare_vectors(res_v1_2, expected_v1_2, 0));
    REQUIRE(reachi::linalg::compare_vectors(res_v2_1, expected_v2_1, 0));
    REQUIRE(reachi::linalg::compare_vectors(res_v2_2, expected_v2_2, 0));
}

TEST_CASE("Slice matrix based on indexes for columns", "[linalg]") {
    reachi::linalg::vecvec<double> data{{1, 4, 7},
                                        {2, 5, 8},
                                        {3, 6, 9}};

    reachi::linalg::vecvec<double> expected{{4, 7},
                                            {5, 8},
                                            {6, 9}};

    auto res = reachi::linalg::slice_column_from_index_list(data, std::vector<int>{1, 2});
    REQUIRE(res == expected);
}

/*TEST_CASE("Matrix crossproduct", "[linalg]") {
    reachi::linalg::vecvec<int> data_1{{4,  2, 2},
                                       {4,  6, 8},
                                       {-2, 2, 4}};


    reachi::linalg::vecvec<int> expected_1{{36, 28, 32},
                                           {28, 44, 60},
                                           {32, 60, 84}};


    auto res_1 = reachi::linalg::crossprod(data_1, reachi::linalg::transpose(data_1));

    REQUIRE(res_1 == expected_1);
}*/

TEST_CASE("Matrix crossprod", "[linalg]") {
    /* Verify the result for symmetric matrices */
    reachi::linalg::vecvec<int> data1{{1, 4, 7},
                                      {2, 5, 8},
                                      {3, 6, 9}};

    reachi::linalg::vecvec<int> expected_d1{{14, 32,  50},
                                            {32, 77,  122},
                                            {50, 122, 194}};

    auto res_d1 = reachi::linalg::crossprod(data1);
    REQUIRE(res_d1 == expected_d1);

    /* Verify for asymmetric matrices */
    reachi::linalg::vecvec<int> data2{{1, 4, 7},
                                      {2, 5, 8}};

    reachi::linalg::vecvec<int> expected_d2{{5,  14, 23},
                                            {14, 41, 68},
                                            {23, 68, 113}};
    reachi::linalg::vecvec<int> expected_d2_transposed{{66, 78},
                                                       {78, 93}};

    auto res_d2 = reachi::linalg::crossprod(data2);
    auto res_d2_transposed = reachi::linalg::crossprod(reachi::linalg::transpose(data2));
    REQUIRE(res_d2 == expected_d2);
    REQUIRE(res_d2_transposed == expected_d2_transposed);
    REQUIRE(res_d2 == reachi::linalg::crossprod(data2, reachi::linalg::transpose(data2)));


    reachi::linalg::vecvec<int> data3{{1, 4},
                                      {2, 5},
                                      {3, 6}};

    reachi::linalg::vecvec<int> expected_d3{{14, 32},
                                            {32, 77}};
    reachi::linalg::vecvec<int> expected_d3_transposed{{17, 22, 27},
                                                       {22, 29, 36},
                                                       {27, 36, 45}};

    auto res_d3 = reachi::linalg::crossprod(data3);
    auto res_d3_transposed = reachi::linalg::crossprod(reachi::linalg::transpose(data3));
    REQUIRE(res_d3 == expected_d3);
    REQUIRE(res_d3_transposed == expected_d3_transposed);
    REQUIRE(res_d3 == reachi::linalg::crossprod(data3, reachi::linalg::transpose(data3)));
}


TEST_CASE("R libs nearPD implementation verification", "[linalg]") {
    reachi::linalg::vecvec<double> data{{1.0, 1.0, 0.0},
                                        {1.0, 1.0, 1.0},
                                        {0.0, 1.0, 1.0}};

    reachi::linalg::vecvec<double> expected{{1.0000000, 0.7630019, 0.1643439},
                                            {.7630019,  1.0000000, 0.7630019},
                                            {0.1643439, 0.7630019, 1.0000000}};

    auto res = reachi::linkmodel::near_pd(data);
    std::cout << res << std::endl;
}

TEST_CASE("Calculate the infinity norm of a vecvec", "[linalg]") {
    reachi::linalg::vecvec<double> data{{1.0, 1.0, 0.0},
                                        {1.0, 1.0, 1.0},
                                        {0.0, 1.0, 1.0}};
    double expected = 3.0;
    auto res = reachi::linalg::infinity_norm(data);
    REQUIRE(res == expected);
}

TEST_CASE("Eigen lib test", "[eigen]") {
    reachi::linalg::vecvec<double> data{{1.0, 1.0, 0.0},
                                        {1.0, 1.0, 1.0},
                                        {0.0, 1.0, 1.0}};

    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> m(data.size(), data.size());

    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data.size(); j++) {
            m(i, j) = data[i][j];
        }
    }

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>> es;
    es.compute(m);
    std::cout << "Eigen values" << std::endl;
    std::cout << es.eigenvalues() << std::endl;
    std::cout << "Eigen vectors" << std::endl;
    std::cout << es.eigenvectors() << std::endl;

    auto va = es.eigenvalues();
    for (auto row = 0; row < data.size(); ++row) {
        std::cout << va(row) << std::endl;
    }

}

TEST_CASE("R lib slice vecvec based on index for columns", "[r]") {
    reachi::linalg::vecvec<int> data{{1, 4, 7},
                                     {2, 5, 8},
                                     {3, 6, 9}};

    ::std::vector<int> indexes{0, 1};

    reachi::linalg::vecvec<int> expected{{1, 4},
                                         {2, 5},
                                         {3, 6}};

    auto res = reachi::linalg::slice_column_from_index_list(data, indexes);
    REQUIRE(res == expected);
}

TEST_CASE("QR decomposition", "[qr]") {
    reachi::linalg::vecvec<double> v1{{12, -51, 4},
                                      {6,  167, -68},
                                      {-4, 24,  -41}};

    reachi::linalg::vecvec<double> expected_q{{-0.857143, 0.394286,  0.331429},
                                              {-0.428571, -0.902857, -0.0342857},
                                              {0.285714,  -0.171429, 0.942857}};

    reachi::linalg::vecvec<double> expected_r{{-14,         -21,  14},
                                              {5.97812e-18, -175, 70},
                                              {4.47505e-16, 0,    -35}};

    auto res = reachi::qr::qr_decomposition(v1);
    REQUIRE(reachi::linalg::compare_vectors(res.first, expected_q, 0.01));
    REQUIRE(reachi::linalg::compare_vectors(res.second, expected_r, 0.01));
}

TEST_CASE("QR algorithm for finding eigenvalues and eigenvectors", "[qr]") {
    reachi::linalg::vecvec<double> data{{1.0, 1.0, 0.0},
                                        {1.0, 1.0, 1.0},
                                        {0.0, 1.0, 1.0}};

    auto eigen = reachi::qr::qr_algorithm(data);
    std::cout << "values" << std::endl;
    std::cout << eigen.values << std::endl;
    std::cout << "vectors" << std::endl;
    std::cout << eigen.vectors << std::endl;
    std::cout << "iterations: " << eigen.iterations << std::endl;
}

TEST_CASE("Compute distance dependent path loss", "[linkmodel]") {
    reachi::Node n1{1, {57.01266813458001, 9.994625734716218}};
    reachi::Node n2{2, {57.01266813458001, 9.9929758}};
    reachi::Node n3{3, {57.0117698, 9.9929758}};
    reachi::Node n4{4, {57.0117698, 9.994625734716218}};

    reachi::Link l6{6, n3, n4};
    reachi::Link l1{1, n1, n2};
    reachi::Link l2{2, n1, n3};
    reachi::Link l5{5, n2, n4};
    reachi::Link l3{3, n1, n4};
    reachi::Link l4{4, n2, n3};
    std::vector<reachi::Link> links{l1, l2, l3, l4, l5, l6};

    std::vector<double> l_distance{};
    std::vector<double> l_distance_expected{91.2, 99.5, 91.2, 91.2, 99.5, 91.2};
    std::for_each(links.cbegin(), links.cend(), [&l_distance](auto link) {
        l_distance.emplace_back(reachi::math::distance_pathloss(link));
    });

    REQUIRE(reachi::linalg::compare_vectors(l_distance, l_distance_expected, 0.1));
}

TEST_CASE("Compute the correlation matrix", "[linkmodel]") {
    reachi::Node n1{1, {57.01266813458001, 9.994625734716218}};
    reachi::Node n2{2, {57.01266813458001, 9.9929758}};
    reachi::Node n3{3, {57.0117698, 9.9929758}};
    reachi::Node n4{4, {57.0117698, 9.994625734716218}};

    reachi::Link l6{6, n3, n4};
    reachi::Link l1{1, n1, n2};
    reachi::Link l2{2, n1, n3};
    reachi::Link l5{5, n2, n4};
    reachi::Link l3{3, n1, n4};
    reachi::Link l4{4, n2, n3};
    std::vector<reachi::Link> links{l1, l2, l3, l4, l5, l6};

    reachi::linalg::vecvec<double> corr = reachi::math::generate_correlation_matrix_slow(links);
    reachi::linalg::vecvec<double> corr_expected{{1.0,   0.125, 0.094, 0.094, 0.125, 0.0},
                                                 {0.125, 1.0,   0.125, 0.125, 0.0,   0.125},
                                                 {0.094, 0.125, 1.0,   0.0,   0.125, 0.094},
                                                 {0.094, 0.125, 0.0,   1.0,   0.125, 0.094},
                                                 {0.125, 0.0,   0.125, 0.125, 1.0,   0.125},
                                                 {0.0,   0.125, 0.094, 0.094, 0.125, 1.0}};

    REQUIRE(reachi::linalg::compare_vectors(corr, corr_expected, 0.001));
}

TEST_CASE("Compute stochastic fading path loss", "[linkmodel]") {
    reachi::Node n1{1, {57.01266813458001, 9.994625734716218}};
    reachi::Node n2{2, {57.01266813458001, 9.9929758}};
    reachi::Node n3{3, {57.0117698, 9.9929758}};
    reachi::Node n4{4, {57.0117698, 9.994625734716218}};

    reachi::Link l6{6, n3, n4};
    reachi::Link l1{1, n1, n2};
    reachi::Link l2{2, n1, n3};
    reachi::Link l5{5, n2, n4};
    reachi::Link l3{3, n1, n4};
    reachi::Link l4{4, n2, n3};
    std::vector<reachi::Link> links{l1, l2, l3, l4, l5, l6};

    reachi::linalg::vecvec<double> corr = reachi::math::generate_correlation_matrix_slow(links);

    /* Compute link fading   */
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = std_deviation * corr;
    reachi::linalg::vecvec<double> sigma_expected{{129.96,   16.245, 12.21624, 12.21624, 16.245, 0.0},
                                                  {16.245,   129.96, 16.245,   16.245,   0.0,    16.245},
                                                  {12.21624, 16.245, 129.96,   0.0,      16.245, 12.21624},
                                                  {12.21624, 16.245, 0.0,      129.96,   16.245, 12.21624},
                                                  {16.245,   0.0,    16.245,   16.245,   129.96, 16.245},
                                                  {0.0,      16.245, 12.21624, 12.21624, 16.245, 129.96}};


    REQUIRE(reachi::linalg::compare_vectors(sigma, sigma_expected, 0.1));

    std::vector<double> gaussian_vector{-0.121966, -1.08682, 0.68429, -1.07519, 0.0332695, 0.744836};

    auto l_fading = reachi::cholesky::slow_cholesky(sigma) * gaussian_vector;
    std::vector<double> l_fading_expected{-1.39041, -12.4664, 6.17022, -13.8368, -0.158379, 6.41379};

    REQUIRE(reachi::linalg::compare_vectors(l_fading, l_fading_expected, 0.01));
}

TEST_CASE("Compute RSSI using spatial correlation", "[linkmodel]") {
    reachi::Node n1{1, {57.01266813458001, 9.994625734716218}};
    reachi::Node n2{2, {57.01266813458001, 9.9929758}};
    reachi::Node n3{3, {57.0117698, 9.9929758}};
    reachi::Node n4{4, {57.0117698, 9.994625734716218}};

    reachi::Link l6{6, n3, n4};
    reachi::Link l1{1, n1, n2};
    reachi::Link l2{2, n1, n3};
    reachi::Link l5{5, n2, n4};
    reachi::Link l3{3, n1, n4};
    reachi::Link l4{4, n2, n3};
    std::vector<reachi::Link> links{l1, l2, l3, l4, l5, l6};

    /* Compute link fading   */
    auto corr = reachi::math::generate_correlation_matrix_slow(links);
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = std_deviation * corr;
    std::vector<double> gaussian_vector{-0.121966, -1.08682, 0.68429, -1.07519, 0.0332695, 0.744836};
    auto l_fading = reachi::cholesky::slow_cholesky(sigma) * gaussian_vector;

    /* Compute distance part */
    std::vector<double> l_distance{};
    std::for_each(links.cbegin(), links.cend(), [&l_distance](auto link) {
        l_distance.emplace_back(reachi::math::distance_pathloss(link));
    });

    auto tx_dbm = 26.0;
    auto rssi = tx_dbm - (l_distance + l_fading);
    std::vector<double> l_distance_expected{91.2, 99.5, 91.2, 91.2, 99.5, 91.2};
    std::vector<double> l_fading_expected{-1.39041, -12.4664, 6.17022, -13.8368, -0.158379, 6.41379};
    auto rssi_expected = tx_dbm - (l_distance_expected + l_fading_expected);

    REQUIRE(reachi::linalg::compare_vectors(rssi, rssi_expected, 0.1));
}

TEST_CASE("Compute RSSI using spatial and temporal correlation", "[linkmodel]") {
    reachi::Node n1{1, {57.01266813458001, 9.994625734716218}};
    reachi::Node n2{2, {57.01266813458001, 9.9929758}};
    reachi::Node n3{3, {57.0117698, 9.9929758}};
    reachi::Node n4{4, {57.0117698, 9.994625734716218}};

    reachi::Link l6{6, n3, n4};
    reachi::Link l1{1, n1, n2};
    reachi::Link l2{2, n1, n3};
    reachi::Link l5{5, n2, n4};
    reachi::Link l3{3, n1, n4};
    reachi::Link l4{4, n2, n3};
    std::vector<reachi::Link> links{l1, l2, l3, l4, l5, l6};

    /* Compute link fading   */
    auto corr = reachi::math::generate_correlation_matrix_slow(links);
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = std_deviation * corr;
    std::vector<double> gaussian_vector{-0.121966, -1.08682, 0.68429, -1.07519, 0.0332695, 0.744836};
    auto l_fading = reachi::cholesky::slow_cholesky(sigma) * gaussian_vector;

    /* Compute distance part */
    std::vector<double> l_distance{};
    std::for_each(links.cbegin(), links.cend(), [&l_distance](auto link) {
        l_distance.emplace_back(reachi::math::distance_pathloss(link));
    });

    auto tx_dbm = 26.0;
    auto rssi = tx_dbm - (l_distance + l_fading);
    std::vector<double> l_distance_expected{91.2, 99.5, 91.2, 91.2, 99.5, 91.2};
    std::vector<double> l_fading_expected{-1.39041, -12.4664, 6.17022, -13.8368, -0.158379, 6.41379};
    auto rssi_expected = tx_dbm - (l_distance_expected + l_fading_expected);

    REQUIRE(reachi::linalg::compare_vectors(rssi, rssi_expected, 0.1));
}

TEST_CASE("Compute packet probability error", "[radiomodel]") {
    REQUIRE(reachi::radiomodel::pep(-105.3, 160) == Approx(0.097154).margin(0.00001));
    REQUIRE(reachi::radiomodel::pep(-104.0, 160) == Approx(0.014551).margin(0.00001));
}

TEST_CASE("Cholesky verify", "[cholesky]") {
    reachi::Node n1{1, {57.01266813458001, 9.994625734716218}};
    reachi::Node n2{2, {57.01266813458001, 9.9929758}};
    reachi::Node n3{3, {57.0117698, 9.9929758}};
    reachi::Node n4{4, {57.0117698, 9.994625734716218}};

    reachi::Link l6{6, n3, n4};
    reachi::Link l1{1, n1, n2};
    reachi::Link l2{2, n1, n3};
    reachi::Link l5{5, n2, n4};
    reachi::Link l3{3, n1, n4};
    reachi::Link l4{4, n2, n3};
    std::vector<reachi::Link> links{l1, l2, l3, l4, l5, l6};

    /* Compute link fading   */
    auto corr = reachi::math::generate_correlation_matrix_slow(links);
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = std_deviation * corr;
    auto l = reachi::cholesky::slow_cholesky(sigma);
    REQUIRE(reachi::linalg::compare_vectors(sigma, l * reachi::linalg::transpose(l), 0.00001));
}

TEST_CASE("Eigenvalues", "[math]") {
    reachi::linalg::vecvec<double> a{{4.0,   -30.0,  60.0,    -35.0},
                                     {-30.0, 300.0,  -675.0,  420.0},
                                     {60.0,  -675.0, 1620.0,  -1050.0},
                                     {-35.0, 420.0,  -1050.0, 700.0}};

    auto eigen = reachi::linalg::eig(a, 100);

    reachi::linalg::vecvec<double> eigenvector_expected{{0.0291933, 0.179186,  0.582076,  0.792608},
                                                        {-0.328712, -0.741918, -0.370502, 0.451923},
                                                        {0.791411,  0.100228,  -0.509579, 0.322416},
                                                        {-0.514553, 0.638283,  -0.514048, 0.252161}};
    std::vector<double> eigenvalues_expected{2585.25,
                                             37.1015,
                                             1.47805,
                                             0.166643};
    REQUIRE(reachi::linalg::compare_vectors(eigen.vectors, eigenvector_expected, 0.0001));
    REQUIRE(reachi::linalg::compare_vectors(eigen.values, eigenvalues_expected, 0.0001));

    /* Test with a diagonal matrix */
    reachi::linalg::vecvec<double> b{{4.0, 0.0, 0.0, 0.0},
                                     {0.0, 1.0, 0.0, 0.0},
                                     {0.0, 0.0, 3.0, 0.0},
                                     {0.0, 0.0, 0.0, 2.0}};

    auto b_eigen = reachi::linalg::eig(b, 100);
    std::vector<double> bva_expected{4, 3, 2, 1};
    reachi::linalg::vecvec<double> bve_expected{{1.0, 0.0, 0.0, 0.0},
                                                {0.0, 0.0, 0.0, 1.0},
                                                {0.0, 1.0, 0.0, 0.0},
                                                {0.0, 0.0, 1.0, 0.0}};
    REQUIRE(reachi::linalg::compare_vectors(b_eigen.vectors, bve_expected, 0.0001));
    REQUIRE(reachi::linalg::compare_vectors(b_eigen.values, bva_expected, 0.0001));

    /* Use the discretized second derivative matrix */
    reachi::linalg::vecvec<double> c{{-2.0, 1.0,  0.0,  0.0,  0.0},
                                     {1.0,  -2.0, 1.0,  0.0,  0.0},
                                     {0.0,  1.0,  -2.0, 1.0,  0.0},
                                     {0.0,  0.0,  1.0,  -2.0, 1.0},
                                     {0.0,  0.0,  0.0,  1.0,  -2.0}};

    auto c_eigen = reachi::linalg::eig(c, 100);
    std::vector<double> cva_expected{-0.267949, -1, -2, -3, -3.73205};
    reachi::linalg::vecvec<double> cve_expected{{0.288675, -0.5,        0.57735,      0.5,         0.288675},
                                                {0.5,      -0.5,        -4.44985e-17, -0.5,        -0.5},
                                                {0.57735,  -7.0576e-17, -0.57735,     5.05017e-17, 0.57735},
                                                {0.5,      0.5,         1.86451e-16,  0.5,         -0.5},
                                                {0.288675, 0.5,         0.57735,      -0.5,        0.288675}};

    REQUIRE(reachi::linalg::compare_vectors(c_eigen.vectors, cve_expected, 0.1));
    REQUIRE(reachi::linalg::compare_vectors(c_eigen.values, cva_expected, 0.0001));
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
    std::vector links{l1, l2, l3, l4, l5, l6};
    auto corr = generate_correlation_matrix(links);
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = corr * std_deviation;

    auto temp = cholesky(sigma);
    std::cout << "" << std::endl;
    for (const auto &item : temp) {
        std::cout << "id: " << item.first.first << ", " << item.first.second << "\tvalue: " << std::to_string(item.second) << std::endl;
    }*/



    // std::cout << measure<>::execution(generate_correlation_matrix_slow, links) << std::endl;
    // std::cout << measure<>::execution(generate_correlation_matrix, links) << std::endl;

    auto upper = mpilib::geo::Location{57.01266813458001, 9.994625734716218};
    auto lower = mpilib::geo::Location{57.0117698, 9.9929758};
    auto nodes = reachi::data::generate_nodes(15, upper, lower);
    auto links = reachi::data::create_link_vector(nodes, MAX_LINK_DISTANCE);

    auto corr = reachi::math::generate_correlation_matrix(links);
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = corr * std_deviation;
}

TEST_CASE("Next power of 2", "[svd]") {
    uint32_t data_1 = 6;
    uint32_t data_2 = 988;

    uint64_t expected_1 = 8;
    uint64_t expected_2 = 1024;

    auto res_1 = reachi::math::next_power_of_2(data_1);
    auto res_2 = reachi::math::next_power_of_2(data_2);
    REQUIRE(res_1 == expected_1);
    REQUIRE(res_2 == expected_2);
}

TEST_CASE("SVD verification", "[svd]") {
    reachi::linalg::vecvec<double> data{{2, 5, 3},
                                        {2, 4, 2},
                                        {2, 2, 5}};

    std::vector<double> expected_s{9.302, 2.884, 0.372};
    reachi::linalg::vecvec<double> expected_u{{-0.651, -0.389, 0.651},
                                              {-0.510, -0.411, -0.755},
                                              {-0.562, 0.824,  -0.069}};

    reachi::linalg::vecvec<double> expected_v{{-0.370, -0.690, -0.621},
                                              {0.016,  -0.674, 0.738},
                                              {-0.928, 0.263,  0.260}};
    auto res = reachi::svd::svd(data, 20);
    REQUIRE(reachi::linalg::compare_vectors(std::get<0>(res), expected_s, 0.01));
    //REQUIRE(reachi::linalg::compare_vectors(std::get<1>(res), expected_u, 0.1));
    //REQUIRE(reachi::linalg::compare_vectors(std::get<2>(res), expected_v, 0.1));

    /*std::cout << "singular values [0]" << std::endl;
    std::cout << std::get<0>(res) << std::endl;
    std::cout << "u [1]" << std::endl;
    std::cout << std::get<1>(res) << std::endl;
    std::cout << "v [2]" << std::endl;
    std::cout << std::get<2>(res) << std::endl;*/
}


TEST_CASE("Generate Aloha line topology", "[aloha]") {
    mpilib::geo::Location l{57.01266813458001, 9.994625734716218};
    auto nodes = reachi::data::generate_line_topology(l, 100);

    std::cout << "#!/usr/bin/env bash\nmpirun -n 1 ctrlr/ctrlr $2 : \\" << std::endl;
    for (const auto &node : nodes) {
        std::cout << "    -n 1 $1/$1 " << node.get_location().get_latitude() << " "
                  << node.get_location().get_longitude() << " : \\" << std::endl;
    }
    std::cout << "    -n 1 lmc/lmc $2" << std::endl;
}

TEST_CASE("Generate Aloha ring topology", "[aloha]") {
    mpilib::geo::Location l{57.01266813458001, 9.994625734716218};
    auto nodes = reachi::data::generate_ring_topology(l, 500_m, 100);

    /* std::cout << "{" << std::endl;
    for (const auto &node : nodes) {
        std::cout << "\""
                  << node.get_id()
                  << "\""
                  << ": {\"lat\": "
                  << node.get_location().get_latitude()
                  << ",\"lon\": "
                  << node.get_location().get_longitude()
                  << ",\"color\": \"rgb(220, 20, 60)\"},"
                  << std::endl;
    }
    std::cout << "}" << std::endl ;*/

    std::cout << "#!/usr/bin/env bash\nmpirun -n 1 ctrlr/ctrlr $2 : \\" << std::endl;
    for (const auto &node : nodes) {
        std::cout << "    -n 1 $1/$1 " << node.get_location().get_latitude() << " "
                  << node.get_location().get_longitude() << " : \\" << std::endl;
    }
    std::cout << "    -n 1 lmc/lmc $2" << std::endl;
}