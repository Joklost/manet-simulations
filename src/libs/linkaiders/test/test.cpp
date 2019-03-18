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
    auto *linkmodel = static_cast<linkaiders::LinkModel *>(model);

    auto &topologies = linkmodel->topologies;

    REQUIRE(topologies.size() == 45);

    for (const auto &topology : topologies) {
        auto time = topology.first;
        auto &links = topology.second.links;

        for (const auto &link : links) {
            auto &nodes = link.get_nodes();

            CHECK_FALSE(nodes.first == nodes.second);
            CHECK(nodes.first.current_location.get_time() <= time);
            CHECK(nodes.second.current_location.get_time() <= time);
            CHECK_FALSE(link.distance == Approx(0.0));
        }
    }

    deinit(model);
}


TEST_CASE("Topology connectedness", "[linkaiders/linkmodel]") {
    void *model = get_test_model();
    auto *linkmodel = static_cast<linkaiders::LinkModel *>(model);

    for (const auto &topology : linkmodel->topologies) {
        auto time = topology.first;
        auto &links = topology.second.links;

        for (const auto &link : links) {
            auto &node1 = link.get_nodes().first;
            auto &node2 = link.get_nodes().second;

            if ((linkaiders::TX_POWER - link.distance) < linkaiders::DISTANCE_THRESHOLD) {
                CHECK_FALSE(is_connected(model, node1.get_id(), node2.get_id(), time));
            } else {
                CHECK(is_connected(model, node1.get_id(), node2.get_id(), time));
            }
        }
    }

    deinit(model);
}