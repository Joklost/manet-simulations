#ifndef MANETSIMS_HARDWARE_IMPL_H
#define MANETSIMS_HARDWARE_IMPL_H

#include <vector>
#include <iostream>
#include "packethandler.h"

template<class P>
class HardwareImpl {
public:
    ~HardwareImpl() = default;

    void broadcast(P *packet) const;

    std::vector<P *> listen(int time) const;

    void sleep(int time) const;

private:
    PacketHandler &packetHandler = PacketHandler::instance();
};


template<class P>
void HardwareImpl<P>::broadcast(P *packet) const {
    this->packetHandler.add_packet((void *) packet);
}

template<class P>
std::vector<P *> HardwareImpl<P>::listen(const int time) const {
    std::vector<P *> packets{static_cast<P *>(this->packetHandler.get_packet())};
    return packets;
}

template<class P>
void HardwareImpl<P>::sleep(const int time) const {

}


#endif /* MANETSIMS_HARDWARE_IMPL_H */
