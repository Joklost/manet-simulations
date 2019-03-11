#include <iostream>
#include <string>
#include <cstdio>
#include <unistd.h>

#include <catch2/catch.hpp>

#include <linkaiders/linkmodel.h>
#include "../src/model.h"

void *get_test_model() {
    char logpath[] = "gpslog.txt";
    return initialize(33, 2, logpath);
}

TEST_CASE("Initialize link model object", "[linkaiders/linkmodel]") {
    void *model = get_test_model();
    REQUIRE(model);
    deinit(model);

    model = initialize(0, 0, nullptr);
    REQUIRE_FALSE(model);
}

TEST_CASE("Topology generation", "[linkaiders/linkmodel]") {
    void *model = get_test_model();
    auto *linkmodel = static_cast<LinkModel *>(model);

    auto &topologies = linkmodel->topologies;

    for (const auto &topology : topologies) {
        auto time = topology.first;
        auto &links = topology.second;

        for (const auto &link : links) {
            auto &nodes = link.get_nodes();

            CHECK_FALSE(nodes.first == nodes.second);
            CHECK(nodes.first.current_location.get_time() <= time);
            CHECK(nodes.second.current_location.get_time() <= time);
        }
    }

    deinit(model);
}


TEST_CASE("Topology connectedness", "[linkaiders/linkmodel]") {
    void *model = get_test_model();
    REQUIRE(is_connected(model, 41, 42, 1510737880));
    REQUIRE_FALSE(is_connected(model, 10, 44, 1510715240));
    deinit(model);
}