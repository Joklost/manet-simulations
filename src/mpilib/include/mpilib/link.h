#ifndef MANETSIMS_LINK_H
#define MANETSIMS_LINK_H

#include <utility>
#include <ostream>

#include "node.h"

class Link {
public:
    Link(int id, Node node1, Node node2);

    const std::pair<Node, Node> &get_nodes() const;

    double get_distance() const;

    int get_id() const;

    bool operator==(const Link &rhs) const;

    bool operator!=(const Link &rhs) const;

    bool operator<(const Link &rhs) const;

    bool operator>(const Link &rhs) const;

    bool operator<=(const Link &rhs) const;

    bool operator>=(const Link &rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const Link &link);

private:
    double distance;
    int id;
    std::pair<Node, Node> nodes;

};

#endif /* MANETSIMS_LINK_H */
