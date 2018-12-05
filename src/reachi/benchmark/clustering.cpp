#include <iostream>
#include <iomanip>
#include <future>

#include <mpilib/node.h>
#include <reachi/datagen.h>
#include <mpilib/httpclient.h>

#define MIN_PTS 2
#define MIN_EPS 0.01
#define MAX_EPS 0.1
#define EPS_STEP 0.01

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
            << std::setw(12) << "MinPts"
            << std::setw(12) << "Nodes"
            //            << std::setw(12) << "Radius"
            //            << std::setw(12) << "Cost"
            << std::setw(12) << "Time"
            << std::endl;
}

void print_result(std::vector<Optics::Cluster> &clusters, std::vector<Node> &nodes,
                  int graph_id, double eps, int minpts, long duration) {

    double us = duration / 1000.0;
    long ms = static_cast<long>(std::round(us));

    unsigned long node_count = 0;
    for (auto &cluster : clusters) {
        node_count += cluster.size();
    }

    node_count = nodes.size() - node_count + clusters.size();

    std::cout
            << std::setprecision(3)
            << std::fixed
            << std::setw(8) << "" << std::setw(4) << std::setfill('0') << graph_id << std::setfill(' ')
            << std::setw(12) << clusters.size()
            << std::setw(9) << eps << " km"
            << std::setw(12) << minpts
            << std::setw(12) << node_count;
    /*<< std::setw(12) << avg_radius
    << std::setw(12) << total_cost*/

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


int main(int argc, char *argv[]) {
    std::vector<std::future<void>> futures{};
    std::vector<std::thread> threads{};

    Location upper{57.0134, 9.99008};
    Location lower{57.0044, 10.0066};

    auto nodes = generate_nodes(100, upper, lower);
    HttpClient httpclient{"http://0.0.0.0:5000/vis"};
    json j_nodes = nodes;
    httpclient.post("/add-nodes", j_nodes);

    auto eps_steps = static_cast<unsigned long>(std::round(MAX_EPS / EPS_STEP));

    print_header();

    auto m = 0;
    for (int i = 0; i < eps_steps; ++i) {
        m++;
        auto eps = MIN_EPS + i * EPS_STEP;

        auto start = std::chrono::steady_clock::now();

        /**/
        auto clusters = cluster(nodes, eps, MIN_PTS);
        /**/

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        print_result(clusters, nodes, m, eps, MIN_PTS, duration.count());

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