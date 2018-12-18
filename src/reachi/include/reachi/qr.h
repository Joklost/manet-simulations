#ifndef MANETSIMS_QR_H
#define MANETSIMS_QR_H

#include "math.h"

template<typename T>
vecvec<T> add_row_axis(const std::vector<T> vec) {
    return vecvec<T>{vec};
}

template<typename T>
vecvec<T> add_column_axis(const std::vector<T> vec) {
    vecvec<T> res;

    for (const auto &item : vec) {
        res.emplace_back(std::vector<T>{item});
    }

    return res;
}

template<typename T>
vecvec<T> make_householder(std::vector<T> &matrix) {
    auto v = matrix / (matrix[0] + std::copysign(frobenius_norm(matrix), matrix[0])); // v should be a vector
    v[0] = (T) 1;

    auto h = identity(matrix.size());
    auto val_1 = (2 / dot(v, v));
    auto val_3 = add_row_axis(v);
    auto val_4 = add_column_axis(v);
    /*std::cout << val_3 << std::endl;
    std::cout << "" << std::endl;
    std::cout << val_4 << std::endl;*/
    auto val_2 = dot(add_row_axis(v), add_column_axis(v));
    std::cout << val_2 << std::endl;
    auto val = (2 / dot(v, v)) * dot(add_column_axis(v), add_row_axis(v));
    std::cout << val << std::endl;
    //h = h - val;
    return vecvec<T> {};
}

template<typename T>
void qr_decomposition(vecvec<T> &matrix) {
    auto matrix_size = matrix.size();

    auto Q = identity(matrix_size);
    for (auto i = 0; i < matrix_size - 1; ++i) {
        auto h = identity(matrix_size);
        make_householder(slice_to_vector(matrix, i, 0, i));


    }
}

#endif //MANETSIMS_QR_H
