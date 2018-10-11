
#ifndef MANETSIMS_MPI_H
#define MANETSIMS_MPI_H

#include <future>
#include <iostream>
#include "packethandler.h"


bool running = false;
std::future<void> mpi_thread;

void mpi() {

    while (running) {
        auto packet = PacketHandler::instance().get_packet();

    }
}

void init() {
    running = true;
    mpi_thread = std::async(std::launch::async, mpi);
}

void deinit() {
    running = false;
    mpi_thread.get();
}

#endif /* MANETSIMS_MPI_H */
