#ifndef MANETSIMS_LINK_H
#define MANETSIMS_LINK_H

#include <mpilib/geomath.h>

namespace reachi2 {
    struct Node {
        int id;
        mpilib::geo::Location location;

        bool operator==(const Node &rhs) const {
            return id == rhs.id;
        }

        bool operator!=(const Node &rhs) const {
            return !(rhs == *this);
        }

        bool operator==(const int id2) const {
            return this->id == id2;
        }

        bool operator!=(const int id2) const {
            return this->id != id2;
        }
    };

    struct Link {
        int id;
        Node node1, node2;

        bool operator==(const Link &rhs) const {
            return id == rhs.id;
        }

        bool operator!=(const Link &rhs) const {
            return !(rhs == *this);
        }
    };
}

#endif //MANETSIMS_LINK_H
