#include <catch2/catch.hpp>
#include <Eigen/Core>
#include <iostream>
#include <chrono>

#include <geo/geo.h>
#include <geo/geo.h>
#include "reachi2/datagen.h"
#include "reachi2/math.h"
#include "reachi2/constants.h"
#include "reachi2/linkmodel.h"

using namespace std::literals::chrono_literals;
using namespace geo::literals;



TEST_CASE("Linkmodel test", "[linkmodel]") {
    geo::Location upper{57.01266813458001, 10.994625734716218};
    auto lower = geo::square(upper, 1_km);
    auto TX_DBM = 26.0;

    auto nodes = reachi2::data::generate_nodes(10, upper, lower);

    reachi2::Linkmodel linkmodel {nodes};
    linkmodel.compute();
    auto res = linkmodel.get_model();

    for (auto &link : res) {
        link.second = TX_DBM - link.second;
        std::cout << link.second << std::endl;
    }




  /*  for (const auto &item : res) {
        std::cout << item.second << std::endl;
    }*/
}