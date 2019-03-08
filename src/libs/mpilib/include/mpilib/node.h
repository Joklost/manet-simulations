#ifndef MANETSIMS_MPI_NODE_H
#define MANETSIMS_MPI_NODE_H

#include <geo/geo.h>

namespace mpilib {

    class Node {
    public:
        int rank{};
        geo::Location loc{};
        unsigned long localtime{};
        bool dead{};

        bool operator==(const Node &rhs) const;

        bool operator!=(const Node &rhs) const;
    };

}

#endif /* MANETSIMS_MPI_NODE_H */
