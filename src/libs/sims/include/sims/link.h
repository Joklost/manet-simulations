#ifndef MANETSIMS_LINK_H
#define MANETSIMS_LINK_H

#include <utility>
#include <ostream>
#include <functional>

#include "node.h"

namespace sims {

    class Link {
    public:
        Link() = default;
        Link(uint64_t id, sims::Node &node1, sims::Node &node2);

        const ::std::pair<sims::Node, sims::Node> &get_nodes() const;

        double get_distance() const;

        uint64_t get_id() const;

        bool operator==(const Link &rhs) const;

        bool operator!=(const Link &rhs) const;

        bool operator<(const Link &rhs) const;

        bool operator>(const Link &rhs) const;

        bool operator<=(const Link &rhs) const;

        bool operator>=(const Link &rhs) const;

        double distance{};
    private:
        uint64_t id{};
        ::std::pair<sims::Node, sims::Node> nodes;

    };
}

#endif /* MANETSIMS_LINK_H */
