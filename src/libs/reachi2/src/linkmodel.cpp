#include <Eigen/Cholesky>
#include <Eigen/SVD>
#include <Eigen/Eigenvalues>
#include <iostream>
#include <random>

#include <reachi2/constants.h>
#include <reachi2/linkmodel.h>
#include <reachi2/datagen.h>
#include <reachi2/radiomodel.h>
#include <iomanip>

void reachi2::Linkmodel::find_neighbourhood(const reachi2::Link &link) {
    this->neighbourhoods[link.id].clear();

    for (const auto &l : this->links) {
        if (l == link)
            continue;

        if (l.has_common_node(link))
            this->neighbourhoods[link.id].emplace_back(l.id);
    }
}

long double reachi2::Linkmodel::compute(std::vector<double> cholesky) {
    this->pep.clear();
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> distribution{0.0, 1.0};

    int count = 0;
    long double total = 0.0;
//    std::cout << "cholesky   fading     difference   neighbourhood size" << std::endl;
//    std::cout << "cholesky   fading     rssi       pep    neighbourhood size" << std::endl;
//    std::cout << "     cholesky     ||     new model      ||     difference     " << std::endl;

    for (const auto &link : this->links) {
        find_neighbourhood(link);
        double sum = 0.0;
        auto auto_sum = 0.0;
        for (const auto &neighbour : this->neighbourhoods[link.id]) {
            auto angle = link.angle_between(this->links[neighbour]);
            auto autocorrelation = compute_autocorrelation(angle);
            auto_sum += autocorrelation;
        }
        sum = std::sqrt(auto_sum * STANDARD_DEVIATION) + STANDARD_DEVIATION - std::pow(auto_sum, 4);

        auto l_d = distance_pathloss(link.distance);
        auto rssi_org = TX_POWER - (l_d + (-1 * cholesky[count++]));
        auto pep_org = reachi2::radiomodel::pep(rssi_org, PACKET_SIZE);

        auto rssi_new = TX_POWER - (l_d + (-1 * sum));
        auto pep_new = reachi2::radiomodel::pep(rssi_new, PACKET_SIZE);

        /*std::cout
        << std::left
        << std::setw(13) << rssi_org
        << "||     " << std::setw(15) << rssi_new
        << "||     " << std::setw(16) << rssi_org - rssi_new
        << std::endl;*/

        total += std::abs(rssi_org - rssi_new);
       /* std::cout
                << std::left
                << std::setw(11) << cholesky[count]
                << std::setw(11) << sum
                << std::setw(13) << cholesky[count++] - sum
//                << std::setw(11) << rssi
//                << std::setw(7) << pep
                << this->neighbourhoods[link.id].size() << std::endl;*/
    }
    return total;
}

const double reachi2::Linkmodel::compute_autocorrelation(const double angle) const {
    return 0.595 * std::exp(-0.064 * angle) + 0.092;
}

reachi2::Linkmodel::Linkmodel(std::vector<reachi2::Link> &links) {
    this->links = links;
}

reachi2::Linkmodel::Linkmodel(std::vector<reachi2::Node> &nodes,
                              const double threshold) {
    this->links = reachi2::data::create_links(nodes, threshold);
}

double reachi2::Linkmodel::distance_pathloss(const double distance) {
    return (10 * PATHLOSS_EXPONENT) * std::log10(distance) + PATHLOSS_CONSTANT_OFFSET;
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
