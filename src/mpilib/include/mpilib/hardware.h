#ifndef MANETSIMS_HW_H
#define MANETSIMS_HW_H

#include <mpilib/mpi.h>

static bool initialized{};
static int world_size{};
static int world_rank{};
static std::string processor_name;


void init_hardware(const Location &loc);

void deinit_hardware();

bool set_location(const Location &loc);

template<typename T>
void tx(T &packet) {
    /* Step 1: Serialise packet. */
    /* Step 2: Send packet to controller. */
    auto buffer = serialise(packet);
    mpi_send(buffer, CTRL, TX_PKT);
}

template<typename T>
std::vector<T> rx(unsigned long time) {
    /* Step 1: Message controller; I'm ready to receive. */
    /* Step 2: Receive messages from controller, if any. */
    /* Step 3: Deserialise messages, and return them. */
    std::vector<T> packets{};

    mpi_send(time, CTRL, RX_PKT);

    unsigned long packet_count;
    mpi_recv(&packet_count, CTRL, RX_PKT);

    for (auto i = 0; i < packet_count; ++i) {
        std::vector<octet> buffer(sizeof(T));
        mpi_recv(buffer, CTRL, RX_PKT);

        T packet{};
        packets.emplace_back(deserialise<T>(buffer, packet));
    }

    return packets;
}

void sleep(unsigned long time);


#endif /* MANETSIMS_HW_H */
