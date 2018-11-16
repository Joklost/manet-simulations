
#include <catch.hpp>
#include <mpilib/location.h>
#include "datagen.h"

TEST_CASE("Random Number Generator with Mapping", "[clustering]") {
    Location upper{48.0, -92.0};
    Location lower{33.0, -117.0};

    auto nodes = generate_nodes(5000, upper, lower);
    auto links = generate_links(nodes);

    REQUIRE(links.size() == (nodes.size() * (nodes.size() + 1)) / 2 - nodes.size());
}
