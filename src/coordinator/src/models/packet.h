#ifndef COORDINATOR_PACKET_H
#define COORDINATOR_PACKET_H

#include <mpilib/defines.h>
#include <vector>

struct Packet {
    unsigned long time{0ul};
    std::vector<octet> data{};

    bool operator==(const Packet &rhs) const;

    bool operator!=(const Packet &rhs) const;
};

#endif //COORDINATOR_PACKET_H
