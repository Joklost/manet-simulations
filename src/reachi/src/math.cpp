#include <algorithm>
#include <limits>
#include <mpilib/geomath.h>
#include <mpilib/helpers.h>
#include <reachi/math.h>

double distance_pathloss(const double distance) {
    return (10 * PATHLOSS_EXPONENT) * std::log10(distance) + PATHLOSS_CONSTANT_OFFSET;
}

double distance_pathloss(const Location to, const Location from) {
    return distance_pathloss(distance_between(from, to) * KM);
}

double distance_pathloss(Link link) {
    return distance_pathloss(link.get_distance() * KM);
}

double autocorrelation(const double angle) {
    /* TODO: #define the constants */
    return 0.595 * std::exp(-0.064 * angle) + 0.092;
}

double autocorrelation(const Location to, const Location from) {
    /* TODO: Implement using angle_between */
    return 0;
}

double autocorrelation(Link link) {
    /* TODO: Implement using angle_between */
    return 0;
}

bool has_common_node_optics(const Optics::CLink &k, const Optics::CLink &l) {
    const auto &k_clusters = k.get_clusters();
    const auto &l_clusters = l.get_clusters();

    return k_clusters.first == l_clusters.first || k_clusters.first == l_clusters.second ||
           k_clusters.second == l_clusters.first || k_clusters.second == l_clusters.second;
}

Optics::Cluster get_common_node_optics(const Optics::CLink &k, const Optics::CLink &l) {
    const auto &k_clusters = k.get_clusters();
    const auto &l_clusters = l.get_clusters();

    if (k_clusters.first == l_clusters.first || k_clusters.first == l_clusters.second) {
        return k_clusters.first;
    } else if (k_clusters.second == l_clusters.first || k_clusters.second == l_clusters.second) {
        return k_clusters.second;
    } else {
        throw "Links have no common node.";
    }
}

bool has_common_node(const Link &k, const Link &l) {
    auto k_nodes = k.get_nodes();
    auto l_nodes = l.get_nodes();

    return k_nodes.first == l_nodes.first || k_nodes.first == l_nodes.second ||
           k_nodes.second == l_nodes.first || k_nodes.second == l_nodes.second;
}

Node get_common_node(const Link &k, const Link &l) {
    auto k_nodes = k.get_nodes();
    auto l_nodes = l.get_nodes();

    if (k_nodes.first == l_nodes.first || k_nodes.first == l_nodes.second) {
        return k_nodes.first;
    } else if (k_nodes.second == l_nodes.first || k_nodes.second == l_nodes.second) {
        return k_nodes.second;
    } else {
        throw "Links have no common node.";
    }
}

vecvec<double> generate_correlation_matrix(std::vector<Optics::CLink> links) {
    auto size = links.size();
    vecvec<double> corr{};
    corr.resize(size, std::vector<double>(size));

    std::sort(links.begin(), links.end());

    for (auto i = 0; i < size; ++i) {
        for (auto j = 0; j < size; ++j) {
            if (links[i] != links[j] && has_common_node_optics(links[i], links[j])) {
                auto common_node = get_common_node_optics(links[i], links[j]);
                auto li_unique = links[i].get_clusters().first.get_id() == common_node.get_id() ?
                                 links[i].get_clusters().second :
                                 links[i].get_clusters().first;

                auto lj_unique = links[j].get_clusters().first.get_id() == common_node.get_id() ?
                                 links[j].get_clusters().second :
                                 links[j].get_clusters().first;

                auto angle = angle_between(common_node.centroid(), li_unique.centroid(), lj_unique.centroid());
                corr[i][j] = autocorrelation(angle);
            } else if (links[i] == links[j]) {
                corr[i][j] = 1.0;
            } else {
                corr[i][j] = 0.0;
            }
        }
    }

    return corr;
}

vecvec<double> generate_correlation_matrix_slow(std::vector<Link> links) {
    auto size = links.size();
    vecvec<double> corr{};
    corr.resize(size, std::vector<double>(size));

    std::sort(links.begin(), links.end());

    for (auto i = 0; i < size; ++i) {
        for (auto j = 0; j < size; ++j) {
            if (links[i] != links[j] && has_common_node(links[i], links[j])) {
                auto common_node = get_common_node(links[i], links[j]);
                auto li_unique = links[i].get_nodes().first.get_id() == common_node.get_id() ?
                                 links[i].get_nodes().second :
                                 links[i].get_nodes().first;

                auto lj_unique = links[j].get_nodes().first.get_id() == common_node.get_id() ?
                                 links[j].get_nodes().second :
                                 links[j].get_nodes().first;

                auto angle = angle_between(common_node, li_unique, lj_unique);
                corr[i][j] = autocorrelation(angle);

            } else if (links[i] == links[j]) {
                corr[i][j] = 1.0;
            } else {
                corr[i][j] = 0.0;
            }
        }
    }

    return corr;
}

vecvec<double> generate_correlation_matrix(std::vector<Link> links) {
    auto size = links.size();
    vecvec<double> corr{};
    corr.resize(size, std::vector<double>(size));

    std::sort(links.begin(), links.end());

    for (auto i = 0; i < size; ++i) {
        for (auto j = 0; j < i + 1; ++j) {

            if (links[i] == links[j]) {
                corr[i][j] = 1.0;
            } else if (links[i] != links[j] && has_common_node(links[i], links[j])) {
                auto common_node = get_common_node(links[i], links[j]);
                auto li_unique = links[i].get_nodes().first.get_id() == common_node.get_id() ?
                                 links[i].get_nodes().second :
                                 links[i].get_nodes().first;

                auto lj_unique = links[j].get_nodes().first.get_id() == common_node.get_id() ?
                                 links[j].get_nodes().second :
                                 links[j].get_nodes().first;

                auto angle = angle_between(common_node, li_unique, lj_unique);
                auto value = autocorrelation(angle);
                corr[i][j] = value >= CORRELATION_COEFFICIENT_THRESHOLD ? value : 0.0;
            }
        }
    }

    return corr;
}


vecvec<double> identity(unsigned long n) {
    vecvec<double> ret{};
    ret.resize(n, std::vector<double>(n));

    for (auto i = 0; i < n; ++i) {
        ret[i][i] = 1.0;
    }

    return ret;
}

double frobenius_norm(vecvec<double> &a) {
    if (a.empty() or a.front().empty()) {
        return false;
    }
    auto n = a.size();
    auto m = a.front().size();

    auto val = 0.0;

    for (auto j = 0; j < n; ++j) {
        for (auto i = 0; i < m; ++i) {
            val = val + std::pow(a[i][j], 2);
        }
    }

    return std::sqrt(val);
}

double
is_eigen_right(unsigned long n, unsigned long k, vecvec<double> &a, vecvec<double> &x, std::vector<double> &lambda) {
    vecvec<double> c{};
    c.resize(n, std::vector<double>(k));

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

std::vector<double> get_diagonal(unsigned long n, vecvec<double> &a) {
    std::vector<double> v{};
    v.resize(n);

    for (auto i = 0; i < n; ++i) {
        v[i] = a[i][i];
    }

    return v;
}

Eigen eig(const vecvec<double> &c, unsigned long it_max) {
    auto a{c};
    auto n = a.size();
    auto v = identity(n);
    auto d = get_diagonal(n, a);

    std::vector<double> bw{}, zw{};
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

        thresh = std::sqrt(thresh) / (double) (4 * n);
        /* Break if threshold is pretty close to 0. */
        if (is_equal(thresh, 0.0, 0.005)) {
            break;
        }

        for (auto p = 0; p < n; ++p) {
            for (auto q = p + 1; q < n; ++q) {
                auto gapq = 10.0 * std::fabs(a[p][q]);
                auto termp = gapq + std::fabs(d[p]);
                auto termq = gapq + std::fabs(d[q]);
                double h;
                double g;

                /* Annihilate tiny offdiagonal elements. */
                if (4 < it_num && is_equal(termp, std::fabs(d[p])) && is_equal(termq, std::fabs(d[q]))) {
                    a[p][q] = 0.0;
                } else if (thresh <= std::fabs(a[p][q])) {
                    /* Otherwise, apply a rotation. */
                    h = d[q] - d[p];
                    auto term = std::fabs(h) + gapq;
                    double t;
                    if (is_equal(term, std::fabs(h))) {
                        t = a[p][q] / h;
                    } else {
                        auto theta = 0.5 * h / a[p][q];
                        t = 1.0 / (std::fabs(theta) + std::sqrt(1.0 + theta * theta));
                        if (theta < 0.0) {
                            t = -t;
                        }
                    }

                    auto c = 1.0 / std::sqrt(1.0 + t * t);
                    auto s = t * c;
                    auto tau = s / (1.0 + c);
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

    /* Ascending sort the eigenvalues and eigenvectors. */
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

vecvec<double> diag(std::vector<double> &v) {
    auto n = v.size();
    vecvec<double> c{};
    c.resize(n, std::vector<double>(n));

    for (auto i = 0; i < n; ++i) {
        c[i][i] = v[i];
    }

    return c;
}


