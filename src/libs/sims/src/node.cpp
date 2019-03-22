
#include <utility>
#include <sims/node.h>

sims::Node::Node(uint32_t id, geo::Location location) : current_location(location) {
    this->id = id;
}

uint32_t sims::Node::get_id() const {
    return this->id;
}

const geo::Location &sims::Node::get_location() const {
    return this->current_location;
}

bool sims::Node::operator==(const Node &rhs) const {
    return id == rhs.id;
}

bool sims::Node::operator!=(const Node &rhs) const {
    return !(rhs == *this);
}

bool sims::Node::operator<(const Node &rhs) const {
    return id < rhs.id;
}

bool sims::Node::operator>(const Node &rhs) const {
    return rhs < *this;
}

bool sims::Node::operator<=(const Node &rhs) const {
    return !(rhs < *this);
}

bool sims::Node::operator>=(const Node &rhs) const {
    return !(*this < rhs);
}

void sims::Node::update_location(geo::Location &location, const int time) {
    location.set_time(time);
    this->location_history.emplace_back(this->current_location);
    this->current_location = location;
}

void sims::Node::move(int time, double distance, double bearing) {
    this->current_location.move(time, distance, bearing);
}

double sims::Node::get_reachability_distance() const {
    return reachability_distance;
}

void sims::Node::set_reachability_distance(double reachability_distance) {
    sims::Node::reachability_distance = reachability_distance;
}

double sims::Node::get_core_distance() const {
    return core_distance;
}

void sims::Node::set_core_distance(double core_distance) {
    sims::Node::core_distance = core_distance;
}

bool sims::Node::is_processed() const {
    return processed;
}

void sims::Node::set_processed(bool processed) {
    sims::Node::processed = processed;
}
