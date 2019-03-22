#ifndef MANETSIMS_QR_H
#define MANETSIMS_QR_H

#include "math.h"
#include <sims/ostr.h>

namespace sims {
    namespace qr {

        template<typename T>
        sims::linalg::vecvec<T>
        partial_update_vecvec(sims::linalg::vecvec<T> matrix, sims::linalg::vecvec<T> &householder_matrix,
                              int start_index) {
            auto h_shape = sims::linalg::shape(householder_matrix);

            for (auto row_h = 0, row_m = start_index; row_h < h_shape.first; row_m++, row_h++) {
                for (auto column_h = 0, column_m = start_index; column_h < h_shape.second; column_m++, column_h++) {
                    matrix[row_m][column_m] = householder_matrix[row_h][column_h];
                }
            }

            return matrix;
        }

        template<typename T>
        sims::linalg::vecvec<T> add_row_axis(const std::vector<T> vec) {
            return sims::linalg::vecvec<T>{vec};
        }

        template<typename T>
        sims::linalg::vecvec<T> add_column_axis(const std::vector<T> vec) {
            sims::linalg::vecvec<T> res;
            for (const auto &item : vec)
                res.emplace_back(std::vector<T>{item});

            return res;
        }

        template<typename T>
        sims::linalg::vecvec<T> make_householder(std::vector<T> &matrix) {
            auto v = matrix / (matrix[0] + std::copysign(linalg::frobenius_norm(matrix), matrix[0]));
            v[0] = (T) 1;

            auto h = linalg::identity(matrix.size());
            h = h - (2 / linalg::dot(v, v)) * linalg::dot(add_column_axis(v), add_row_axis(v));
            return h;
        }

        template<typename T>
        std::pair<sims::linalg::vecvec<T>, sims::linalg::vecvec<T>>
        qr_decomposition(sims::linalg::vecvec<T> matrix) {
            auto size = matrix.size();
            auto a = matrix;
            auto q = sims::linalg::identity(size);

            for (auto i = 0; i < size - 1; ++i) {
                auto h = sims::linalg::identity(size);
                auto slice = sims::linalg::slice_to_vector(a, i, i);
                auto householder_matrix = make_householder(slice);
                h = partial_update_vecvec(h, householder_matrix, i);
                q = sims::linalg::dot(q, h);
                a = sims::linalg::dot(h, a);
            }
            return std::make_pair(q, a);
        }

        /* http://www.ohiouniversityfaculty.com/youngt/IntNumMeth/lecture17.pdf */
        /**
         * Find the eigenvalues of a symmetric matrix
         * @tparam T
         * @param matrix
         * @return Eigen object with vectors and values
         */
        template<typename T>
        linalg::Eigen qr_algorithm(sims::linalg::vecvec<T> &matrix) {
            auto h = matrix;
            auto eigen_values = sims::linalg::get_diagonal(matrix, matrix.size());
            linalg::vecvec<double> eigen_vectors;
            double threshold = 1;
            long unsigned int iteration = 0;

            while (threshold > 0.1) {
                iteration++;
                auto e_old = eigen_values;

                /* apply QR method */
                auto qr = qr_decomposition(h);
                if (eigen_vectors.empty()) {
                    eigen_vectors = qr.first;
                } else {
                    eigen_vectors = eigen_vectors * qr.first;
                    //eigen_vectors = sims::linalg::dot(eigen_vectors, qr.first);
                }
                h = qr.second * qr.first;
                eigen_values = sims::linalg::get_diagonal(h, h.size());

                /* test for convergence */
                auto t = eigen_values - e_old;
                threshold = sims::linalg::frobenius_norm(t);
            }


            return linalg::Eigen{eigen_vectors, eigen_values, iteration, 0};
        }

    }
}

#endif //MANETSIMS_QR_H