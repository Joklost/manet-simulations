#include <cmath>
#include "radiomodel.h"

double RadioModel::lin(const double log) {
    return std::pow(10, log / 10);;
}

double RadioModel::log(const double lin) {
    return 10 * std::log10(lin);
}

double RadioModel::pep(const double rssi, const unsigned long packetsize, const std::vector<double> &interference) {
    auto P_N_dB = THERMAL_NOISE + NOISE_FIGURE;
    auto P_N = lin(P_N_dB);

    auto P_I = 0.0;
    for (auto &RSSI_interference_dB : interference) {
        P_I += lin(RSSI_interference_dB);
    }

    auto P_NI = P_N + P_I;
    auto P_NI_dB = log(P_NI);
    auto SINR_dB = rssi - P_NI_dB;
    auto SINR = lin(SINR_dB);

    auto bep = 0.5 * std::erfc(std::sqrt(SINR / 2.0));  /* Bit error probability. */
    auto pep = 1.0 - std::pow((1.0 - bep), packetsize * 8.0); /* Packet error probability. */
    return pep;
}
