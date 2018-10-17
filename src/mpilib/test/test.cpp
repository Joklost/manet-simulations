#include <catch.hpp>

#include "mpilib/geomath.h"
#include "mpilib/helpers.h"

TEST_CASE("Compute distance between two GPS coordinates", "[geomath]") {
    Location l1{0, 57.01266813458001, 9.994625734716218};
    Location l2{0, 57.01266813458001, 9.9929758};
    REQUIRE(distance_between(l1, l2) == Approx(0.100).margin(0.001));
}

TEST_CASE("Compute bearing between two GPS coordinates", "[geomath]") {
    Location l1{0, 57.01266813458001, 9.994625734716218};
    Location l2{0, 57.01266813458001, 9.9929758};
    REQUIRE(bearing_between(l1, l2) == Approx(90.0).margin(1.0));
}

TEST_CASE("Compute the angle between two GPS coordinates", "[geomath]") {
    Location p1{0, 57.01266813458001, 9.994625734716218};
    Location p2{0, 57.01266813458001, 9.9929758};
    Location p3{0, 57.0117698, 9.9929758};
    REQUIRE(angle_between(p1, p2, p3) == Approx(45.0).margin(0.1));
}

TEST_CASE("Compute distance between two GPS coordinates after moving one 1.5 km", "[geomath]") {
    Location l1{0, 57.01266813458001, 9.994625734716218};
    Location l2{0, 57.01266813458001, 9.9929758};
    REQUIRE(distance_between(l1, l2) == Approx(0.100).margin(0.001));
    l2.move(10, 1.5, 270.0);
    REQUIRE(distance_between(l1, l2) == Approx(1.600).margin(0.01));
}
