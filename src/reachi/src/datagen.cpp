#include <random>
#include <mpilib/httpclient.h>
#include <mpilib/helpers.h>
#include <json.hpp>

#include <reachi/datagen.h>

using json = nlohmann::json;

std::vector<Node> generate_nodes(unsigned long count, Location &upper, Location &lower) {
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

std::vector<Link> create_link_vector(std::vector<Node> &nodes, double threshold /* kilometers */) {
    std::vector<Link> links{};

    for (uint32_t i = 0; i < nodes.size(); ++i) {
        for (uint32_t j = i; j < nodes.size(); ++j) {
            if (i == j) {
                continue;
            }

            Link l{generate_link_id(i, j), nodes[i], nodes[j]};
            if (l.get_distance() < threshold or threshold <= 0.01) {
                links.emplace_back(l);
            }
        }
    }

    return links;
}

void visualise_nodes(std::vector<Node> &nodes) {
    visualise_nodes(nodes, 10000);
}

void visualise_nodes(std::vector<Node> &nodes, unsigned long chunk_size) {
    HttpClient httpclient{"http://localhost:8050"};
    httpclient.get("/clear");

    for_each_interval(nodes.begin(), nodes.end(), chunk_size, [&httpclient, &chunk_size](auto from, auto to) {
        std::vector<json> serialized_nodes{};
        serialized_nodes.reserve(chunk_size);

        std::for_each(from, to, [&httpclient, &serialized_nodes](Node el) {
           serialized_nodes.emplace_back(el.serialize());
        });

        json j = serialized_nodes;
        httpclient.post_async("/updatechunk", j);
    });
}
