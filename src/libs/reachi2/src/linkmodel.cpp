#include <Eigen/Cholesky>
#include <Eigen/SVD>
#include <Eigen/Eigenvalues>
#include <iostream>
#include <random>

#include <mpilib/helpers.h>
#include "reachi2/constants.h"
#include "reachi2/linkmodel.h"

void reachi2::Linkmodel::find_neighbourhood(const reachi2::Link &link) {
    for (const auto &l : this->graph) {
        if (l == link)
            continue;

        if (l.has_common_node(link))
            this->neighbourhoods[link.id].emplace_back(l.id);
    }
}

void reachi2::Linkmodel::compute() {
    for (const auto &link : this->graph) {
        this->find_neighbourhood(link);
        auto fading = this->generate_fading_value(link);
        this->model[link.id] = this->pathloss_values[link.id] * fading;
    }
}

const double reachi2::Linkmodel::generate_fading_value(const reachi2::Link &link) {
//    step 1: iterate neighbourhood
//    step 2: calc random normal val based on angle
//    step 3: sum then normalize
    auto total = this->neighbourhoods[link.id].size();
    auto sum = 0.0;

    for (const auto &neighbour : this->neighbourhoods[link.id]) {
        auto angle = link.angle_between(this->graph[neighbour]);
        auto deviation = total / (STANDARD_DEVIATION * (angle / 180) * 1.2);
        auto val = this->generate_gaussian_value(0, deviation);

        sum += val;
    }

    return sum;
}

reachi2::Linkmodel::Linkmodel(std::vector<reachi2::Link> &links) {
    this->graph = links;
    for (const auto &l : this->graph) {
        this->pathloss_values.emplace_back(this->distance_pathloss(l.distance));
    }
}

reachi2::Linkmodel::Linkmodel(const std::vector<reachi2::Node> &nodes, const double threshold = 0.55 /* KM */) {
    unsigned int link_id = 0;
    auto n_size = nodes.size();

    for (auto i = 0; i < n_size; ++i) {
        for (auto j = i; j < n_size; ++j) {
            if (i == j)
                continue;

            Link l(link_id, nodes[i], nodes[j]);
            if (l.distance < threshold or threshold <= 0.01) {
                link_id++;
                this->graph.emplace_back(l);
                this->pathloss_values.emplace_back(this->distance_pathloss(l.distance));
            }
        }
    }
}

double reachi2::Linkmodel::distance_pathloss(const double distance) const {
    return (10 * PATHLOSS_EXPONENT) * std::log10(distance) + PATHLOSS_CONSTANT_OFFSET;
}

const double reachi2::Linkmodel::generate_gaussian_value(double mean, double std_deviation) const {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> distribution{mean, std_deviation};
    return distribution(gen);
}
