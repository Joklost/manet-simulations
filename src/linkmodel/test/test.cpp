#include <catch.hpp>
#include <iostream>
#include <mpilib/helpers.h>

#include "linkmodel/cholesky.h"
#include "linkmodel/math.h"

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
    auto vec = gaussian_vector(0.0, 1.0, size);
    REQUIRE(vec.size() == size);
}

TEST_CASE("Multiplication operator for 4x3 and 3x4 matrices", "[math]") {
    vecvec<int> m1{{-1, 1,  4},
                   {6,  -4, 2},
                   {-3, 5,  0},
                   {3,  7,  -2}};

    vecvec<int> m2{{-1, 1,  4,  8},
                   {6,  9,  10, 2},
                   {11, -4, 5,  -3}};

    vecvec<int> result{{51, -8,  26, -18},
                       {-8, -38, -6, 34},
                       {33, 42,  38, -14},
                       {17, 74,  72, 44}};

    REQUIRE((m1 * m2) == result);
}

TEST_CASE("Multiply 4x1 and 1x4 matrices", "[math]") {
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


TEST_CASE("Compute distance dependent path loss", "[network]") {
    Node n1{1, {57.01266813458001, 9.994625734716218}};
    Node n2{2, {57.01266813458001, 9.9929758}};
    Node n3{3, {57.0117698, 9.9929758}};
    Node n4{4, {57.0117698, 9.994625734716218}};

    Link l1{n1, n2};
    Link l2{n1, n3};
    Link l3{n1, n4};
    Link l4{n2, n3};
    Link l5{n2, n4};
    Link l6{n3, n4};

    std::vector links{l1, l2, l3, l4, l5, l6};
    std::vector<double> links_d{};
    std::vector<double> links_d_expected{91.2, 99.5, 91.2, 91.2, 99.5, 91.2};
    std::for_each(links.cbegin(), links.cend(), [&links_d](auto link) {
        links_d.emplace_back(distance_pathloss(link));
    });

    REQUIRE(compare_vectors(links_d, links_d_expected, 0.1));
}