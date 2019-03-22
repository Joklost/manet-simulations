#include <sims/radiomodel.h>
#include <sims/constants.h>

#include <iostream>

double sims::radiomodel::pep(double rssi, unsigned long packetsize, std::vector<double> interference) {
    auto P_N_dB = THERMAL_NOISE + NOISE_FIGURE;
    auto P_N = linearize(P_N_dB);

    auto P_I = 0.0;
    for (auto &RSSI_interference_dB : interference) {
        P_I += linearize(RSSI_interference_dB);
    }

    auto P_NI = P_N + P_I;
    auto P_NI_dB = logarithmicize(P_NI);
    auto SINR_dB = rssi - P_NI_dB;
    auto SINR = linearize(SINR_dB);

    auto bep = 0.5 * std::erfc(std::sqrt(SINR / 2.0));  /* Bit error probability. */
    auto pep = 1.0 - std::pow((1.0 - bep), packetsize * 8.0); /* Packet error probability. */
    return pep;
}

double sims::radiomodel::linearize(double logarithmic_value) {
    return std::pow(10, logarithmic_value / 10);
}

double sims::radiomodel::logarithmicize(double linear_value) {
    return 10 * std::log10(linear_value);
}
