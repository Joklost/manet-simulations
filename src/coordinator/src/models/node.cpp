#include "node.h"

bool Node::operator==(const Node &rhs) const {
    return rank == rhs.rank &&
           id == rhs.id;
}

bool Node::operator!=(const Node &rhs) const {
    return !(rhs == *this);
}
