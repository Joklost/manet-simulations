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
#include "linalg.h"

namespace reachi {
    namespace math {

        template<typename T>
        linalg::vec<T> generate_gaussian_vector(const T mean, const T std_deviation, const unsigned long count) {
            ::std::vector<T> vec{};
            vec.reserve(count);

            ::std::random_device rd{};
            ::std::mt19937 gen{rd()};
            ::std::normal_distribution<T> distribution{mean, std_deviation};
            for (auto i = 0; i < count; i++) {
                vec.emplace_back(distribution(gen));
            }

            return vec;
        }

        double distance_pathloss(double distance);

        double distance_pathloss(mpilib::geo::Location to, mpilib::geo::Location from);

        double distance_pathloss(Link link);

        double distance_pathloss(Optics::CLink link);

        double autocorrelation(double angle);

        linalg::vecvec<double> generate_correlation_matrix(::std::vector<Optics::CLink> links);

        linalg::vecvec<double> generate_correlation_matrix_slow(::std::vector<Link> links);

        linalg::vecvec<double> generate_correlation_matrix(::std::vector<Link> links);

        bool common_node(Link &k, Link &l);


    }
}

#endif /* MANETSIMS_MATH_H */