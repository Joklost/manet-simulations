#ifndef MANETSIMS_LINK_H
#define MANETSIMS_LINK_H

#include <mpilib/geomath.h>

namespace reachi2 {
    struct Node {
        unsigned int id;
        mpilib::geo::Location location;

        bool operator==(const Node &rhs) const {
            return id == rhs.id;
        }

        bool operator!=(const Node &rhs) const {
            return !(rhs == *this);
        }

        bool operator==(const int &rhs) const {
            return this->id == rhs;
        }

        bool operator!=(const int &rhs) const {
            return this->id != rhs;
        }
    };

    struct Link {
        unsigned int id;
        Node node1, node2;
        double distance;

        Link(unsigned int id, Node node1, Node node2) {
            this->id = id;
            this->node1 = node1;
            this->node2 = node2;
            this->distance = mpilib::geo::distance_between(node1.location, node2.location);
        }

        bool operator==(const Link &rhs) const {
            return id == rhs.id;
        }

        bool operator!=(const Link &rhs) const {
            return !(rhs == *this);
        }

        double angle_between(const Link &link) const {
            if (this->node1 == link.node1) {
                return mpilib::geo::angle_between(this->node1.location, this->node2.location, link.node2.location);
            } else if(this->node1 == link.node2) {
                return mpilib::geo::angle_between(this->node1.location, this->node2.location, link.node1.location);
            } else if(this->node2 == link.node1) {
                return mpilib::geo::angle_between(this->node2.location, this->node1.location, link.node2.location);
            } else {
                return mpilib::geo::angle_between(this->node2.location, this->node1.location, link.node1.location);
            }
        }

        bool has_common_node(const Link &link) const {
            return has_node(link.node1) || has_node(link.node2);
        }

        bool has_node(const Node &node) const {
            return has_node(node.id);
        }

        bool has_node(const int &rhs) const {
            return this->node1 == rhs || this->node2 == rhs;
        }
    };
}

#endif //MANETSIMS_LINK_H
