
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

unsigned int Node::get_index() const {
    return index;
}

void Node::set_index(unsigned int index) {
    Node::index = index;
}

double Node::get_reachability_distance() const {
    return reachability_distance;
}

void Node::set_reachability_distance(double reachability_distance) {
    Node::reachability_distance = reachability_distance;
}

double Node::get_core_distance() const {
    return core_distance;
}

void Node::set_core_distance(double core_distance) {
    Node::core_distance = core_distance;
}

bool Node::is_processed() const {
    return processed;
}

void Node::set_processed(bool processed) {
    Node::processed = processed;
}

