#ifndef MANETSIMS_LMAC_PACKET_H
#define MANETSIMS_LMAC_PACKET_H

#include <mpilib/packet.h>

class LmacPacket : public Packet {
public:
    int message;
};

#endif /* MANETSIMS_LMAC_PACKET_H */
