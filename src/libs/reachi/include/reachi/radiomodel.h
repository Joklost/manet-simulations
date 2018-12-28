#ifndef MANETSIMS_RADIOMODEL_H
#define MANETSIMS_RADIOMODEL_H

#include <cmath>

namespace reachi {
    namespace radiomodel {

        double pep(double rssi, unsigned long packetsize, double interference = 0.0);

    }
}

#endif //MANETSIMS_RADIOMODEL_H
