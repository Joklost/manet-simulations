#ifndef MANETSIMS_MPILIB_NODE_H
#define MANETSIMS_MPILIB_NODE_H

#include <mpilib/location.h>
#include <vector>

class Node {
public:
    Node(int id, Location location);

    int get_id() const;

    const Location &get_location() const;

    void update_location(Location &location, int time);

    bool operator==(const Node &rhs) const;

    bool operator!=(const Node &rhs) const;

    bool operator<(const Node &rhs) const;

    bool operator>(const Node &rhs) const;

    bool operator<=(const Node &rhs) const;

    bool operator>=(const Node &rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const Node &node);

private:
    int id;
    Location current_location;
    std::vector<Location> location_history;

};

#endif /* MANETSIMS_MPILIB_NODE_H */
