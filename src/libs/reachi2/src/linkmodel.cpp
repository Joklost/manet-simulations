#include <Eigen/Cholesky>
#include <Eigen/SVD>
#include <Eigen/Eigenvalues>
#include <iostream>
#include <random>

#include <reachi2/constants.h>
#include <reachi2/linkmodel.h>
#include <reachi2/datagen.h>
#include <reachi2/radiomodel.h>

void reachi2::Linkmodel::find_neighbourhood(const reachi2::Link &link) {
    this->neighbourhoods[link.id].clear();

    for (const auto &l : this->links) {
        if (l == link)
            continue;

        if (l.has_common_node(link))
            this->neighbourhoods[link.id].emplace_back(l.id);
    }
}

void reachi2::Linkmodel::compute() {
    this->pep.clear();

    for (const auto &link : this->links) {
        auto l_d = distance_pathloss(link.distance * 1000);
        auto rssi = TX_POWER - l_d;
        auto pep = reachi2::radiomodel::pep(rssi, PACKET_SIZE);

//        if (pep == 1) {
        /*find_neighbourhood(link);
        double sum = 0.0;

        for (const auto &neighbour : this->neighbourhoods[link.id]) {
            auto angle = link.angle_between(this->links[neighbour]);
            auto fading = pep * angle;
            sum += fading;
        }*/

//        rssi = TX_POWER - (l_d);
        this->prev_rssi[link.id] = rssi;
        this->pep[link.id] = pep;
        std::cout
        << "distance = " << link.distance
        << ", l_d = " << l_d
        << ", rssi = " << rssi
        << ", pep = " << pep
        << std::endl;

        /*} else {
            this->prev_rssi[link.id] = rssi;
            this->pep[link.id] = pep;
        }*/
    }
}

reachi2::Linkmodel::Linkmodel(std::vector<reachi2::Link> &links) {
    this->links = links;
}

reachi2::Linkmodel::Linkmodel(std::vector<reachi2::Node> &nodes, const double threshold) {
    this->links = reachi2::data::create_links(nodes, threshold);
}

double reachi2::Linkmodel::distance_pathloss(const double distance) const {
    return (10 * PATHLOSS_EXPONENT) * std::log10(distance) - PATHLOSS_CONSTANT_OFFSET;
}

const double reachi2::Linkmodel::generate_gaussian_value(double mean, double std_deviation) const {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> distribution{mean, std_deviation};
    return distribution(gen);
}

const std::unordered_map<int, double> &reachi2::Linkmodel::get_pep() const {
    return pep;
}
