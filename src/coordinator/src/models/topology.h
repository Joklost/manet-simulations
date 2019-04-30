#ifndef COORDINATOR_TOPOLOGY_H
#define COORDINATOR_TOPOLOGY_H

#include <unordered_map>

#include "location.h"

struct Topology {
    double timestamp{};
    std::unordered_map<unsigned long, Location> locations{};
    std::unordered_map<unsigned long, std::unordered_map<unsigned long, double>> links{};

    double get_link(unsigned long rank1, unsigned long rank2);
};

#endif //COORDINATOR_TOPOLOGY_H
