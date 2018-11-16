
#include <utility>
#include <mpilib/node.h>

Node::Node(uint32_t id, Location location) : current_location(location) {
    this->id = id;
}

uint32_t Node::get_id() const {
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
    location.set_time(time);
    this->location_history.emplace_back(this->current_location);
    this->current_location = location;
}

nlohmann::json Node::serialize() {
    return {{"id",  this->id},
            {"lat", std::to_string(this->current_location.get_latitude())},
            {"lon", std::to_string(this->current_location.get_longitude())}};
}

void Node::move(int time, double distance, double bearing) {
    this->current_location.move(time, distance, bearing);
}

