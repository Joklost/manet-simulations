#ifndef MANETSIMS_MPILIB_NODE_H
#define MANETSIMS_MPILIB_NODE_H

#include <mpilib/location.h>
#include <vector>
#include <nlohmann/json.hpp>


class Node {
public:
    Node(uint32_t id, Location location);

    uint32_t get_id() const;

    const Location &get_location() const;

    void update_location(Location &location, int time);

    bool operator==(const Node &rhs) const;

    bool operator!=(const Node &rhs) const;

    bool operator<(const Node &rhs) const;

    bool operator>(const Node &rhs) const;

    bool operator<=(const Node &rhs) const;

    bool operator>=(const Node &rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const Node &node);

    nlohmann::json serialize();

    void move(int time, double distance /*kilometers */, double bearing /* degrees */);

private:
    uint32_t id;
    Location current_location;
    std::vector<Location> location_history;

};

#endif /* MANETSIMS_MPILIB_NODE_H */
