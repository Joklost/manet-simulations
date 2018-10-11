#include <catch.hpp>

#include <iostream>

#include "mpilib/math.h"
#include "mpilib/geomath.h"
#include "mpilib/helpers.h"

TEST_CASE("Compute distance between two GPS coordinates", "[geomath]") {
    Location l1{57.01266813458001, 9.994625734716218};
    Location l2{57.01266813458001, 9.9929758};
    REQUIRE(distance_between(l1, l2) == Approx(0.100).margin(0.001));
}

TEST_CASE("Compute bearing between two GPS coordinates", "[geomath]") {
    Location l1{57.01266813458001, 9.994625734716218};
    Location l2{57.01266813458001, 9.9929758};
    REQUIRE(bearing_between(l1, l2) == Approx(90.0).margin(1.0));
}

TEST_CASE("Compute the Cholesky decomposition", "[math]") {
    vecvec<double> matrix{{25.0, 15.0, -5.0},
                          {15.0, 18.0, 0.0},
                          {-5.0, 0.0,  11.0}};

    vecvec<double> result{{5.0,  0.0, 0.0},
                          {3.0,  3.0, 0.0},
                          {-1.0, 1.0, 3.0}};
    REQUIRE(result == cholesky(matrix));
}

TEST_CASE("Generate a Gaussian Vector", "[math]") {
    auto size = 1000000u;
    auto vec = gaussian_vector(0.0, std::pow(11.4, 2), size);
    REQUIRE(vec.size() == size);
}
