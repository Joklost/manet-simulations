#ifndef MANETSIMS_REACHI_NODE_H
#define MANETSIMS_REACHI_NODE_H

#include <vector>
#include <geo/geo.h>
#include <ostream>

#include "constants.h"

namespace sims {

    class Node {
    public:
        Node() = default;

        Node(uint32_t id, geo::Location location);

        uint32_t get_id() const;

        const geo::Location &get_location() const;

        void update_location(geo::Location &location, int time);

        bool operator==(const Node &rhs) const;

        bool operator!=(const Node &rhs) const;

        bool operator<(const Node &rhs) const;

        bool operator>(const Node &rhs) const;

        bool operator<=(const Node &rhs) const;

        bool operator>=(const Node &rhs) const;

        void move(int time, double distance /*kilometers */, double bearing /* degrees */);

        /* OPTICS */
        double get_reachability_distance() const;

        void set_reachability_distance(double reachability_distance);

        double get_core_distance() const;

        void set_core_distance(double core_distance);

        bool is_processed() const;

        void set_processed(bool processed);

        uint32_t id{};
        std::vector<geo::Location> location_history;
        geo::Location current_location;

    private:
        /* OPTICS */
        double reachability_distance{};
        double core_distance{};
        bool processed{};
    };
}

namespace std {
    template<>
    struct hash<sims::Node> {
        std::size_t operator()(const sims::Node &k) const {
            return std::hash<uint32_t>{}(k.get_id());
        }
    };
}
#endif /* MANETSIMS_REACHI_NODE_H */
