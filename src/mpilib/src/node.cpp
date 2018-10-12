
#include <utility>
#include "mpilib/node.h"

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
