#ifndef COORDINATOR_NODE_H
#define COORDINATOR_NODE_H

#include <vector>

#include <geo/geo.h>

struct Node {
    bool operator==(const Node &rhs) const;

    bool operator!=(const Node &rhs) const;

    unsigned long rank{};
    unsigned long id{};
    std::string name{};
    geo::Location loc{};
    std::vector<geo::Location> location_history{};

    bool dead{};
    unsigned long action_count{};
};

#endif //COORDINATOR_NODE_H
