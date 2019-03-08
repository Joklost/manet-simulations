#ifndef MANETSIMS_LINALG_H
#define MANETSIMS_LINALG_H

#include <cassert>
#include <vector>
#include <cmath>

#include <common/equality.h>

namespace reachi {
    namespace linalg {

        template<typename T>
        using vec = ::std::vector<T>;

        template<typename T>
        using vecvec = ::std::vector<::std::vector<T>>;

        struct Eigen {
            linalg::vecvec<double> vectors{};
            ::std::vector<double> values{};

            unsigned long iterations{};
            unsigned long rotations{};
        };

        ::std::vector<double> get_diagonal(linalg::vecvec<double> &a, unsigned long n = 0ul);

        linalg::vecvec<double> diag(::std::vector<double> &v);

        linalg::vecvec<double> identity(unsigned long n);

        Eigen eig(const linalg::vecvec<double> &a, unsigned long it_max);

        double is_eigen_right(unsigned long n, unsigned long k,
                              linalg::vecvec<double> &a,
                              linalg::vecvec<double> &x,
                              ::std::vector<double> &lambda);

        double frobenius_norm(linalg::vecvec<double> &a);

        double frobenius_norm(::std::vector<double> &a);

        double infinity_norm(linalg::vecvec<double> &matrix);

        template<typename T>
        vecvec<T> transpose(const vecvec<T> matrix) {
            vecvec<T> vec{};
            auto rows = matrix.size();
            auto row_size = matrix.front().size();

            if (rows == row_size) {
                vec.resize(rows, ::std::vector<T>(row_size));

                for (auto i = 0; i < rows; i++) {
                    for (auto j = 0; j < row_size; j++) {
                        vec[j][i] = matrix[i][j];
                    }
                }

                return vec;
            }

            vec.resize(row_size, ::std::vector<T>(rows));

            for (auto i = 0; i < row_size; i++) {
                for (auto j = 0; j < rows; j++) {
                    vec[i][j] = matrix[j][i];
                }
            }
            return vec;
        }


        template<typename T>
        vecvec<T> normalize_vecvec(const vecvec<T> &matrix, const T min, const T max) {
            vecvec<T> res;
            auto size = matrix.size();
            res.resize(size, std::vector<T>(size));

            for (auto row = 0; row < size; ++row) {
                for (auto column = 0; column < size; ++column) {
                    res[row][column] = (matrix[row][column] - min) / (max - min);
                }
            }

            return res;
        }

        template<typename T>
        ::std::pair<unsigned long, unsigned long> shape(const vecvec<T> &matrix) {
            return ::std::make_pair(matrix.size(), matrix[0].size());
        }

        template<typename T>
        vecvec<T> diag(const vecvec<T> &lhs) {
            vecvec<T> res;
            auto size = lhs.size();
            res.resize(size, ::std::vector<T>(size));

            for (auto i = 0; i < size; ++i)
                res[i][i] = lhs[i][i];

            return res;
        }

        template<typename T>
        vecvec<T> diag(const vecvec<T> &lhs, const T value) {
            auto res = lhs;

            for (auto i = 0; i < lhs.size(); ++i) {
                res[i][i] = value;
            }

            return res;
        }

        template<typename T>
        vecvec<T> crossprod(const vecvec<T> &lhs, const vecvec<T> &rhs) {
            vecvec<T> res, a, b;
            unsigned long a_size, b_size;

            a = lhs;
            b = rhs;

            a_size = a.front().size();
            b_size = b.size();
            res.resize(a_size, ::std::vector<T>(b_size));

            for (auto i = 0; i < a_size; ++i) {
                for (auto j = 0; j < b_size; ++j) {
                    auto sum = (T) 0;
                    if (i == j) {
                        for (auto k = 0; k < a.size(); ++k)
                            sum += std::pow(a[k][i], 2);
                    } else {
                        for (auto k = 0; k < a.size(); ++k)
                            sum += a[k][i] * b[j][k];
                    }

                    res[i][j] = sum;
                }
            }

            return res;
        }

        template<typename T>
        vecvec<T> crossprod(const vecvec<T> &lhs) {
            return crossprod(lhs, transpose(lhs));
        }

        /***
         * Calculate the dot product of two matrixes. If asymmetric eg. 2x3 3x2, then the matrix with the lowest row count must be first argument
         * @tparam T
         * @param lhs N x M matrix where n <= m
         * @param rhs N x M matrix where n >= m
         * @return N x N matrix
         */
        template<typename T>
        vecvec<T> dot(const vecvec<T> &lhs, const vecvec<T> &rhs) {
            vecvec<T> res;

            auto ls = shape(lhs);
            auto rs = shape(rhs);
            unsigned long n = ls.first, m = rs.second;
            res.resize(n, ::std::vector<T>(m));

            for (auto row = 0; row < n; ++row) {
                for (auto column = 0; column < m; ++column) {
                    auto sum = 0.0;

                    for (auto i = 0; i < ls.second; ++i)
                        sum += lhs[row][i] * rhs[i][column];

                    res[row][column] = sum;
                }
            }
            return res;
        }


        template<typename T>
        ::std::vector<T> dot(const vecvec<T> &lhs, const ::std::vector<T> &rhs) {
            assert(lhs.size() == rhs.size());
            ::std::vector<T> res{};
            auto size = lhs.size();
            res.resize(size);

            for (auto i = 0; i < size; ++i) {
                auto sum = (T) 0;

                for (auto j = 0; j < size; ++j)
                    sum += lhs[i][j] * rhs[j];

                res[i] = sum;
            }

            return res;
        }

        template<typename T>
        ::std::vector<T> dot(const ::std::vector<T> &lhs, const vecvec<T> &rhs) {
            return dot(rhs, lhs);
        }

        template<typename T>
        T dot(const ::std::vector<T> &lhs, const ::std::vector<T> &rhs) {
            assert(lhs.size() == rhs.size());
            T res = 0;

            for (auto i = 0; i < lhs.size(); ++i) {
                res += lhs[i] * rhs[i];
            }

            return res;
        }

        template<typename T>
        vecvec<T> slice_column_from_index_list(const vecvec<T> &lhs, const ::std::vector<int> &indexes) {
            vecvec<T> res;
            auto size = lhs.size();
            res.resize(size, ::std::vector<T>{});

            for (const auto &index : indexes) {
                for (auto row = 0; row < lhs.size(); ++row) {
                    res[row].emplace_back(lhs[row][index]);
                }
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
        ::std::vector<T> slice(const ::std::vector<T> &lhs, int start = 0, int end = 0ul) {
            ::std::vector<T> res;

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
        vecvec<T>
        slice(const vecvec<T> &lhs, int row_start = 0, int row_end = 0ul, int column_start = 0, int column_end = 0) {
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
        ::std::vector<T> slice_to_vector(vecvec<T> &lhs, int column_index, int row_start = 0, int row_end = 0ul) {
            ::std::vector<T> res;

            if (row_end == 0ul) row_end = static_cast<int>(lhs.size());
            for (; row_start < row_end; ++row_start) {
                res.emplace_back(lhs[row_start][column_index]);
            }

            return res;
        }


#ifdef TWOBLUECUBES_SINGLE_INCLUDE_CATCH_HPP_INCLUDED

        template<typename T>
        bool compare_vectors(std::vector<T> a, std::vector<T> b, T epsilon) {
            if (a.size() != b.size()) return false;
            for (auto i = 0; i < a.size(); i++) {
                if (a[i] != Approx(b[i]).margin(epsilon)) {
                    return false;
                }
            }
            return true;
        }

        template<typename T>
        bool compare_vectors(vecvec<T> a, vecvec<T> b, T epsilon) {
            if (a.size() != b.size()) return false;
            for (auto i = 0; i < a.size(); i++) {
                if (!compare_vectors(a[i], b[i], epsilon)) return false;
            }
            return true;
        }

#else

        template<typename T>
        bool compare_vectors(std::vector<T> a, std::vector<T> b, T epsilon) {
            if (a.size() != b.size()) return false;
            for (auto i = 0; i < a.size(); i++) {
                if (!common::is_equal(a[i], b[i], epsilon)) {
                    return false;
                }
            }
            return true;
        }

        template<typename T>
        bool compare_vectors(vecvec<T> a, vecvec<T> b, T epsilon) {
            if (a.size() != b.size()) return false;
            for (auto i = 0; i < a.size(); i++) {
                if (!compare_vectors(a[i], b[i], epsilon)) return false;
            }
            return true;
        }

#endif

    }
}


template<typename T>
reachi::linalg::vecvec<T> operator*(const ::std::vector<T> &lhs, const ::std::vector<T> &rhs) {
    reachi::linalg::vecvec<T> res;
    auto size = lhs.size();
    //auto iter_size = size / 4;
    res.resize(size, ::std::vector<T>(size));

    for (auto row = 0; row < size; ++row) {
        for (auto column = 0; column < size; ++column) {
            res[row][column] = lhs[row] * rhs[column];
        }
    }

    return res;
}

template<typename T>
reachi::linalg::vecvec<T> operator*(const reachi::linalg::vecvec<T> &lhs, const reachi::linalg::vecvec<T> &rhs) {
    reachi::linalg::vecvec<T> res;

    auto n = lhs[0].size();
    auto m = lhs.size();
    auto p = rhs[0].size();
    if (m != p) {
        throw "The number of columns of the first matrix must equal the number of rows of the second matrix.";
    }
    res.resize(m, ::std::vector<T>(p));

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
::std::vector<T> operator*(const reachi::linalg::vecvec<T> &lhs, const ::std::vector<T> &rhs) {
    ::std::vector<T> res;
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
::std::vector<T> operator*(const ::std::vector<T> &lhs, const T scalar) {
    ::std::vector<T> res;
    res.resize(lhs.size());

    for (auto i = 0; i < lhs.size(); ++i) {
        res[i] = lhs[i] * scalar;
    }
    return res;
}

template<typename T>
::std::vector<T> operator*(const T scalar, const ::std::vector<T> &rhs) {
    return rhs * scalar;
}


template<typename T>
::std::vector<T> operator*(const ::std::vector<T> &lhs, const reachi::linalg::vecvec<T> &rhs) {
    return rhs * lhs;
}

template<typename T>
reachi::linalg::vecvec<T> operator*(const reachi::linalg::vecvec<T> &lhs, const T scale) {
    reachi::linalg::vecvec<T> res;

    auto n = lhs[0].size();
    auto m = lhs.size();
    res.resize(m, ::std::vector<T>(n));

    for (auto i = 0; i < m; ++i) {
        for (auto j = 0; j < n; ++j) {
            res[i][j] = lhs[i][j] * scale;
        }
    }

    return res;
}

template<typename T>
reachi::linalg::vecvec<T> operator*(const T scale, const reachi::linalg::vecvec<T> &rhs) {
    return rhs * scale;
}


template<typename T>
bool operator>(const reachi::linalg::vecvec<T> &lhs, const T value) {
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

template<typename T>
::std::vector<T> operator+(const ::std::vector<T> &lhs, const ::std::vector<T> &rhs) {
    if (lhs.size() != rhs.size()) {
        throw "Vectors must be of the same size";
    }
    ::std::vector<T> res;
    res.resize(lhs.size());

    for (int i = 0; i < lhs.size(); ++i) {
        res[i] = lhs[i] + rhs[i];
    }

    return res;
}

template<typename T>
reachi::linalg::vecvec<T> operator+(const reachi::linalg::vecvec<T> &lhs, const reachi::linalg::vecvec<T> &rhs) {
    if (lhs.size() != rhs.size() && lhs.front().size() != rhs.front().size())
        throw "Vecvec's must be of the same size";

    reachi::linalg::vecvec<T> res;
    auto size = lhs.size();
    res.resize(size, ::std::vector<T>(size));

    for (auto row = 0; row < size; ++row) {
        for (auto column = 0; column < size; ++column) {
            res[row][column] = lhs[row][column] + rhs[row][column];
        }
    }

    return res;
}


template<typename T>
reachi::linalg::vecvec<T> operator+(const reachi::linalg::vecvec<T> &lhs, const T scalar) {
    reachi::linalg::vecvec<T> res;
    auto size = lhs.size();
    res.resize(size, ::std::vector<T>(size));

    for (auto row = 0; row < size; ++row) {
        for (auto column = 0; column < size; ++column) {
            res[row][column] = lhs[row][column] + scalar;
        }
    }

    return res;
}

template<typename T>
reachi::linalg::vecvec<T> operator+(const T scalar, const reachi::linalg::vecvec<T> &rhs) {
    return rhs + scalar;
}

template<typename T>
::std::vector<T> operator+(const ::std::vector<T> &lhs, const T scalar) {
    ::std::vector<T> res;
    res.resize(lhs.size());

    for (int i = 0; i < lhs.size(); ++i) {
        res[i] = lhs[i] + scalar;
    }

    return res;
}

template<typename T>
::std::vector<T> operator+(const T scalar, const ::std::vector<T> &rhs) {
    return rhs + scalar;
}

template<typename T>
::std::vector<T> operator-(const ::std::vector<T> &lhs, const ::std::vector<T> &rhs) {
    if (lhs.size() != rhs.size()) {
        throw "Vectors must be of the same size";
    }
    ::std::vector<T> res;
    res.resize(lhs.size());

    for (int i = 0; i < lhs.size(); ++i) {
        res[i] = lhs[i] - rhs[i];
    }

    return res;
}

template<typename T>
::std::vector<T> operator-(const ::std::vector<T> &lhs, const T scalar) {
    ::std::vector<T> res;
    res.resize(lhs.size());

    for (int i = 0; i < lhs.size(); ++i) {
        res[i] = lhs[i] - scalar;
    }

    return res;
}

template<typename T>
reachi::linalg::vecvec<T> operator-(const reachi::linalg::vecvec<T> &lhs, const reachi::linalg::vecvec<T> &rhs) {
    if (lhs.size() != rhs.size() || lhs.front().size() != rhs.front().size()) {
        throw "Vectors must be of the same size";
    }

    auto n = lhs.size();
    auto m = lhs.front().size();

    reachi::linalg::vecvec<T> res;
    res.resize(n, ::std::vector<T>(m));

    for (auto i = 0; i < n; ++i) {
        for (auto j = 0; j < m; ++j) {
            res[i][j] = lhs[i][j] - rhs[i][j];
        }
    }

    return res;
}


template<typename T>
reachi::linalg::vecvec<T> operator/(const reachi::linalg::vecvec<T> &lhs, const T scalar) {
    reachi::linalg::vecvec<T> res;
    auto size = lhs.size();
    res.resize(size, ::std::vector<T>(size));

    for (auto row = 0; row < size; ++row) {
        for (auto column = 0; column < size; ++column) {
            res[row][column] = lhs[row][column] / scalar;
        }
    }

    return res;
}

template<typename T>
reachi::linalg::vec<T> operator/(const reachi::linalg::vec<T> &lhs, const T scalar) {
    ::std::vector<T> res;
    auto size = lhs.size();
    res.resize(size);

    for (auto i = 0; i < size; ++i) {
        res[i] = lhs[i] / scalar;
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
::std::vector<T> operator-(const T scalar, const ::std::vector<T> &rhs) {
    ::std::vector<T> res;
    res.resize(rhs.size());

    for (int i = 0; i < rhs.size(); ++i) {
        res[i] = scalar - std::abs(rhs[i]);
    }

    return res;
}


#endif /* MANETSIMS_LINALG_H */
