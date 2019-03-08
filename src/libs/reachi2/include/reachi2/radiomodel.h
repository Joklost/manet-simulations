#ifndef MANETSIMS_RADIOMODEL_H
#define MANETSIMS_RADIOMODEL_H

#include <vector>

namespace reachi2 {
    namespace radiomodel {
        double linearize(double logarithmic_value);
        double logarithmicize(double linear_value);

        /**
         * Compute the probability for packet loss given an RSSI in dBm, a packet size in bytes,
         * and a vector of interfering transmitters in RSSI, also in dBm.
         * @param rssi Received Signal Strength Indication in dBm.
         * @param packetsize Size of the packet in bytes.
         * @param interference std::vector containing RSSI from interfering transmitters.
         * @return Probability of packet loss.
         */
        double pep(double rssi, unsigned long packetsize, std::vector<double> interference = std::vector<double>{});
    }
}

#endif //MANETSIMS_RADIOMODEL_H
