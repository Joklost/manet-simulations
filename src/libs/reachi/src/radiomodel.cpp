#include <reachi/radiomodel.h>
#include <reachi/constants.h>

double reachi::radiomodel::packet_error_probability(const double rssi, const int packetsize) {
    return 1.0 - std::pow((1.0 - ((1.0 / 2.0) * std::erfc(
            std::sqrt((std::pow(10.0, (rssi - (THERMAL_NOISE + NOISE_FIGURE)) / 10.0)) / 2.0)))),
                          static_cast<double>(packetsize));
}
