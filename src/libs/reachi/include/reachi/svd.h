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
std::vector<T> svd_1d(const vecvec<T> &matrix, const double &epsilon, const int max_iteration) {
    std::vector<T> last_v, current_v = random_unit_vector<T>(matrix.size());

    auto b = dot(matrix, transpose(matrix));
    int iteration = 1;

    while (true) {
        last_v = current_v;
        current_v = dot(b, last_v);
        current_v = current_v / frobenius_norm(current_v);

        if (std::abs(dot(current_v, last_v)) > 1 - epsilon || iteration == max_iteration) {
            return current_v;
        }
        iteration++;
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
std::tuple<std::vector<T>, vecvec<T>, vecvec<T>>
svd(const vecvec<T> &matrix, const int max_iteration, const double epsilon = 0.1 /*1e-10*/) {
    std::vector<T> singular_values;
    vecvec<T> us, vs, matrix_1d_cache;

    std::vector<vecvec<T>> us_times_vs;

    for (auto i = 0; i < matrix.size(); ++i) {
        std::cout << "svd iteration " << i + 1 << " of " << matrix.size() << std::endl;
        auto matrix_for_1d = matrix;

        if (i != 0) {
            matrix_for_1d = matrix_for_1d - singular_values.back() * (us.back() * vs.back());
            matrix_for_1d = matrix_for_1d - matrix_1d_cache;
        }

        matrix_1d_cache = matrix_for_1d;


        auto u = svd_1d(matrix_for_1d, epsilon, max_iteration);
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