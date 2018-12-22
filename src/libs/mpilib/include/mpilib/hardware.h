#ifndef MANETSIMS_HW_H
#define MANETSIMS_HW_H

#include <iostream>

#include <mpilib/mpi.h>

#include "hwglobals.h"

namespace hardware {

    void init(const mpilib::geo::Location &loc, bool debug = false);

    void deinit();

    unsigned long get_id();

    unsigned long get_world_size();

    bool set_location(const mpilib::geo::Location &loc);

    void sleep(unsigned long time);


    template<typename T>
    void broadcast(T &packet) {
        if (!hardware::initialized) {
            return;
        }

        /* Step 1: Serialise packet. */
        /* Step 2: Send packet to ctlr. */
        /* Step 3: Wait for acknowledgement. */
        auto buffer = mpilib::serialise<T>(packet);
        hardware::logger->debug("broadcast(octets={})", buffer.size());

        mpi::send(hardware::localtime, CTLR, TX_PKT);
        mpi::send(buffer, CTLR, TX_PKT_DATA);
        auto new_time = mpi::recv<unsigned long>(CTLR, TX_PKT_ACK);

        hardware::localtime = new_time;
    }

    template<typename T>
    std::vector<T> listen(unsigned long duration) {
        if (!hardware::initialized) {
            return std::vector<T>{};
        }

        /* Step 1: Message ctlr; I'm ready to receive. */
        /* Step 2: Receive messages from ctlr, if any. */
        /* Step 3: Deserialise messages, and return them. */
        hardware::logger->debug("listen(duration={})", duration);

        std::vector<T> packets{};

        /* Let the ctlr know that we want to listen for a number of time slots. */
        mpi::send(hardware::localtime, CTLR, RX_PKT);
        mpi::send(duration, CTLR, RX_PKT_DURATION);

        /* Wait for ctlr to respond with number of packets. */
        auto packet_count = mpi::recv<unsigned long>(CTLR, RX_PKT_COUNT);

        for (auto i = 0; i < packet_count; ++i) {
            auto buffer = mpi::recv<std::vector<octet>>(CTLR, RX_PKT_DATA);
            packets.emplace_back(mpilib::deserialise<T>(buffer));
        }

        auto new_time = mpi::recv<unsigned long>(CTLR, RX_PKT_ACK);
        hardware::localtime = new_time;

        return packets;
    }
}

#endif /* MANETSIMS_HW_H */
