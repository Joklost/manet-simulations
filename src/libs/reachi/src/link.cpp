#include <reachi/ostr.h>
#include <reachi/link.h>

#include <mpilib/helpers.h>
#include <geo/geo.h>
#include <mpilib/ostr.h>
#include <mpilib/link.h>

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

void reachi::to_json(json &j, const reachi::Link &p) {
    j = json{{"id", p.get_id()},
             {"first", p.get_nodes().first.get_id()},
             {"second", p.get_nodes().second.get_id()}};
}

void reachi::from_json(const json &j, reachi::Link &p) {

}

/*
void reachi::to_json(json &j, const reachi::Node &p) {
    j = json{{"id",  p.get_id()},
             {"lat", p.get_location().get_latitude()},
             {"lon", p.get_location().get_longitude()}};
}

void reachi::from_json(const json &j, reachi::Node &p) {
    auto id = j.at("id").get<uint32_t>();
    auto lat = j.at("lat").get<double>();
    auto lon = j.at("lon").get<double>();

    p = {id, {lat, lon}};
}
 * */