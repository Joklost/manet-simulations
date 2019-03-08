#include <reachi/ostr.h>
#include <reachi/link.h>

#include <geo/geo.h>

reachi::Link::Link(uint64_t id, Node &node1, Node &node2) : nodes(std::make_pair(node1, node2)) {
    this->id = id;
    this->distance = geo::distance_between(node1.get_location(), node2.get_location());
}


const std::pair<reachi::Node, reachi::Node> &reachi::Link::get_nodes() const {
    return this->nodes;
}

double reachi::Link::get_distance() const {
    return distance;
}

bool reachi::Link::operator==(const Link &rhs) const {
    return this->id == rhs.id;
}

bool reachi::Link::operator!=(const Link &rhs) const {
    return !(rhs == *this);
}

uint64_t reachi::Link::get_id() const {
    return id;
}

bool reachi::Link::operator<(const Link &rhs) const {
    return id < rhs.id;
}

bool reachi::Link::operator>(const Link &rhs) const {
    return rhs < *this;
}

bool reachi::Link::operator<=(const Link &rhs) const {
    return !(rhs < *this);
}

bool reachi::Link::operator>=(const Link &rhs) const {
    return !(*this < rhs);
}