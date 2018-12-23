#ifndef MANETSIMS_MPI_NODE_H
#define MANETSIMS_MPI_NODE_H

#include "location.h"

namespace mpilib {

    class Node {
    public:
        int rank{};
        mpilib::geo::Location loc{};
        unsigned long localtime{};
        bool dead{};

        bool operator==(const Node &rhs) const;

        bool operator!=(const Node &rhs) const;
    };

}

#endif /* MANETSIMS_MPI_NODE_H */
