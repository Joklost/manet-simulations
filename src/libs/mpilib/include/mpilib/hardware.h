#ifndef MANETSIMS_HW_H
#define MANETSIMS_HW_H

#include <iostream>

#include <mpilib/mpi.h>

static unsigned long my_time{};
static bool initialized{};
static int world_size{};
static int world_rank{};
static std::string processor_name;


void init_hardware(const Location &loc);

void deinit_hardware();

unsigned long get_id();

unsigned long get_world_size();

bool set_location(const Location &loc);

template<typename T>
void tx(T &packet) {
    /* Step 1: Serialise packet. */
    /* Step 2: Send packet to ctlr. */
    auto buffer = serialise(packet);
    mpi_send(buffer, CTRL, TX_PKT);

    unsigned long new_time;
    mpi_recv(&new_time, CTRL, TX_PKT_ACK);
}

template<typename T>
std::vector<T> rx(unsigned long time) {
    /* Step 1: Message ctlr; I'm ready to receive. */
    /* Step 2: Receive messages from ctlr, if any. */
    /* Step 3: Deserialise messages, and return them. */
    std::vector<T> packets{};

    /* Let the ctlr know that we want to listen for a number of time slots. */
    mpi_send(time, CTRL, RX_PKT);

    /* Wait for ctlr to respond with number of packets. */
    unsigned long packet_count;
    mpi_recv(&packet_count, CTRL, RX_PKT_COUNT);

    for (auto i = 0; i < packet_count; ++i) {
        std::vector<octet> buffer(sizeof(T));
        mpi_recv(buffer, CTRL, RX_PKT);

        packets.emplace_back(deserialise<T>(buffer));
    }

    unsigned long new_time;
    mpi_recv(&new_time, CTRL, RX_PKT_ACK);

    return packets;
}

void sleep(unsigned long time);


#endif /* MANETSIMS_HW_H */
