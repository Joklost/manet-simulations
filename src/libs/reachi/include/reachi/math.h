#ifndef MANETSIMS_MATH_H
#define MANETSIMS_MATH_H

#include <vector>
#include <random>
#include <map>
#include <cmath>
#include <mpilib/helpers.h>
#include <mpilib/geomath.h>
#include <mpilib/defines.h>

#include "link.h"
#include "constants.h"
#include "clustering.h"


/**
 * https://rosettacode.org/wiki/Matrix_multiplication
 */

template<typename T>
std::pair<unsigned long, unsigned long> shape(const vecvec<T> &matrix) {
    return std::make_pair(matrix.size(), matrix[0].size());
}


template<typename T>
vecvec<T> operator*(const std::vector<T> &lhs, const std::vector<T> &rhs) {
    vecvec<T> res;
    auto rhs_size = rhs.size();
    res.resize(lhs.size(), std::vector<T>(rhs_size));

    for (auto row = 0; row < lhs.size(); ++row) {
        for (auto column = 0; column < rhs.size(); ++column) {
            res[row][column] = lhs[row] * rhs[column];
        }
    }

    return res;
}

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


template<typename T>
vecvec<T> operator/(const vecvec<T> &lhs, const T scalar) {
    vecvec<T> res;
    auto size = lhs.size();
    res.resize(size, std::vector<T>(size));

    for (auto row = 0; row < size; ++row) {
        for (auto column = 0; column < size; ++column) {
            res[row][column] = lhs[row][column] / scalar;
        }
    }

    return res;
}

template<typename T>
std::vector<T> operator/(const std::vector<T> &lhs, const T scalar) {
    std::vector<T> res;
    auto size = lhs.size();
    res.resize(size);

    for (auto i = 0; i < size; ++i) {
        res[i] = lhs[i] / scalar;
    }
    return res;
}


template<typename T>
vecvec<T> dot(const vecvec<T> &lhs, const vecvec<T> &rhs) {
    vecvec<T> res{};

    auto ls = shape(lhs);
    auto rs = shape(rhs);
    unsigned long n, m;

    //const vecvec<T>& n_matrix = std::move(lhs), m_matrix = std::move(rhs);

    if (ls.first < rs.first) {
        n = ls.first;
        m = rs.second;
    } else {
        n = rs.first;
        m = ls.second;
        /*n_matrix = rhs;
        m_matrix = lhs;*/
    }
    res.resize(n, std::vector<T>(m));

    for (auto row = 0; row < n; ++row) {
        for (auto column = 0; column < m; ++column) {
            auto sum = 0.0;

            for (auto i = 0; i < n; ++i) {
                sum += n < m ?
                       lhs[row][i] * rhs[i][column] :
                       rhs[row][i] * lhs[i][column];
            }

            res[row][column] = sum;
        }
    }
    return res;
}


template<typename T>
std::vector<T> dot(const vecvec<T> &lhs, const std::vector<T> &rhs) {
    assert(lhs.size() == rhs.size());
    std::vector<T> res{};
    auto size = lhs.size();
    res.resize(size);

    for (auto i = 0; i < size; ++i) {
        auto sum = (T) 0;

        for (auto j = 0; j < size; ++j) {
            sum += lhs[i][j] * rhs[j];
        }
        res[i] = sum;
    }

    return res;
}

template<typename T>
std::vector<T> dot(const std::vector<T> &lhs, const vecvec<T> &rhs) {
    return dot(rhs, lhs);
}

template<typename T>
T dot(const std::vector<T> &lhs, const std::vector<T> &rhs) {
    assert(lhs.size() == rhs.size());
    T res = 0;

    for (auto i = 0; i < lhs.size(); ++i) {
        res += lhs[i] * rhs[i];
    }

    return res;
}

/***
 *
 * @tparam T
 * @param lhs vector to slice
 * @param start index to begin slicing from
 * @param end index - 1 to end slicing
 * @return
 */
template<typename T>
std::vector<T> slice(const std::vector<T> &lhs, int start = 0, int end = 0ul) {
    std::vector<T> res;

    if (end == 0ul)
        end = static_cast<int>(lhs.size());

    for (; start < end; start++) {
        res.emplace_back(lhs[start]);
    }

    return res;
}

/***
 *
 * @tparam T
 * @param lhs matrix to slice from
 * @param row_start row index to start slicing from
 * @param row_end row index to end slicing from
 * @param column_start column index to start slicing from
 * @param column_end column index -1 to end slicing from
 * @return
 */
template<typename T>
vecvec<T> slice(const vecvec<T> &lhs, int row_start = 0, int row_end = 0ul, int column_start = 0, int column_end = 0) {
    vecvec<T> res;

    if (row_end == 0ul)
        row_end = static_cast<int>(lhs.size() - 1);

    for (; row_start <= row_end; ++row_start) {
        res.emplace_back(slice(lhs[row_start], column_start, column_end));
    }

    return res;
}

/***
 *
 * @tparam T
 * @param lhs matrix to slice from
 * @param column_index index for column value
 * @param row_start start index
 * @param row_end end index
 * @return
 */
template<typename T>
std::vector<T> slice_to_vector(const vecvec<T> &lhs, int column_index, int row_start = 0, int row_end = 0ul) {
    std::vector<T> res;

    if (row_end == 0ul) row_end = static_cast<int>(lhs.size());

    for (; row_start < row_end; ++row_start) {
        res.emplace_back(lhs[row_start][column_index]);
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
    auto rows = matrix.size();
    auto row_size = matrix[0].size();

    vecvec<T> vec{};
    vec.resize(rows, std::vector<T>(row_size));

    for (auto i = 0; i < rows; i++) {
        for (auto j = 0; j < row_size; j++) {
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

double frobenius_norm(std::vector<double> &a);

double
is_eigen_right(unsigned long n, unsigned long k, vecvec<double> &a, vecvec<double> &x, std::vector<double> &lambda);

std::vector<double> get_diagonal(unsigned long n, vecvec<double> &a);

Eigen eig(const vecvec<double> &a, unsigned long it_max);

vecvec<double> diag(std::vector<double> &v);

double distance_pathloss(double distance);

double distance_pathloss(mpilib::geo::Location to, mpilib::geo::Location from);

double distance_pathloss(Link link);

double autocorrelation(double angle);

double autocorrelation(mpilib::geo::Location to, mpilib::geo::Location from);

double autocorrelation(Link link);

vecvec<double> generate_correlation_matrix(std::vector<Optics::CLink> links);

vecvec<double> generate_correlation_matrix_slow(std::vector<Link> links);

vecvec<double> generate_correlation_matrix(std::vector<Link> links);

bool common_node(Link &k, Link &l);

#endif /* MANETSIMS_MATH_H */
