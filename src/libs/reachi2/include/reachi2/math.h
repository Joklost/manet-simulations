#ifndef MANETSIMS_MATH_H
#define MANETSIMS_MATH_H

#include <Eigen/StdList>
#include <vector>

#include <mpilib/location.h>
#include "Link.h"


namespace reachi2 {
    namespace math {
        Eigen::MatrixXd generate_correlation_matrix(std::vector<reachi2::Link> &links);

        reachi2::Node get_common_node(const reachi2::Link &lhs, const reachi2::Link &rhs);

        double distance_pathloss(double distance);

        double distance_pathloss(mpilib::geo::Location &to, mpilib::geo::Location &from);

        double distance_pathloss(reachi2::Link &link);

        double autocorrelation(double angle);
    }
}


#endif //MANETSIMS_MATH_H
