
#include <utility>
#include <mpilib/node.h>

Node::Node(int id, Location location) {
    this->id = id;
    this->current_location = location;
}

int Node::get_id() const {
    return this->id;
}

const Location &Node::get_location() const {
    return this->current_location;
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
    os << "id: " << node.id << " current_location: " << node.current_location;
    return os;
}

void Node::update_location(Location &location, const int time) {
    location.time = time;
    this->location_history.emplace_back(this->current_location);
    this->current_location = location;
}

