#ifndef MANETSIMS_MPILIB_NODE_H
#define MANETSIMS_MPILIB_NODE_H

#include <mpilib/location.h>
#include <ostream>

class Node {
public:
    Node(int id, Location location);

    int get_id() const;

    const Location &get_location() const;

    bool operator==(const Node &rhs) const;

    bool operator!=(const Node &rhs) const;

    bool operator<(const Node &rhs) const;

    bool operator>(const Node &rhs) const;

    bool operator<=(const Node &rhs) const;

    bool operator>=(const Node &rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const Node &node);

private:
    int id;
    Location location;
};

#endif /* MANETSIMS_MPILIB_NODE_H */
