#include <iostream>
#include <iomanip>
#include <future>

#include <mpilib/node.h>
#include <mpilib/helpers.h>
#include <reachi/datagen.h>
#include <reachi/math.h>
#include <mpilib/httpclient.h>
#include <reachi/cholesky.h>
#include <reachi/radiomodel.h>
#include "math.h"

#define MIN_PTS 2
#define MIN_EPS 0.01
#define MAX_EPS 0.10
#define EPS_STEP 0.1
#define LINK_THRESHOLD 0.150
#define TX_DBM -20.0
#define AREA 1.0
#define NODES 100

#define TRY try {

#define CATCH   } catch (const char * e) {           \
                    std::cout << e << std::endl;     \
                    return 0;                        \
                }


std::vector<Optics::Cluster> cluster(std::vector<Node> &nodes, double eps, int minpts) {
    Optics optics{};

    auto ordering = optics.compute_ordering(nodes, eps, minpts);
    auto clusters = optics.cluster(ordering);

    return clusters;
}


void print_header() {
    std::cout
            << std::setw(12) << "Run"
            << std::setw(12) << "Clusters"
            << std::setw(12) << "Epsilon"
            << std::setw(12) << "Nodes"
            << std::setw(12) << "Links"
            << std::setw(12) << "Delta"
            << std::setw(12) << "Time"
            << std::endl;
}

void print_result(int graph_id, long cluster_count, double eps, long node_count, long link_count, double delta,
                  long duration) {
    double us = duration / 1000.0;
    long ms = static_cast<long>(std::round(us));
    std::cout
            << std::setprecision(3)
            << std::fixed
            << std::setw(8) << "" << std::setw(4) << std::setfill('0') << graph_id << std::setfill(' ')
            << std::setw(12) << cluster_count
            << std::setw(9) << eps << " km"
            << std::setw(12) << node_count
            << std::setw(12) << link_count
            << std::setw(12) << delta;

    if (ms > 100) {
        std::cout
                << std::setw(9) << ms << " ms";
    } else {
        std::cout
                << std::setw(9) << us << " ms";
    }
    std::cout
            << std::endl;
}

void request_graph(HttpClient &httpclient, std::vector<Optics::Cluster> clusters, int graph_id, double eps) {
    std::vector<json> serialized_clusters{};
    serialized_clusters.reserve(clusters.size());

    for (auto &cluster : clusters) {
        std::vector<int> node_ids{};
        node_ids.reserve(cluster.get_nodes().size());

        for (auto &node : cluster.get_nodes()) {
            node_ids.emplace_back(node.get_id());
        }

        json c = node_ids;
        serialized_clusters.emplace_back(c);
    }

    json json_clusters = {
            {"clusters", serialized_clusters},
            {"params",   {
                                 {"id", graph_id},
                                 {"minpts", MIN_PTS},
                                 {"eps", eps},
                                 {"count", clusters.size()}
                         }}
    };

    httpclient.post("/request-graph", json_clusters);
}

long clusterize_remaining(const std::vector<Node> &nodes, std::vector<Optics::Cluster> &clusters) {
    auto cnodes = nodes;
    unsigned long node_count = 0;
    for (auto &cluster : clusters) {
        node_count += cluster.size();

        for (auto &node : cluster.get_nodes()) {
            if (cluster.contains(node)) {
                cnodes.erase(std::remove(cnodes.begin(), cnodes.end(), node), cnodes.end());
            }
        }
    }
    node_count = nodes.size() - node_count + clusters.size();
    auto id = static_cast<uint32_t>(clusters.size());
    for (auto &node : cnodes) {
        id++;
        std::vector<Node> single_node_cluster{node};
        Optics::Cluster c{id, single_node_cluster};
        clusters.push_back(c);
    }

    assert(node_count == clusters.size());
    return node_count;
}

Location create_square(Location &upper, double size) {
    return move_location(move_location(upper, size, 180), size, 90);
}

int main(int argc, char *argv[]) {
    std::vector<std::future<void>> futures{};
    std::vector<std::thread> threads{};

    Location upper{57.0134, 9.99008};
    Location lower = create_square(upper, AREA);

    auto setup_start = std::chrono::steady_clock::now();
    /**/

    auto nodes = generate_nodes(NODES, upper, lower);

    std::vector<Optics::Cluster> og_clusters{};
    og_clusters.reserve(nodes.size());

    for (auto &node : nodes) {
        std::vector<Node> cluster{node};
        og_clusters.emplace_back(node.get_id(), cluster);
    }
    auto og_links = create_link_vector(og_clusters, LINK_THRESHOLD);
    std::cout << og_links.size() << std::endl;

    /* Compute distance part */
    std::vector<double> og_l_distance{};
    std::for_each(og_links.cbegin(), og_links.cend(), [&og_l_distance](Optics::CLink link) {
        og_l_distance.emplace_back(distance_pathloss(link.get_distance()));
    });

    /* Compute fading part */
    auto og_corr = generate_correlation_matrix(og_links);
    std::cout << "pd: " << is_positive_definite(og_corr) << std::endl;

    if (!is_positive_definite(og_corr)) {
//        og_corr = iterative_spectral(og_corr, 25, 0.001);
return 0;
    }

    std::cout << "pd: "  << is_positive_definite(og_corr) << std::endl;

//    auto setup_end1 = std::chrono::steady_clock::now();
//    auto setup_duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(setup_end1 - setup_start);


    auto std_deviation = std::pow(11.4, 2);
    auto og_sigma = std_deviation * og_corr;
    auto og_gaussian = generate_gaussian_vector(0.0, 1.0, og_links.size());
    auto og_choleskied = cholesky_v2(og_sigma);
    assert(compare_vectors(og_sigma, og_choleskied * transpose(og_choleskied), 0.00001));
    auto og_l_fading = og_choleskied * og_gaussian;

    auto og_rssi = TX_DBM - (og_l_distance + og_l_fading);

    std::vector<double> og_pep{};
    og_pep.reserve(og_links.size());

    for (auto &rssi : og_rssi) {
        og_pep.push_back(packet_error_probability(rssi, 160));
    }

    /**/
    auto setup_end = std::chrono::steady_clock::now();
    auto setup_duration = std::chrono::duration_cast<std::chrono::microseconds>(setup_end - setup_start);

    HttpClient httpclient{"http://0.0.0.0:5000/vis"};
    json j_nodes = nodes;
    httpclient.post("/add-nodes", j_nodes);

    auto eps_steps = static_cast<unsigned long>(std::round(MAX_EPS / EPS_STEP));

    print_header();

    auto m = 0;

    print_result(0, 0, 0.0, nodes.size(), og_links.size(), 0.0, setup_duration.count());

    for (int i = 0; i < eps_steps; ++i) {
        m++;
        auto eps = MIN_EPS + i * EPS_STEP;

        auto start = std::chrono::steady_clock::now();
        /**/

        auto clusters = cluster(nodes, eps, MIN_PTS);

        auto cluster_count = clusters.size();
        auto node_count = clusterize_remaining(nodes, clusters);
        auto links = create_link_vector(clusters, LINK_THRESHOLD);

        /* Compute distance part */
        std::vector<double> l_distance{};
        std::for_each(links.cbegin(), links.cend(), [&l_distance](Optics::CLink link) {
            l_distance.emplace_back(distance_pathloss(link.get_distance()));
        });

        /* Compute fading part */
        auto corr = generate_correlation_matrix(links);
        auto sigma = std_deviation * corr;
        auto gaussian = generate_gaussian_vector(0.0, 1.0, links.size());
        auto l_fading = our_cholesky(sigma) * gaussian;

        std::vector<double> rssi{TX_DBM - (l_distance + l_fading)};

        assert(rssi.size() == links.size());
        for (int j = 0; j < rssi.size(); ++j) {
            links[j].set_rssi(rssi[j]);
        }

        for (auto &og_link : og_links) {
            auto first = og_link.get_clusters().first.get_nodes().front();
            auto second = og_link.get_clusters().second.get_nodes().front();
            auto k = 0;
            for (auto &link : links) {
                k++;
                if (!link.contains(first, second)) {
                    continue;
                }
                link.set_pep(packet_error_probability(link.get_rssi(), 160));
                std::cout
                          << link.get_rssi()
                          << " : "
                          << link.get_pep()
                          << std::endl;
            }
        }

        /**/
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        print_result(m, cluster_count, eps, node_count, links.size(), 0.0, duration.count());

        if (clusters.empty()) {
            continue;
        }

        std::future<void> future = std::async(std::launch::async, request_graph,
                                              std::ref(httpclient), clusters, m, eps);
        futures.push_back(std::move(future));
    }

    for (auto &future : futures) {
        future.wait();
    }
}