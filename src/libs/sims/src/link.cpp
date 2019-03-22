#include <sims/ostr.h>
#include <sims/link.h>

#include <geo/geo.h>

sims::Link::Link(uint64_t id, Node &node1, Node &node2) : nodes(std::make_pair(node1, node2)) {
    this->id = id;
    this->distance = geo::distance_between(node1.get_location(), node2.get_location());
}


const std::pair<sims::Node, sims::Node> &sims::Link::get_nodes() const {
    return this->nodes;
}

double sims::Link::get_distance() const {
    return distance;
}

bool sims::Link::operator==(const Link &rhs) const {
    return this->id == rhs.id;
}

bool sims::Link::operator!=(const Link &rhs) const {
    return !(rhs == *this);
}

uint64_t sims::Link::get_id() const {
    return id;
}

bool sims::Link::operator<(const Link &rhs) const {
    return id < rhs.id;
}

bool sims::Link::operator>(const Link &rhs) const {
    return rhs < *this;
}

bool sims::Link::operator<=(const Link &rhs) const {
    return !(rhs < *this);
}

bool sims::Link::operator>=(const Link &rhs) const {
    return !(*this < rhs);
}