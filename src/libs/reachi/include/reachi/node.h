#ifndef MANETSIMS_REACHI_NODE_H
#define MANETSIMS_REACHI_NODE_H

#include <vector>
#if 0
#include <nlohmann/json.hpp>
#endif
#include <mpilib/location.h>
#include <ostream>

#include "constants.h"
#if 0
using json = nlohmann::json;
#endif
namespace reachi {

    class Node {
    public:
        Node();

        Node(uint32_t id, mpilib::geo::Location location);

        uint32_t get_id() const;

        const mpilib::geo::Location &get_location() const;

        void update_location(mpilib::geo::Location &location, int time);

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

    private:
        uint32_t id;
        mpilib::geo::Location current_location;
        std::vector<mpilib::geo::Location> location_history;

        /* OPTICS */
        double reachability_distance{};
        double core_distance{};
        bool processed{};
    };
#if 0
    void to_json(json &j, const Node &p);

    void from_json(const json &j, Node &p);
#endif
}

namespace std {
    template<>
    struct hash<reachi::Node> {
        std::size_t operator()(const reachi::Node &k) const {
            return std::hash<uint32_t>{}(k.get_id());
        }
    };
}
#endif /* MANETSIMS_REACHI_NODE_H */
