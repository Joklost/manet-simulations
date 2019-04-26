#include <map>
#include <iostream>
#include <deque>

#include <common/all.h>
#include <geo/geo.h>
#include <geo/generator.h>
#include <sims/node.h>
#include <sims/link.h>
#include <sims/math.h>

std::map<unsigned long, sims::Node> parse_log(const std::vector<std::string> &lines) {
    std::map<unsigned long, sims::Node> nodes{};

    for (auto &line : lines) {
        auto tokens = common::split(line, ",");
        auto id = std::stoul(tokens.fpop());
        auto latitude = std::stod(tokens.fpop());
        auto longitude = std::stod(tokens.fpop());
        auto timestamp = std::stod(tokens.fpop());

        auto &node = nodes[id];
        node.id = id;
        node.location_history.emplace_back(timestamp, latitude, longitude);
    }

    for (auto &item : nodes) {
        auto &node = item.second;
        std::sort(node.location_history.begin(), node.location_history.end());
    }

    return nodes;
};

struct Topology {
    double timestamp{};
    std::vector<sims::Link> links{};
    std::map<unsigned long, geo::Location> locations{};
    std::map<unsigned long, std::map<unsigned long, double>> connections{};
};

int main(int argc, char *argv[]) {
    geo::Location l{14.629740, 121.103266};
//    std::string gridlog{"gridlog_8x8_rssi.txt"};
//    std::cout << "Generating " << gridlog << std::endl;
    const auto time_gap = 20000.0;
    auto nodes = parse_log(grid(l, 4, 475_m, 100000.0, time_gap));
    std::vector<sims::Node> node_list(nodes.size());
    std::map<double, Topology, common::is_less<double>> topologies{};

    for (auto &item : nodes) {
        auto &node = item.second;
        node_list.push_back(node);

        for (auto &location : node.location_history) {
            if (topologies.find(location.time) == topologies.end()) {
                topologies[location.time] = {location.time};
            }
        }
    }

    for (auto &item : topologies) {
        const auto time = item.first;
        auto &topology = item.second;
        auto &links = topology.links;
        for (unsigned long i = 0; i < node_list.size(); ++i) {
            auto node1 = node_list[i]; /* Copy */

            for (auto &location : node1.location_history) {
                if (location.time <= time && location.time > (time - time_gap)) {
                    node1.current_location = location;
                    topology.locations[node1.id] = location;
                }
            }

            for (unsigned long j = i + 1; j < node_list.size(); ++j) {
                auto node2 = node_list[j]; /* Copy */

                for (auto &location : node2.location_history) {
                    if (location.time <= time && location.time > (time - time_gap)) {
                        node2.current_location = location;
                    }
                }

                if (node1.current_location.latitude > 0 && node2.current_location.latitude > 0 &&
                    node1.current_location.longitude > 0 && node2.current_location.longitude > 0) {
                    /* Compute distance based path loss on the links as we create them. */
                    auto distance = geo::distance_between(node1.current_location, node2.current_location);
                    auto pathloss = sims::math::distance_pathloss(distance * KM);
                    auto rssi = 26.0 - pathloss;
                    if (rssi > -112) {
                        auto id = common::combine_ids(node1.id, node2.id);
                        links.emplace_back(id, node1, node2);
                        auto &link = links.back();
                        link.distance = rssi;
                        topology.connections[node1.id][node2.id] = rssi;
                        topology.connections[node2.id][node1.id] = rssi;
                    }
                }
            }
        }

    }

    for (auto &item : topologies) {
        auto time = item.first;
        auto &topology = item.second;

        for (auto &node_item : nodes) {
            auto &node = node_item.second;
            std::cout << node.id << ","
                      << std::fixed << topology.locations[node.id].latitude << ","
                      << std::fixed << topology.locations[node.id].longitude << ","
                      << std::fixed << time;

            for (auto &connection : topology.connections[node.id]) {
                std::cout << "," << connection.first << "," << std::fixed << connection.second;
            }
            std::cout << std::endl;
        }


    }
}