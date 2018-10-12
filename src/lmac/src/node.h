#ifndef MANETSIMS_LMAC_NODE_H
#define MANETSIMS_LMAC_NODE_H

#include <mpilib/geomath.h>
#include <mpi/hardware.h>
#include <thread>

#include "packet.h"

enum State {
    initialization, wait, discover, active
};


class LmacNode {
public:
    LmacNode(int id, Hardware<LmacPacket> hardware, Location location, bool gateway);

    void start();

    void join();

private:
    int id;
    State state;
    Location location;
    bool gateway;
    Hardware<LmacPacket> hardware;
    std::thread protocol_thread;

    void protocol();
};


#endif //MANETSIMS_LMAC_NODE_H
