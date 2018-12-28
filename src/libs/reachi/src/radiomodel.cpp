#include <reachi/radiomodel.h>
#include <reachi/constants.h>

double reachi::radiomodel::pep(double rssi, unsigned long packetsize, double interference) {
    auto noise_power = THERMAL_NOISE + NOISE_FIGURE;
    auto sinr_db = rssi - noise_power - interference; /* Signal to noise (and interference) ratio. */
    auto sinr = std::pow(10.0, sinr_db / 10.0); /* Signal to noise (and interference) ratio in power ratio. */
    auto bep = 0.5 * std::erfc(std::sqrt(sinr / 2.0));  /* Bit error probability. */
    return 1.0 - std::pow((1.0 - bep), packetsize * 8.0); /* Packet error probability. */
}
