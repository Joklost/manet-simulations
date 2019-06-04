#ifndef COORDINATOR_RADIOMODEL_H
#define COORDINATOR_RADIOMODEL_H

#include <vector>


struct RadioModel {
    constexpr static const double THERMAL_NOISE = -119.66;
    constexpr static const double NOISE_FIGURE = 4.2;

    static double lin(double log);

    static double log(double lin);

    static double pep(double rssi, unsigned long packetsize, const std::vector<double> &interference);
};

#endif //COORDINATOR_RADIOMODEL_H
