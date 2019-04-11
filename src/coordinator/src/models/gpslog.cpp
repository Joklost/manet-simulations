#include <fstream>
#include <algorithm>

#include <common/strings.h>

#include "gpslog.h"

std::map<unsigned long, Node> load_log(const char *logpath) {
    std::map<unsigned long, Node> nodes{};
    std::ifstream logfile{logpath};

    if (!logfile.is_open()) {
        throw std::runtime_error("failed to open gpslog file");
    }

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

        auto &node = nodes[id]; /* operator[] implicitly constructs new object */
        node.id = id;
        node.location_history.emplace_back(time, lat, lon);
        auto &location = node.location_history.back();

        while (!tokens.empty()) {
            auto neighbour_id = std::stoul(tokens.fpop());
            auto rssi = std::stod(tokens.fpop());
            location.connections[neighbour_id] = rssi;
        }

    }

    logfile.close();

    for (auto &item : nodes) {
        auto &node = item.second;
        std::sort(node.location_history.begin(), node.location_history.end());
    }

    return nodes;
}

