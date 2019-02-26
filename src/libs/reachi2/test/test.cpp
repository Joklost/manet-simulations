#include <catch2/catch.hpp>
#include <Eigen/Core>
#include <iostream>
#include <chrono>

#include <reachi/datagen.h>
#include <mpilib/geomath.h>
#include <mpilib/location.h>
#include "reachi2/math.h"
#include "reachi2/constants.h"
#include "reachi2/linkmodel.h"

using namespace std::literals::chrono_literals;
using namespace mpilib::geo::literals;

TEST_CASE("Linkmodel test", "[linkmodel]") {
    mpilib::geo::Location upper{57.01266813458001, 10.994625734716218};
    auto lower = mpilib::geo::square(upper, 5_km);

    auto nodes = reachi::data::generate_nodes(10, upper, lower);

    reachi2::Linkmodel linkmodel {nodes};
}