#include <fstream>
#include <common/strings.h>
#include <algorithm>

#include "gpslog.h"

std::unordered_map<unsigned long, reachi::Node> parse_gpsfile(const char *gpslog) {
    std::unordered_map<unsigned long, reachi::Node> nodes{};
    std::ifstream logfile{gpslog};

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
        auto id = std::stoul(tokens[0]);
        auto latitude = std::stod(tokens[1]);
        auto longitude = std::stod(tokens[2]);
        auto timestamp = std::stoul(tokens[3]);

        auto &node = nodes[id]; /* operator[] implicitly constructs new object */
        node.id = id;
        node.location_history.emplace_back(timestamp, latitude, longitude);
    }

    logfile.close();

    for (auto &item : nodes) {
        std::sort(item.second.location_history.begin(), item.second.location_history.end());
    }

    return nodes;
}
