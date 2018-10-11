#ifndef MANETSIMS_MATH_H
#define MANETSIMS_MATH_H

#include <vector>
#include <cmath>
#include <iostream>
#include <random>

#include "mpilib/helpers.h"


template<typename T>
vecvec<T> operator*(vecvec<T> const &rhs) {

}


template<typename T>
vecvec<T> &operator*=(vecvec<T> const &self, vecvec<T> const &rhs) {

}


template<typename T>
std::vector<T> gaussian_vector(const T mean, const T std_deviation, const uint count) {
    std::vector<T> vec{};
    vec.reserve(count);

    std::default_random_engine generator;
    std::normal_distribution<T> distribution{mean, std_deviation};
    for (auto i = 0; i < count; i++) {
        vec.push_back(distribution(generator));
    }

    return vec;
}

template<typename T>
vecvec<T> cholesky(const vecvec<T> matrix) {
    auto size = matrix.size();
    vecvec<T> vec{};
    vec.resize(size, std::vector<T>(size));

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < (i + 1); j++) {
            T sum = (T) 0;
            for (int k = 0; k < j; k++) {
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


#endif /* MANETSIMS_MATH_H */
