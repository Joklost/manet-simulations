#include "linkmodel/radiomodel.h"
#include "linkmodel/constants.h"

double packet_error_probability(const double rx_rssi, const int packetsize) {
    return 1.0 - std::pow((1.0 - ((1.0 / 2.0) * std::erfc(
            std::sqrt((std::pow(10.0, (rx_rssi - (THERMAL_NOISE + NOISE_FIGURE)) / 10.0)) / 2.0)))), packetsize);
}
