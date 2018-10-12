
#include "mpilib/link.h"

Link::Link(Node node1, Node node2) : nodes(std::make_pair(node1, node2)) {
    this->distance = distance_between(node1.get_location(), node2.get_location());
}


const std::pair<Node, Node> &Link::get_nodes() const {
    return this->nodes;
}

double Link::get_distance() const {
    return distance;
}

