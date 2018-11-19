#ifndef MANETSIMS_MPILIB_NODE_H
#define MANETSIMS_MPILIB_NODE_H

#include <mpilib/location.h>
#include <vector>
#include <json.hpp>
#include <reachi/constants.h>


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

    /* OPTICS */

    unsigned int get_index() const;

    void set_index(unsigned int index);

    double get_reachability_distance() const;

    void set_reachability_distance(double reachability_distance);

    double get_core_distance() const;

    void set_core_distance(double core_distance);

    bool is_processed() const;

    void set_processed(bool processed);

private:
    uint32_t id;
    Location current_location;
    std::vector<Location> location_history;

    /* OPTICS */
    unsigned int index{};
    double reachability_distance{UNDEFINED};
    double core_distance{UNDEFINED};
    bool processed{};
};


namespace std {
    template<>
    struct hash<Node> {
        std::size_t operator()(const Node &k) const {
            return std::hash<uint32_t>{}(k.get_id());
        }
    };
}

#endif /* MANETSIMS_MPILIB_NODE_H */
