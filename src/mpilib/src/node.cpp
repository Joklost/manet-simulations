
#include <utility>
#include <mpilib/node.h>

Node::Node(int id, Location location) {
    this->id = id;
    this->location = location;
}

int Node::get_id() const {
    return this->id;
}

const Location &Node::get_location() const {
    return this->location;
}

bool Node::operator==(const Node &rhs) const {
    return id == rhs.id;
}

bool Node::operator!=(const Node &rhs) const {
    return !(rhs == *this);
}

bool Node::operator<(const Node &rhs) const {
    return id < rhs.id;
}

bool Node::operator>(const Node &rhs) const {
    return rhs < *this;
}

bool Node::operator<=(const Node &rhs) const {
    return !(rhs < *this);
}

bool Node::operator>=(const Node &rhs) const {
    return !(*this < rhs);
}

std::ostream &operator<<(std::ostream &os, const Node &node) {
    os << "id: " << node.id << " location: " << node.location;
    return os;
}

