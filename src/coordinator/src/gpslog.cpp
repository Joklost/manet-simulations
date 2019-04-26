#include <fstream>
#include <algorithm>

#include <common/strings.h>

#include "gpslog.h"

std::pair<double, std::vector<Node>> load_log(const char *logpath) {
    std::map<unsigned long, Node> nodes{};
    std::ifstream logfile{logpath};

    if (!logfile.is_open()) {
        throw std::runtime_error("failed to open gpslog file");
    }

    double greatest_time{};

    while (logfile.good()) {
        std::string line;
        std::getline(logfile, line);

        if (line.empty()) {
            continue;
        }

        auto tokens = common::split(line, ",");
        auto id = std::stoul(tokens.fpop());
        auto lat = std::stod(tokens.fpop());
        auto lon = std::stod(tokens.fpop());
        auto time = std::stod(tokens.fpop());

        if (time > greatest_time) {
            greatest_time = time;
        }

        auto &node = nodes[id]; /* operator[] implicitly constructs new object */
        node.id = id;
        auto &location = node.locations[time];
        location.time = time;
        location.latitude = lat;
        location.longitude = lon;

        while (!tokens.empty()) {
            auto neighbour_id = std::stoul(tokens.fpop());
            auto rssi = std::stod(tokens.fpop());
            location.links[neighbour_id] = rssi;
        }
    }

    logfile.close();
    std::vector<Node> nodelist{};
    nodelist.reserve(nodes.size());


    auto i = 1ul;
    for (auto &id_node_pair : nodes) {
        auto &node = id_node_pair.second;
        node.rank = i;
        nodelist.push_back(id_node_pair.second);
        i += 1ul;
    }

    return {greatest_time, nodelist};
}

