#ifndef COORDINATOR_RADIOMODEL_H
#define COORDINATOR_RADIOMODEL_H


//
//double linklayer::linearize(double logarithmic_value) {
//    return std::pow(10, logarithmic_value / 10);
//}
//
//double linklayer::logarithmicize(double linear_value) {
//    return 10 * std::log10(linear_value);
//}
//
//double linklayer::pep(double rssi, unsigned long packetsize, const std::vector<double> &interference) {
//    auto P_N_dB = linklayer::THERMAL_NOISE + linklayer::NOISE_FIGURE;
//    auto P_N = linearize(P_N_dB);
//
//    auto P_I = 0.0;
//    for (auto &RSSI_interference_dB : interference) {
//        P_I += linearize(RSSI_interference_dB);
//    }
//
//    auto P_NI = P_N + P_I;
//    auto P_NI_dB = logarithmicize(P_NI);
//    auto SINR_dB = rssi - P_NI_dB;
//    auto SINR = linearize(SINR_dB);
//
//    auto bep = 0.5 * std::erfc(std::sqrt(SINR / 2.0));  /* Bit error probability. */
//    auto pep = 1.0 - std::pow((1.0 - bep), packetsize * 8.0); /* Packet error probability. */
//    return pep;
//}



#endif //COORDINATOR_RADIOMODEL_H
