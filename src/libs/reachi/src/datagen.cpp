#include <random>
#include <mpilib/httpclient.h>
#include <mpilib/helpers.h>
#include <geo/geo.h>

#include <reachi/datagen.h>

using json = nlohmann::json;

std::vector<reachi::Node>
reachi::data::generate_nodes(unsigned long count, geo::Location &upper, geo::Location &lower) {
    std::vector<reachi::Node> nodes{};
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
        reachi::Node n{i, l};
        nodes.emplace_back(n);
    }

    return nodes;
}

std::vector<reachi::Node>
reachi::data::generate_cluster(geo::Location &center, uint32_t begin, unsigned long count,
                               double radius /* kilometer */) {
    std::vector<reachi::Node> nodes{};
    nodes.reserve(count);

    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<double> dist{0.0, 1.0};
    auto gen = std::bind(dist, eng);

    for (uint32_t i = begin; i < begin + count; ++i) {
        auto r = radius / KM_PER_DEGREE; /* Convert radius to degrees */
        auto u = gen();
        auto v = gen();

        auto w = r * std::sqrt(u);
        auto t = 2 * M_PI * v;
        auto x = (w * std::cos(t)) / std::cos(center.get_longitude());
        auto y = w * std::sin(t);

        reachi::Node n{i, {center.get_latitude() + x, center.get_longitude() + y}};
        nodes.emplace_back(n);
    }

    return nodes;
};

std::vector<reachi::Link>
reachi::data::create_link_vector(std::vector<reachi::Node> &nodes, double threshold /* kilometers */) {
    std::vector<Link> links{};

    for (uint32_t i = 0; i < nodes.size(); ++i) {
        for (uint32_t j = i; j < nodes.size(); ++j) {
            if (i == j) {
                continue;
            }

            Link l{mpilib::generate_link_id(i, j), nodes[i], nodes[j]};
            if (l.get_distance() < threshold or threshold <= 0.01) {
                links.emplace_back(l);
            }
        }
    }

    return links;
}

std::vector<reachi::Optics::CLink>
reachi::data::create_link_vector(std::vector<Optics::Cluster> &clusters, double threshold /* km */) {
    std::vector<Optics::CLink> links{};
    uint64_t id_count = 0;

    for (uint32_t i = 0; i < clusters.size(); ++i) {
        for (uint32_t j = i; j < clusters.size(); ++j) {
            if (i == j) {
                continue;
            }

            Optics::CLink l{id_count, clusters[i], clusters[j]};
            if (l.get_distance() < threshold or threshold <= 0.000001) {
                links.emplace_back(l);
                id_count++;
            }
        }
    }

    return links;
}


::std::vector<reachi::Node> reachi::data::generate_line_topology(geo::Location start, double distance, int size) {
    ::std::vector<reachi::Node> nodes{reachi::Node{0, start}};

    for (uint32_t i = 1; i < size; ++i) {
        auto loc = start;
        loc.move(0, distance * i, 90.0);
        nodes.emplace_back(reachi::Node{i, loc});
    }

    return nodes;
}

::std::vector<reachi::Node>
reachi::data::generate_ring_topology(geo::Location start, double distance, double size) {
    ::std::vector<reachi::Node> nodes{};
    double degree = 0.0;
    double step = 360.0 / size;
    uint32_t id = 0;

    while (degree < 360) {
        start.move(0, distance, degree);
        nodes.emplace_back(reachi::Node{id, start});
        id++;
        degree += step;
    }

    return nodes;
}


void reachi::data::visualise_nodes(std::vector<reachi::Node> &nodes) {
    visualise_nodes(nodes, 10000);
}

void reachi::data::visualise_nodes(std::vector<reachi::Node> &nodes, unsigned long chunk_size) {
    mpilib::HttpClient httpclient{"http://localhost:8050"};
    httpclient.get("/clear");

    mpilib::for_each_interval(nodes.begin(), nodes.end(), chunk_size, [&httpclient, &chunk_size](auto from, auto to) {
        std::vector<reachi::Node> node_chunk{from, to};

        json j = node_chunk;
        httpclient.post_async("/updatechunk", j);
    });
}

void reachi::data::visualise_clusters(std::vector<reachi::Optics::Cluster> clusters) {
    mpilib::HttpClient httpclient{"http://localhost:8050"};

    std::vector<json> serialized_clusters{};
    serialized_clusters.reserve(clusters.size());

    for (auto &cluster : clusters) {
        std::vector<int> nodes{};
        nodes.reserve(cluster.get_nodes().size());

        for (auto &node : cluster.get_nodes()) {
            nodes.emplace_back(node.get_id());
        }

        json c = nodes;
        serialized_clusters.emplace_back(c);
    }

    json j = serialized_clusters;
    httpclient.post_async("/colornodes", j);
}