#include <iostream>
#include <reachi/datagen.h>
#include <reachi/clustering.h>
#include <mpilib/node.h>
#include <mpilib/httpclient.h>
#include <mpilib/helpers.h>
#include <fstream>
#include <iomanip>


int main(int argc, char *argv[]) {

    std::vector<Node> nodes{};
    std::ifstream f("nodes.json");

    if (f.good()) {
        nodes = json::parse(f).get<std::vector<Node>>();
    } else {
        Location upper{56.8804073, 8.675316};
        Location lower{56.7391447, 8.8079536};
        nodes = generate_nodes(1000, upper, lower);

        Location cl1{56.84521, 8.727969};
        auto c1 = generate_cluster(cl1, static_cast<uint32_t>(nodes.size()), 300, 1.5);
        nodes.insert(nodes.end(), c1.begin(), c1.end());

        Location cl2{56.825899, 8.768404};
        auto c2 = generate_cluster(cl2, static_cast<uint32_t>(nodes.size()), 200, 1.0);
        nodes.insert(nodes.end(), c2.begin(), c2.end());

        Location cl3{56.776121, 8.726132};
        auto c3 = generate_cluster(cl3, static_cast<uint32_t>(nodes.size()), 200, 1.0);
        nodes.insert(nodes.end(), c3.begin(), c3.end());


        json jsonfile = nodes;
        std::ofstream file("nodes.json");
        file << jsonfile;
    }
    std::string url = "http://127.0.0.1:5000";
    auto url_length = static_cast<int>(url.length() + 12);

    HttpClient httpclient{url};
    json json_nodes = nodes;
    httpclient.post("/add-nodes", json_nodes);

    Optics optics{};

    auto minpts_start = 2;
    auto minpts_end = 20;

    auto eps_start = 0.01;
    auto eps_end = 0.2;
    auto eps_step = 0.01;
    auto eps_steps = static_cast<unsigned long>(std::round(eps_end / eps_step));

    auto m = 1;

    std::cout
            << std::setw(url_length) << "Run"
            << std::setw(12) << "Clusters"
            << std::setw(12) << "Epsilon"
            << std::setw(12) << "MinPts"
            << std::setw(12) << "Time"
            << std::endl;

    std::cout << std::string(url_length + 4 * 12, '-') << std::endl;

    std::vector<std::thread> threads{};

    for (auto j = minpts_start; j < minpts_end; j++) {
        auto minpts = j;

        for (auto k = 0; k < eps_steps; k++) {
            auto eps = eps_start + k * eps_step;


            auto start = std::chrono::steady_clock::now();
            auto ordering = optics.compute_ordering(nodes, eps, minpts);
            auto clusters = optics.cluster(ordering);
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            std::cout
                    << url << "/graphs/" << std::setw(4) << std::setfill('0') << m << std::setfill(' ')
                    << std::setw(12) << clusters.size()
                    << std::setw(9) << eps << " km"
                    << std::setw(12) << minpts
                    << std::setw(9) << duration.count() << " ms"
                    << std::endl;

            m++;

            if (clusters.empty()) {
                continue;
            }

            std::thread t{[&httpclient, clusters, m, minpts, eps]() {
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
                                             {"id", m},
                                             {"minpts", minpts},
                                             {"eps", eps},
                                             {"count", clusters.size()}
                                     }}
                };

                httpclient.post("/request-graph", json_clusters);
            }};

            t.detach();
            threads.push_back(std::move(t));

        }
    }

    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}