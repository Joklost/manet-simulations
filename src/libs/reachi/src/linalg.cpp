#include <reachi/linalg.h>

#include <common/equality.h>

reachi::linalg::vecvec<double> reachi::linalg::identity(unsigned long n) {
    vecvec<double> ret{};
    ret.resize(n, ::std::vector<double>(n));

    for (auto i = 0; i < n; ++i) {
        ret[i][i] = 1.0;
    }

    return ret;
}


double reachi::linalg::frobenius_norm(vecvec<double> &a) {
    if (a.empty() or a.front().empty()) {
        return false;
    }
    auto n = a.size();
    auto m = a.front().size();

    auto val = 0.0;

    for (auto j = 0; j < n; ++j) {
        for (auto i = 0; i < m; ++i) {
            val = val + ::std::pow(a[i][j], 2);
        }
    }

    return ::std::sqrt(val);
}

double reachi::linalg::frobenius_norm(::std::vector<double> &a) {
    auto res = 0.0;

    for (double i : a) {
        res += ::std::pow(i, 2);
    }
    return ::std::sqrt(res);
}

double reachi::linalg::is_eigen_right(unsigned long n, unsigned long k,
                                      vecvec<double> &a,
                                      vecvec<double> &x,
                                      ::std::vector<double> &lambda) {
    vecvec<double> c{};
    c.resize(n, ::std::vector<double>(k));

    for (auto j = 0; j < k; ++j) {
        for (auto i = 0; i < n; ++i) {
            c[i][j] = 0.0;
            for (auto l = 0; l < n; ++l) {
                c[i][j] = c[i][j] + a[i][l] * x[l][j];
            }
        }
    }

    for (auto j = 0; j < k; ++j) {
        for (auto i = 0; i < n; ++i) {
            c[i][j] = c[i][j] - lambda[j] * x[i][j];
        }
    }

    return frobenius_norm(c);
}

std::vector<double> reachi::linalg::get_diagonal(reachi::linalg::vecvec<double> &a, unsigned long n) {
    std::vector<double> v{};
    if (n == 0ul) n = a.size();
    v.resize(n);

    for (auto i = 0; i < n; ++i) {
        v[i] = a[i][i];
    }

    return v;
}


reachi::linalg::Eigen reachi::linalg::eig(const vecvec<double> &c, unsigned long it_max) {
    auto a{c};
    auto n = a.size();
    auto v = identity(n);
    auto d = get_diagonal(a, n);

    ::std::vector<double> bw{}, zw{};
    bw.resize(n);
    zw.resize(n);

    for (auto i = 0; i < n; ++i) {
        bw[i] = d[i];
        zw[i] = 0.0;
    }

    unsigned long it_num = 0;
    unsigned long rot_num = 0;
    auto thresh = 0.0;

    while (it_num < it_max) {
        it_num++;

        /* The convergence threshold is based on the size of the
         * elements in the strict upper triangle of the matrix. */

        for (auto j = 0; j < n; ++j) {
            for (auto i = 0; i < j; ++i) {
                thresh = thresh + a[i][j] * a[i][j];
            }
        }

        thresh = ::std::sqrt(thresh) / (double) (4 * n);
        /* Break if threshold is pretty close to 0. */
        if (common::is_equal(thresh, 0.0, 0.005)) {
            break;
        }

        for (auto p = 0; p < n; ++p) {
            for (auto q = p + 1; q < n; ++q) {
                auto gapq = 10.0 * ::std::fabs(a[p][q]);
                auto termp = gapq + ::std::fabs(d[p]);
                auto termq = gapq + ::std::fabs(d[q]);
                double h;
                double g;

                /* Annihilate tiny offdiagonal elements. */
                if (4 < it_num && common::is_equal(termp, ::std::fabs(d[p])) &&
                    common::is_equal(termq, ::std::fabs(d[q]))) {
                    a[p][q] = 0.0;
                } else if (thresh <= ::std::fabs(a[p][q])) {
                    /* Otherwise, apply a rotation. */
                    h = d[q] - d[p];
                    auto term = ::std::fabs(h) + gapq;
                    double t;
                    if (common::is_equal(term, ::std::fabs(h))) {
                        t = a[p][q] / h;
                    } else {
                        auto theta = 0.5 * h / a[p][q];
                        t = 1.0 / (::std::fabs(theta) + ::std::sqrt(1.0 + theta * theta));
                        if (theta < 0.0) {
                            t = -t;
                        }
                    }

                    auto c1 = 1.0 / ::std::sqrt(1.0 + t * t);
                    auto s = t * c1;
                    auto tau = s / (1.0 + c1);
                    h = t * a[p][q];

                    /* Accumulate corrections to the diagonal elements. */
                    zw[p] = zw[p] - h;
                    zw[q] = zw[q] + h;
                    d[p] = d[p] - h;
                    d[q] = d[q] + h;

                    a[p][q] = 0.0;

                    /* Rotate, using information from the upper triangle of A only. */
                    for (auto j = 0; j < p; ++j) {
                        g = a[j][p];
                        h = a[j][q];
                        a[j][p] = g - s * (h + g * tau);
                        a[j][q] = h + s * (g - h * tau);
                    }

                    for (auto j = p + 1; j < q; ++j) {
                        g = a[p][j];
                        h = a[j][q];
                        a[p][j] = g - s * (h + g * tau);
                        a[j][q] = h + s * (g - h * tau);
                    }

                    for (auto j = q + 1; j < n; ++j) {
                        g = a[p][j];
                        h = a[q][j];
                        a[p][j] = g - s * (h + g * tau);
                        a[q][j] = h + s * (g - h * tau);
                    }

                    /* Accumulate information in the eigenvector matrix. */
                    for (auto j = 0; j < n; ++j) {
                        g = v[j][p];
                        h = v[j][q];
                        v[j][p] = g - s * (h + g * tau);
                        v[j][q] = h + s * (g - h * tau);
                    }

                    rot_num++;
                }
            }
        }

        for (auto i = 0; i < n; ++i) {
            bw[i] = bw[i] + zw[i];
            d[i] = bw[i];
            zw[i] = 0;
        }
    }

    /* Restore upper triangle of the input matrix. */
    for (auto j = 0; j < n; ++j) {
        for (auto i = 0; i < j; ++i) {
            a[i][j] = a[j][i];
        }
    }

    /* Descending sort the eigenvalues and eigenvectors. */
    for (auto k = 0; k < n - 1; ++k) {
        auto m = k;
        for (auto l = k + 1; l < n; ++l) {
            if (d[l] > d[m]) {
                m = l;
            }
        }

        if (m != k) {
            auto t = d[m];
            d[m] = d[k];
            d[k] = t;
            for (auto i = 0; i < n; ++i) {
                auto w = v[i][m];
                v[i][m] = v[i][k];
                v[i][k] = w;
            }
        }
    }

    Eigen result{v, d, it_num, rot_num};
    return result;
}

double reachi::linalg::infinity_norm(reachi::linalg::vecvec<double> &matrix) {
    double res = 0ul;

    for (const auto &row : matrix) {
        auto sum = 0.0;
        for (const auto &item : row)
            sum += item;

        if (res == 0ul || sum > res)
            res = sum;
    }

    return res;
}



