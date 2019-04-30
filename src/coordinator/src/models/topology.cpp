
#include <common/equality.h>

#include "topology.h"

double Topology::get_link(unsigned long rank1, unsigned long rank2) {
    auto rssi1 = this->links[rank1][rank2];
    auto rssi2 = this->links[rank2][rank1];

    if (common::is_zero(rssi1) || common::is_zero(rssi2)) {
        return 0.0;
    }

    return (rssi1 + rssi2) / 2.0;
}

