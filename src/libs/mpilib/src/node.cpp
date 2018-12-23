#include <mpilib/node.h>


bool mpilib::Node::operator==(const mpilib::Node &rhs) const {
    return rank == rhs.rank;
}

bool mpilib::Node::operator!=(const mpilib::Node &rhs) const {
    return !(rhs == *this);
}
