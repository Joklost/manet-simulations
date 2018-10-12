#ifndef MANETSIMS_MATH_H
#define MANETSIMS_MATH_H

#include <vector>
#include <random>
#include <mpilib/geomath.h>

#include "constants.h"

template<typename T>
using vecvec = std::vector<std::vector<T>>;

/**
 * https://rosettacode.org/wiki/Matrix_multiplication
 */
template<typename T>
vecvec<T> operator*(const vecvec<T> &lhs, const vecvec<T> &rhs) {
    vecvec<T> res;

    auto n = lhs[0].size();
    auto m = lhs.size();
    auto p = rhs[0].size();
    if (m != p) {
        throw "The number of columns of the first matrix must equal the number of rows of the second matrix.";
    }
    res.resize(m, std::vector<T>(p));

    for (auto i = 0; i < m; i++) {
        for (auto j = 0; j < p; j++) {
            for (auto k = 0; k < n; k++) {
                res[i][j] += lhs[i][k] * rhs[k][j];
            }
        }
    }

    return res;
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

double distance_pathloss(double distance);

double distance_pathloss(Location to, Location from);

double autocorrelation(double angle);

double autocorrelation(Location to, Location from);

#endif /* MANETSIMS_MATH_H */
