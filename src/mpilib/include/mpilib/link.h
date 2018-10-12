#ifndef MANETSIMS_LINK_H
#define MANETSIMS_LINK_H

#include <utility>

#include "node.h"

class Link {
public:
    Link(Node node1, Node node2);

    const std::pair<Node, Node> &get_nodes() const;

    double get_distance() const;

private:
    double distance;
    std::pair<Node, Node> nodes;

};

#endif /* MANETSIMS_LINK_H */
