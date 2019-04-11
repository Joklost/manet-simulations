#include "link.h"

Link::Link(unsigned long long id, Node &n1, Node &n2) {
    this->id = id;
    this->nodes = std::make_pair(n1, n2);
}

bool Link::operator==(const Link &rhs) const {
    return id == rhs.id;
}

bool Link::operator!=(const Link &rhs) const {
    return !(rhs == *this);
}
