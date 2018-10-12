#include <catch.hpp>

#include <iostream>
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

TEST_CASE("Compute the angle between two GPS coordinates", "[geomath]") {
    Location pos1{57.01266813458001, 9.994625734716218};
    Location pos2{57.01266813458001, 9.9929758};
    Location pos3{57.0117698, 9.9929758};
    Location pos4{57.0117698, 9.994625734716218};

    Location pos40{14.6294310000000, 121.098663000000};
    Location pos41{14.6294830000000, 121.099789000000};
    Location pos43{14.6290680000000, 121.095302000000};

    /*std::cout << angle_between(pos1, pos2, pos3) << std::endl;*/
    /*std::cout << angle_between(pos2, pos1, pos3) << std::endl;*/
    /*std::cout << angle_between(pos4, pos2, pos3) << std::endl;*/
    /*std::cout << angle_between(pos1, pos4, pos3) << std::endl;*/

    REQUIRE(angle_between(pos1, pos2, pos3) == Approx(45.0).margin(0.1));



    /*REQUIRE(angle_between(pos40, pos41, pos43) == Approx(180.0).margin(5.0));*/
    /*std::cout << angle_between(pos40, pos41, pos43) << std::endl;*/
}