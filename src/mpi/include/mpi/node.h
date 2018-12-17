#ifndef MANETSIMS_MPI_NODE_H
#define MANETSIMS_MPI_NODE_H

#include <mpi/location.h>
#include <vector>
#include <json.hpp>
#include <reachi/constants.h>

using json = nlohmann::json;

class Node {
public:
    Node();

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

    void move(int time, double distance /*kilometers */, double bearing /* degrees */);

    /* OPTICS */

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

void to_json(json &j, const Node &p);

void from_json(const json &j, Node &p);

#endif /* MANETSIMS_MPI_NODE_H */
