
#include <utility>
#include <reachi/node.h>

reachi::Node::Node(uint32_t id, geo::Location location) : current_location(location) {
    this->id = id;
}

uint32_t reachi::Node::get_id() const {
    return this->id;
}

const geo::Location &reachi::Node::get_location() const {
    return this->current_location;
}

bool reachi::Node::operator==(const Node &rhs) const {
    return id == rhs.id;
}

bool reachi::Node::operator!=(const Node &rhs) const {
    return !(rhs == *this);
}

bool reachi::Node::operator<(const Node &rhs) const {
    return id < rhs.id;
}

bool reachi::Node::operator>(const Node &rhs) const {
    return rhs < *this;
}

bool reachi::Node::operator<=(const Node &rhs) const {
    return !(rhs < *this);
}

bool reachi::Node::operator>=(const Node &rhs) const {
    return !(*this < rhs);
}

void reachi::Node::update_location(geo::Location &location, const int time) {
    location.set_time(time);
    this->location_history.emplace_back(this->current_location);
    this->current_location = location;
}

void reachi::Node::move(int time, double distance, double bearing) {
    this->current_location.move(time, distance, bearing);
}

double reachi::Node::get_reachability_distance() const {
    return reachability_distance;
}

void reachi::Node::set_reachability_distance(double reachability_distance) {
    reachi::Node::reachability_distance = reachability_distance;
}

double reachi::Node::get_core_distance() const {
    return core_distance;
}

void reachi::Node::set_core_distance(double core_distance) {
    reachi::Node::core_distance = core_distance;
}

bool reachi::Node::is_processed() const {
    return processed;
}

void reachi::Node::set_processed(bool processed) {
    reachi::Node::processed = processed;
}
