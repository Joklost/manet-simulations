#ifndef MANETSIMS_LINK_H
#define MANETSIMS_LINK_H

#include <utility>
#include <ostream>
#include <functional>

#include "node.h"

class Link {
public:
    Link(uint32_t id, Node node1, Node node2);

    const std::pair<Node, Node> &get_nodes() const;

    double get_distance() const;

    uint32_t get_id() const;

    bool operator==(const Link &rhs) const;

    bool operator!=(const Link &rhs) const;

    bool operator<(const Link &rhs) const;

    bool operator>(const Link &rhs) const;

    bool operator<=(const Link &rhs) const;

    bool operator>=(const Link &rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const Link &link);

private:
    double distance;
    uint32_t id;
    std::pair<Node, Node> nodes;

};

using linkpair = std::pair<Link, Link>;

namespace std {
    template<>
    struct hash<Link> {
        std::size_t operator()(const Link &k) const {
            return std::hash<uint32_t>{}(k.get_id());
        }
    };

    template<>
    struct hash<linkpair> {
        std::size_t operator()(const linkpair &lp) const {
            auto h1 = std::hash<Link>{}(lp.first);
            auto h2 = std::hash<Link>{}(lp.second);

            return lp.first < lp.second ? h1 ^ (h2 << 1) : h2 ^ (h1 << 1);
        }
    };
}

#endif /* MANETSIMS_LINK_H */
