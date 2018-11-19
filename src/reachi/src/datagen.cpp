#include <random>
#include <mpilib/httpclient.h>
#include <mpilib/helpers.h>
#include "datagen.h"


std::vector<Node>   generate_nodes(unsigned long count, Location &upper, Location &lower) {
    std::vector<Node> nodes{};
    nodes.reserve(count);

    auto lat_min = lower.get_latitude();
    auto lat_max = upper.get_latitude();
    std::random_device rd_lat;
    std::default_random_engine eng_lat(rd_lat());
    std::uniform_real_distribution dist_lat{lat_min, lat_max};
    auto gen_lat = std::bind(dist_lat, eng_lat);

    auto lon_min = lower.get_longitude();
    auto lon_max = upper.get_longitude();
    std::random_device rd_lon;
    std::default_random_engine eng_lon(rd_lon());
    std::uniform_real_distribution dist_lon{lon_min, lon_max};
    auto gen_lon = std::bind(dist_lon, eng_lon);

    for (uint32_t i = 0; i < count; ++i) {
        Location l{gen_lat(), gen_lon()};
        Node n{i, l};
        nodes.emplace_back(n);
    }

    return nodes;
}

std::vector<Link> generate_links(std::vector<Node> &nodes) {
    std::vector<Link> links{};
    links.reserve((nodes.size() * (nodes.size() + 1)) / 2 - nodes.size());

    for (uint32_t i = 0; i < nodes.size(); ++i) {
        for (uint32_t j = i; j < nodes.size(); ++j) {
            if (i == j) {
                continue;
            }
            Link l{generate_link_id(i, j), nodes[i], nodes[j]};
            links.emplace_back(l);
        }
    }

    return links;
}

void visualise_nodes(std::vector<Node> &nodes) {
    HttpClient httpclient{"http://localhost:8050"};

}
