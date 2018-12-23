#ifndef MANETSIMS_HW_H
#define MANETSIMS_HW_H

#include <iostream>

#include <mpilib/mpi.h>

#include "hwglobals.h"


namespace hardware {

    /**
     * Add execution time to our localtime.
     * @private
     */
    void prepare_localtime();

    /**
     * Gets the current execution time.
     * @return std::chrono::time_point
     * @private
     */
    std::chrono::time_point<std::chrono::high_resolution_clock> now();

    /**
     * Initialise the hardware interface.
     *
     * @param loc The initial geolocation of this node.
     * @param debug Enable debug logging.
     */
    void init(const mpilib::geo::Location &loc, bool debug = false);

    /**
     * Deinitialise the hardware interface.
     */
    void deinit();

    /**
     * Get the unique identifer assigned to this node.
     * @return unsigned long, identifier.
     */
    unsigned long get_id();

    /**
     * Get the size of the known world (the amount of nodes).
     * @return unsigned long, world size.
     */
    unsigned long get_world_size();

    /**
     * Set the geolocation of this node.
     * @param loc The new geolocation.
     * @return bool, true if successfull.
     */
    bool set_location(const mpilib::geo::Location &loc);

    /**
     * Report the local time of this node to the controller.
     *
     * This can be called as an alternative to #broadcast, #listen, or #sleep,
     * if none is applicable.
     */
    void report_localtime();

    /**
     * Sleep for the given duration.
     * @param duration Amount of microseconds to sleep for.
     */
    void sleep(std::chrono::microseconds duration);


    /**
     * Broadcast a packet.
     * @tparam T Type of the #packet. The type must be Trivially Copyable.
     * @param packet Object of type T containing the data that should be broadcast.
     * @return std::chrono::microseconds, amount of microseconds it took to broadcast the packet.
     */
    template<typename T>
    std::chrono::microseconds broadcast(T &packet) {
        if (!hardware::initialized) {
            return 0us;
        }

        /* Step 1: Serialise packet. */
        /* Step 2: Send packet to ctrlr. */
        /* Step 3: Wait for acknowledgement. */
        auto buffer = mpilib::serialise<T>(packet);
        auto duration = mpilib::transmission_time(BAUDRATE, buffer.size());

        hardware::prepare_localtime();
        hardware::logger->debug("broadcast(octets={}, localtime={}, duration={})",
                                buffer.size(), hardware::localtime, duration);

        mpi::send(static_cast<unsigned long>(hardware::localtime.count()), CTRLR, TX_PKT);
        mpi::send(static_cast<unsigned long>(duration.count()), CTRLR, TX_PKT_DURATION);
        mpi::send(buffer, CTRLR, TX_PKT_DATA);
        auto new_time = mpi::recv<unsigned long>(CTRLR, TX_PKT_ACK);

        hardware::localtime = std::chrono::microseconds{new_time};
        hardware::clock = hardware::now();

        return duration;
    }

    /**
     * Listen for packets for the given duration.
     * @tparam T Type of the packets returned. The type must be Trivially Copyable.
     * @param duration Amount of microseconds to listen for.
     * @return std::vector containing packets of type T
     */
    template<typename T>
    std::vector<T> listen(std::chrono::microseconds duration) {
        if (!hardware::initialized) {
            return std::vector<T>{};
        }

        /* Step 1: Message ctrlr; I'm ready to receive. */
        /* Step 2: Receive messages from ctrlr, if any. */
        /* Step 3: Deserialise messages, and return them. */

        hardware::prepare_localtime();
        hardware::logger->debug("listen(localtime={}, duration={})", hardware::localtime, duration);

        std::vector<T> packets{};

        /* Let the ctrlr know that we want to listen for a number of time slots. */
        mpi::send(static_cast<unsigned long>(hardware::localtime.count()), CTRLR, RX_PKT);
        mpi::send(static_cast<unsigned long>(duration.count()), CTRLR, RX_PKT_DURATION);

        /* Wait for ctrlr to respond with number of packets. */
        auto packet_count = mpi::recv<unsigned long>(CTRLR, RX_PKT_COUNT);

        for (auto i = 0; i < packet_count; ++i) {
            auto buffer = mpi::recv<std::vector<octet>>(CTRLR, RX_PKT_DATA);
            packets.emplace_back(mpilib::deserialise<T>(buffer));
        }

        auto new_time = mpi::recv<unsigned long>(CTRLR, RX_PKT_ACK);
        hardware::localtime = std::chrono::microseconds{new_time};
        hardware::clock = hardware::now();

        return packets;
    }

}

#endif /* MANETSIMS_HW_H */
