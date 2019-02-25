#ifndef MANETSIMS_LINK_H
#define MANETSIMS_LINK_H

#include <utility>
#include <ostream>
#include <functional>

#include "node.h"

namespace reachi {

    class Link {
    public:
        Link(uint64_t id, reachi::Node node1, reachi::Node node2);

        const ::std::pair<reachi::Node, reachi::Node> &get_nodes() const;

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
        ::std::pair<reachi::Node, reachi::Node> nodes;

    };

    void to_json(json &j, const Link &p);

    void from_json(const json &j, Link &p);
}

#endif /* MANETSIMS_LINK_H */
