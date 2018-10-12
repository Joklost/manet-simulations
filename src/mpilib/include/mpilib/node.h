#ifndef MANETSIMS_MPILIB_NODE_H
#define MANETSIMS_MPILIB_NODE_H

#include <mpilib/geomath.h>

class Node {
public:
    Node(int id, Location location);

    int get_id() const;

    const Location &get_location() const;

private:
    int id;
    Location location;
};

#endif /* MANETSIMS_MPILIB_NODE_H */
