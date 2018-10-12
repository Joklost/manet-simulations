
#include <linkmodel/math.h>

#include "linkmodel/math.h"

double distance_pathloss(const double distance) {
    return (-10 * PATHLOSS_EXPONENT) * std::log10(distance) + PATHLOSS_CONSTANT_OFFSET;
}

double distance_pathloss(const Location to, const Location from) {
    return distance_pathloss(distance_between(to, from) * KM);
}

double distance_pathloss(Link link) {
    return distance_pathloss(distance_between(link.node1.location, link.node2.location) * KM);
}

double autocorrelation(const double angle) {
    /* TODO: #define the constants */
    return 0.595 * std::exp(-0.064 * angle) + 0.092;
}

double autocorrelation(const Location to, const Location from) {
    /* TODO: Implement using angle_between */
    return 0;
}

double autocorrelation(Link link) {
    /* TODO: Implement using angle_between */
    return 0;
}
