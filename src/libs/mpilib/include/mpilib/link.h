#ifndef MANETSIMS_MPI_LINK_H
#define MANETSIMS_MPI_LINK_H

#include <ostream>

namespace mpilib {

    class Link {
    public:
        int first{};
        int second{};
        double pathloss{};
        double distance{};

        friend std::ostream &operator<<(std::ostream &os, const Link &link1) {
            os << "Link{"
               << "first: " << link1.first
               << ", second: " << link1.second
               << ", pathloss: " << link1.pathloss
               << ", distance: " << link1.distance << "}";
            return os;
        }
    };

}

#endif //MANETSIMS_MPI_LINK_H
