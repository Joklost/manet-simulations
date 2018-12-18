#ifndef MANETSIMS_SVD_H
#define MANETSIMS_SVD_H

#include <array>
#include <complex>

template<typename T>
std::vector<T> random_unit_vector(const unsigned long size) {
    auto unnormalized = generate_gaussian_vector(0.0, 1.0, size);
    auto norm = frobenius_norm(unnormalized);
    return unnormalized / norm;
}

template<typename T>
std::vector<T> svd_1d(const vecvec<T> &matrix, const double &epsilon) {
    std::vector<T> last_v, current_v = random_unit_vector<T>(matrix.size());

    auto b = dot(matrix, transpose(matrix));

    while (true) {
        last_v = current_v;
        current_v = dot(b, last_v);
        current_v = current_v / frobenius_norm(current_v);

        if (std::abs(dot(current_v, last_v)) > 1 - epsilon) {
            return current_v;
        }
    }
}

/***
 * Calculate the singular value decomposition (SVD)
 * @tparam T
 * @param matrix
 * @param epsilon
 * @return tuple(singular_values, u, v)
 */
template<typename T>
std::tuple<std::vector<T>, vecvec<T>, vecvec<T>> svd(const vecvec<T> &matrix, const double epsilon = 1e-10) {
    std::vector<T> singular_values;
    vecvec<T> us, vs;

    for (auto i = 0; i < matrix.size(); ++i) {
        auto matrix_for_1d = matrix;

        for (auto j = 0; j < i; ++j) matrix_for_1d = matrix_for_1d - singular_values[j] * (us[j] * vs[j]);

        auto u = svd_1d(matrix_for_1d, epsilon);
        auto v_unnormalized = dot(transpose(matrix), u);
        auto sigma = frobenius_norm(v_unnormalized);
        auto v = v_unnormalized / sigma;

        singular_values.emplace_back(sigma);
        us.emplace_back(u);
        vs.emplace_back(v);
    }

    return std::make_tuple(singular_values, transpose(us), vs);
}

#endif //MANETSIMS_SVD_H