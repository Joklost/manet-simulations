#include <iostream>
#include <string>
#include <cstdio>
#include <unistd.h>

#include <catch2/catch.hpp>

#include <linkaiders/linkmodel.h>

void *get_test_model() {
    char logpath[] = "gpslog.txt";
    return initialize(33, 2, logpath);
}

TEST_CASE("Initialise link model object with cast", "[linkaiders/linkmodel]") {
    void *model = get_test_model();
    REQUIRE(model);
    deinit(model);
}