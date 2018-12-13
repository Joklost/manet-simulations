#ifndef MANETSIMS_MATH_H
#define MANETSIMS_MATH_H

#include <vector>
#include <random>
#include <map>
#include <cmath>
#include <mpilib/geomath.h>
#include <mpilib/link.h>
#include <mpilib/helpers.h>

#include "constants.h"
#include "clustering.h"

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
    auto q = rhs.size();
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
bool operator>(const vecvec<T> &lhs, const T value) {
    if (lhs.empty() or lhs.front().empty()) {
        return false;
    }
    auto n = lhs.size();
    auto m = lhs.front().size();


    for (auto i = 0; i < n; ++i) {
        for (auto j = 0; j < m; ++j) {
            if (lhs[i][j] < value) {
                return false;
            }
        }
    }

    return true;
}
/*
template<typename T>
bool operator>(const T value, const vecvec<T> &rhs) {
    return rhs < value;
}

template<typename T>
bool operator<(const T value, const vecvec<T> &rhs) {
    return value < rhs;
}

template<typename T>
bool operator<(const vecvec<T> &lhs, const T value) {

}
*/
template<typename T>
std::vector<T> generate_gaussian_vector(const T mean, const T std_deviation, const unsigned long count) {
    std::vector<T> vec{};
    vec.reserve(count);

    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<T> distribution{mean, std_deviation};
    for (auto i = 0; i < count; i++) {
        vec.emplace_back(distribution(gen));
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
vecvec<T> operator-(const vecvec<T> &lhs, const vecvec<T> &rhs) {
    if (lhs.size() != rhs.size() || lhs.front().size() != rhs.front().size()) {
        throw "Vectors must be of the same size";
    }

    auto n = lhs.size();
    auto m = lhs.front().size();

    vecvec<T> res;
    res.resize(n, std::vector<T>(m));

    for (auto i = 0; i < n; ++i) {
        for (auto j = 0; j < m; ++j) {
            res[i][j] = lhs[i][j] - rhs[i][j];
        }
    }

    return res;
}

/**
 * Shorthand for subtracting a vector consisting of multiple elements of the same scalar.
 * @tparam T
 * @param scalar
 * @param rhs
 * @return
 */
template<typename T>
std::vector<T> operator-(const T scalar, const std::vector<T> &rhs) {
    std::vector<T> res;
    res.resize(rhs.size());

    for (int i = 0; i < rhs.size(); ++i) {
        res[i] = scalar - std::abs(rhs[i]);
    }

    return res;
}

template<typename T>
vecvec<T> transpose(const vecvec<T> matrix) {
    auto size = matrix.size();

    vecvec<T> vec{};
    vec.resize(size, std::vector<T>(size));

    for (auto i = 0; i < size; i++) {
        for (auto j = 0; j < size; j++) {
            vec[j][i] = matrix[i][j];
        }
    }

    return vec;
}

struct Eigen {
    vecvec<double> vectors{};
    std::vector<double> values{};

    unsigned long iterations{};
    unsigned long rotations{};
};

vecvec<double> identity(unsigned long n);

double frobenius_norm(vecvec<double> &a);

double
is_eigen_right(unsigned long n, unsigned long k, vecvec<double> &a, vecvec<double> &x, std::vector<double> &lambda);

std::vector<double> get_diagonal(unsigned long n, vecvec<double> &a);

Eigen eig(const vecvec<double> &a, unsigned long it_max);

vecvec<double> diag(std::vector<double> &v);

double distance_pathloss(double distance);

double distance_pathloss(Location to, Location from);

double distance_pathloss(Link link);

double autocorrelation(double angle);

double autocorrelation(Location to, Location from);

double autocorrelation(Link link);

vecvec<double> generate_correlation_matrix(std::vector<Optics::CLink> links);

vecvec<double> generate_correlation_matrix_slow(std::vector<Link> links);

vecvec<double> generate_correlation_matrix(std::vector<Link> links);

bool common_node(Link &k, Link &l);

#endif /* MANETSIMS_MATH_H */
