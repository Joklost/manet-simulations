#include <random>
#include <functional>

#include <reachi2/datagen.h>

std::vector<reachi2::Node>
reachi2::data::generate_nodes(unsigned long count, geo::Location &upper, geo::Location &lower) {
    std::vector<reachi2::Node> nodes{};
    nodes.reserve(count);

    auto lat_min = lower.get_latitude();
    auto lat_max = upper.get_latitude();
    std::random_device rd_lat;
    std::default_random_engine eng_lat(rd_lat());
    std::uniform_real_distribution<double> dist_lat{lat_min, lat_max};
    auto gen_lat = std::bind(dist_lat, eng_lat);

    auto lon_min = lower.get_longitude();
    auto lon_max = upper.get_longitude();
    std::random_device rd_lon;
    std::default_random_engine eng_lon(rd_lon());
    std::uniform_real_distribution<double> dist_lon{lon_min, lon_max};
    auto gen_lon = std::bind(dist_lon, eng_lon);

    for (uint32_t i = 1; i <= count; ++i) {
        geo::Location l{gen_lat(), gen_lon()};
        reachi2::Node n{i, l};
        nodes.emplace_back(n);
    }

    return nodes;
}

std::vector<reachi2::Link> reachi2::data::create_links(std::vector<reachi2::Node> nodes, double threshold) {
    unsigned int link_id = 0;
    auto n_size = nodes.size();
    std::vector<reachi2::Link> links;

    for (auto i = 0; i < n_size; ++i) {
        for (auto j = i; j < n_size; ++j) {
            if (i == j)
                continue;

            reachi2::Link l(link_id, nodes[i], nodes[j]);
            if (l.distance < threshold or threshold <= 0.01) {
                link_id++;
                links.emplace_back(l);
            }
        }
    }

    return links;
}
