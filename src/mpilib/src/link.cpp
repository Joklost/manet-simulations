
#include <mpilib/link.h>
#include <mpilib/helpers.h>
#include <mpilib/geomath.h>

Link::Link(uint32_t id, Node node1, Node node2) : nodes(std::make_pair(node1, node2)) {
    this->id = id;
    this->distance = distance_between(node1.get_location(), node2.get_location());
}


const std::pair<Node, Node> &Link::get_nodes() const {
    return this->nodes;
}

double Link::get_distance() const {
    return distance;
}

bool Link::operator==(const Link &rhs) const {
    return this->id == rhs.id;
}

bool Link::operator!=(const Link &rhs) const {
    return !(rhs == *this);
}

uint32_t Link::get_id() const {
    return id;
}

bool Link::operator<(const Link &rhs) const {
    return id < rhs.id;
}

bool Link::operator>(const Link &rhs) const {
    return rhs < *this;
}

bool Link::operator<=(const Link &rhs) const {
    return !(rhs < *this);
}

bool Link::operator>=(const Link &rhs) const {
    return !(*this < rhs);
}

std::ostream &operator<<(std::ostream &os, const Link &link) {
    os << "distance: " << link.distance << " id: " << link.id << " nodes: " << link.nodes;
    return os;
}
