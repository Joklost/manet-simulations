#ifndef MANETSIMS_LINALG_H
#define MANETSIMS_LINALG_H

#include <Eigen/Core>

namespace reachi2 {
    namespace linalg {
        Eigen::MatrixXd identity(int size) {
            return Eigen::MatrixXd::Identity (size, size);
        }
    }
}

#endif //MANETSIMS_LINALG_H
