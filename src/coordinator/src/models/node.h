#ifndef COORDINATOR_NODE_H
#define COORDINATOR_NODE_H

#include <map>
#include <ostream>

#include <common/equality.h>

#include "location.h"

struct Node {
    unsigned long rank{};
    unsigned long id{};
    std::string name{};
//    Location loc{};
    std::map<double, Location, common::is_less<double>> locations{};
//    std::vector<Location> location_history{};

    bool dead{};
    unsigned long action_count{};

    bool operator==(const Node &rhs) const;

    bool operator!=(const Node &rhs) const;
};

#endif //COORDINATOR_NODE_H
