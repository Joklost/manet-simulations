#ifndef MANETSIMS_HW_H
#define MANETSIMS_HW_H

#include <iostream>

#include <mpilib/mpi.h>

#include "hwglobals.h"

namespace hardware {

    void init(const mpilib::geo::Location &loc);

    void deinit();

    unsigned long get_id();

    unsigned long get_world_size();

    bool set_location(const mpilib::geo::Location &loc);

    void sleep(unsigned long time);


    template<typename T>
    void broadcast(T &packet) {
        /* Step 1: Serialise packet. */
        /* Step 2: Send packet to ctlr. */
        auto buffer = mpilib::serialise(packet);
        mpi::send(buffer, CTLR, TX_PKT);

        unsigned long new_time;
        mpi::recv(&new_time, CTLR, TX_PKT_ACK);
    }

    template<typename T>
    std::vector<T> listen(unsigned long time) {
        /* Step 1: Message ctlr; I'm ready to receive. */
        /* Step 2: Receive messages from ctlr, if any. */
        /* Step 3: Deserialise messages, and return them. */
        std::vector<T> packets{};

        /* Let the ctlr know that we want to listen for a number of time slots. */
        mpi::send(time, CTLR, RX_PKT);

        /* Wait for ctlr to respond with number of packets. */
        unsigned long packet_count;
        mpi::recv(&packet_count, CTLR, RX_PKT_COUNT);

        for (auto i = 0; i < packet_count; ++i) {
            std::vector<octet> buffer(sizeof(T));
            mpi::recv(buffer, CTLR, RX_PKT);

            packets.emplace_back(mpilib::deserialise<T>(buffer));
        }

        unsigned long new_time;
        mpi::recv(&new_time, CTLR, RX_PKT_ACK);

        return packets;
    }
}

#endif /* MANETSIMS_HW_H */
