#ifndef MANETSIMS_PACKETHANDLER_H
#define MANETSIMS_PACKETHANDLER_H

#include "queue.h"

class PacketHandler {
public:
    static PacketHandler &instance() {
        static PacketHandler instance;
        return instance;
    }

    PacketHandler(PacketHandler const &) = delete;

    void operator=(PacketHandler const &) = delete;

    void add_packet(void * packet) {
        packet_queue.push(packet);
    }

    void * get_packet() {
        return packet_queue.pop();
    }

private:
    PacketHandler() = default;

    Queue<void *> packet_queue;
};


#endif /* MANETSIMS_PACKETHANDLER_H */
