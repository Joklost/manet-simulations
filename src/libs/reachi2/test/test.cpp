#include <catch2/catch.hpp>
#include <Eigen/Core>
#include <iostream>
#include <chrono>

#include <geo/geomath.h>
#include <geo/location.h>
#include <reachi2/datagen.h>
#include <reachi2/math.h>
#include <reachi2/constants.h>
#include <reachi2/linkmodel.h>

using namespace std::literals::chrono_literals;
using namespace geo::literals;


TEST_CASE("Linkmodel test", "[linkmodel]") {
    geo::Location upper{57.01266813458001, 10.994625734716218};
    auto lower = geo::square(upper, 1_km);

    auto nodes = reachi2::data::generate_nodes(10, upper, lower);
    auto links = reachi2::data::create_links(nodes);

    reachi2::Linkmodel linkmodel {links};
    linkmodel.compute();
    auto model = linkmodel.get_pep();
    auto res = linkmodel.distance_pathloss(100);
    std::cout << res << std::endl;

//    for (const auto &item : model) {
//        std::cout << item.second << std::endl;
//    }
}