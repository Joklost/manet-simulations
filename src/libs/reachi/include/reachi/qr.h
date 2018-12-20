#ifndef MANETSIMS_QR_H
#define MANETSIMS_QR_H

#include "math.h"

template<typename T>
vecvec<T> partial_update_vecvec(vecvec<T> matrix, vecvec<T> &householder_matrix, int start_index) {
    auto h_shape = shape(householder_matrix);

    for (auto row_h = 0, row_m = start_index; row_h < h_shape.first; row_m++, row_h++) {
        for (auto column_h = 0, column_m = start_index; column_h < h_shape.second; column_m++, column_h++) {
            matrix[row_m][column_m] = householder_matrix[row_h][column_h];
        }
    }

    return matrix;
}

template<typename T>
vecvec<T> add_row_axis(const std::vector<T> vec) {
    return vecvec<T>{vec};
}

template<typename T>
vecvec<T> add_column_axis(const std::vector<T> vec) {
    vecvec<T> res;
    for (const auto &item : vec)
        res.emplace_back(std::vector<T>{item});

    return res;
}

template<typename T>
vecvec<T> make_householder(std::vector<T> &matrix) {
    auto v = matrix / (matrix[0] + std::copysign(frobenius_norm(matrix), matrix[0]));
    v[0] = (T) 1;

    auto h = identity(matrix.size());
    h = h - (2 / dot(v, v)) * dot(add_column_axis(v), add_row_axis(v));
    return h;
}

template<typename T>
std::pair<vecvec<T>, vecvec<T>> qr_decomposition(vecvec<T> matrix) {
    auto size = matrix.size();
    auto a = std::move(matrix);
    auto q = identity(size);

    for (auto i = 0; i < size - 1; ++i) {
        auto h = identity(size);
        auto slice = slice_to_vector(a, i, i);
        auto householder_matrix = make_householder(slice);
        h = partial_update_vecvec(h, householder_matrix, i);
        q = dot(q, h);
        a = dot(h, a);
    }
    return std::make_pair(q, a);
}

// http://www.ohiouniversityfaculty.com/youngt/IntNumMeth/lecture17.pdf
template<typename T>
std::vector<T> qr_algorithm(vecvec<T> &matrix) {
    auto h = matrix;
    auto e = get_diagonal(matrix, matrix.size());
    int threshold = 1;
    int iteration = 0;

    while (threshold  > 0) {
        iteration++;
        std::cout << iteration << std::endl;
        auto e_old = e;

        /* apply QR method */
        auto qr = qr_decomposition(h);
        h = qr.second * qr.first;
        e = get_diagonal(h, h.size());

        /* test for convergence */
        auto t = e - e_old;
        threshold = (int) frobenius_norm(t);
    }
    return e;
}

#endif //MANETSIMS_QR_H
