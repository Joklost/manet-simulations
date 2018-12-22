#ifndef MANETSIMS_LINK_H
#define MANETSIMS_LINK_H

#include <utility>
#include <ostream>
#include <functional>

#include "node.h"

namespace reachi {

    class Link {
    public:
        Link(uint64_t id, Node node1, Node node2);

        const ::std::pair<Node, Node> &get_nodes() const;

        double get_distance() const;

        uint64_t get_id() const;

        bool operator==(const Link &rhs) const;

        bool operator!=(const Link &rhs) const;

        bool operator<(const Link &rhs) const;

        bool operator>(const Link &rhs) const;

        bool operator<=(const Link &rhs) const;

        bool operator>=(const Link &rhs) const;

    private:
        double distance;
        uint64_t id;
        ::std::pair<Node, Node> nodes;

    };

}

#endif /* MANETSIMS_LINK_H */
