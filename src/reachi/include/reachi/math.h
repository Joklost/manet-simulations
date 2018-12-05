#ifndef MANETSIMS_MATH_H
#define MANETSIMS_MATH_H

#include <vector>
#include <random>
#include <map>
#include <mpilib/geomath.h>
#include <mpilib/link.h>

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

    for (auto i = 0; i < m; ++i) {
        for (auto j = 0; j < p; ++j) {
            for (auto k = 0; k < n; ++k) {
                res[i][j] += lhs[i][k] * rhs[k][j];
            }
        }
    }

    return res;
}

template<typename T>
std::vector<T> operator*(const vecvec<T> &lhs, const std::vector<T> &rhs) {
    std::vector<T> res;
    auto n = lhs[0].size();

    res.resize(n);

    if (n != rhs.size()) {
        throw "The number of columns of the first matrix must equal the number of rows of the second matrix.";
    }

    for (auto i = 0; i < lhs.size(); ++i) {
        T calc_res = 0;
        for (auto j = 0; j < rhs.size(); ++j) {
            calc_res += lhs[i][j] * rhs[j];
        }
        res[i] = calc_res;
    }

    return res;
}

template<typename T>
std::vector<T> operator*(const std::vector<T> &lhs, const T scalar) {
    std::vector<T> res;
    res.resize(lhs.size());

    for (auto i = 0; i < lhs.size(); ++i) {
        res[i] = lhs[i] * scalar;
    }
    return res;
}

template<typename T>
std::vector<T> operator*(const T scalar, const std::vector<T> &rhs) {
    return rhs * scalar;
}


template<typename T>
std::vector<T> operator*(const std::vector<T> &lhs, const vecvec<T> &rhs) {
    return rhs * lhs;
}

template<typename T>
vecvec<T> operator*(const vecvec<T> &lhs, const T scale) {
    vecvec<T> res;

    auto n = lhs[0].size();
    auto m = lhs.size();
    res.resize(m, std::vector<T>(n));

    for (auto i = 0; i < m; ++i) {
        for (auto j = 0; j < n; ++j) {
            res[i][j] = lhs[i][j] * scale;
        }
    }

    return res;
}

template<typename T>
vecvec<T> operator*(const T scale, const vecvec<T> &rhs) {
    return rhs * scale;
}

template<typename T>
std::vector<T> generate_gaussian_vector(const T mean, const T std_deviation, const unsigned long count) {
    std::vector<T> vec{};
    vec.reserve(count);

    std::random_device rd;
    std::default_random_engine generator{rd};
    std::normal_distribution<T> distribution{mean, std_deviation};
    for (auto i = 0; i < count; i++) {
        vec.emplace_back(distribution(generator));
    }

    return vec;
}

template<typename T>
std::vector<T> operator+(const std::vector<T> &lhs, const std::vector<T> &rhs) {
    if (lhs.size() != rhs.size()) {
        throw "Vectors must be of the same size";
    }
    std::vector<T> res;
    res.resize(lhs.size());

    for (int i = 0; i < lhs.size(); ++i) {
        res[i] = lhs[i] + rhs[i];
    }

    return res;
}

template<typename T>
std::vector<T> operator+(const std::vector<T> &lhs, const T scalar) {
    std::vector<T> res;
    res.resize(lhs.size());

    for (int i = 0; i < lhs.size(); ++i) {
        res[i] = lhs[i] + scalar;
    }

    return res;
}

template<typename T>
std::vector<T> operator+(const T scalar, const std::vector<T> &rhs) {
    return rhs + scalar;
}

template<typename T>
std::vector<T> operator-(const std::vector<T> &lhs, const std::vector<T> &rhs) {
    if (lhs.size() != rhs.size()) {
        throw "Vectors must be of the same size";
    }
    std::vector<T> res;
    res.resize(lhs.size());

    for (int i = 0; i < lhs.size(); ++i) {
        res[i] = lhs[i] - rhs[i];
    }

    return res;
}

template<typename T>
std::vector<T> operator-(const std::vector<T> &lhs, const T scalar) {
    std::vector<T> res;
    res.resize(lhs.size());

    for (int i = 0; i < lhs.size(); ++i) {
        res[i] = lhs[i] - scalar;
    }

    return res;
}

template<typename T>
std::vector<T> operator-(const T scalar, const std::vector<T> &rhs) {
    std::vector<T> res;
    res.resize(rhs.size());

    for (int i = 0; i < rhs.size(); ++i) {
        res[i] = scalar - rhs[i];
    }

    return res;
}

bool is_equal(double a, double b);

double distance_pathloss(double distance);

double distance_pathloss(Location to, Location from);

double distance_pathloss(Link link);

double autocorrelation(double angle);

double autocorrelation(Location to, Location from);

double autocorrelation(Link link);

vecvec<double> generate_correlation_matrix_slow(std::vector<Link> links);

vecvec<double> generate_correlation_matrix(std::vector<Link> links); // TODO: bedre funktions navn

bool common_node(Link &k, Link &l);

#endif /* MANETSIMS_MATH_H */
