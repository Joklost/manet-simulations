#ifndef MANETSIMS_CHOLESKY_H
#define MANETSIMS_CHOLESKY_H

#include <vector>
#include <cmath>
#include "math.h"

template<typename T>
vecvec<T> slow_cholesky(const vecvec<T> matrix) {
    auto size = matrix.size();
    vecvec<T> vec{};
    vec.resize(size, std::vector<T>(size));

    for (auto i = 0; i < size; i++) {
        for (auto j = 0; j < (i + 1); j++) {
            T sum = (T) 0;
            for (auto k = 0; k < j; k++) {
                sum += vec[i][k] * vec[j][k];
            }

            if (i == j) {
                vec[i][j] = std::sqrt(matrix[i][i] - sum);
            } else {
                vec[i][j] = ((T) 1) / vec[j][j] * (matrix[i][j] - sum);
            }

        }
    }

    return vec;
}


#endif /* MANETSIMS_CHOLESKY_H */
