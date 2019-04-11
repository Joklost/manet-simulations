#ifndef COORDINATOR_LINK_H
#define COORDINATOR_LINK_H

#include <utility>

#include "node.h"

struct Link {

    Link() = default;
    Link(unsigned long long id, Node &n1, Node &n2);

    bool operator==(const Link &rhs) const;

    bool operator!=(const Link &rhs) const;

    unsigned long long id{};
    std::pair<Node, Node> nodes{};

    double rssi{};
};

#endif //COORDINATOR_LINK_H
