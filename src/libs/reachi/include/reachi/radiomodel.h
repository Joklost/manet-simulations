#ifndef MANETSIMS_RADIOMODEL_H
#define MANETSIMS_RADIOMODEL_H

#include <cmath>

namespace reachi {
    namespace radiomodel {

        double packet_error_probability(double rssi, int packetsize);

    }
}


#endif //MANETSIMS_RADIOMODEL_H
