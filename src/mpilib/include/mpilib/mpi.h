#ifndef MANETSIMS_SHARED_H
#define MANETSIMS_SHARED_H

#include <mpich/mpi.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/fmt/ostr.h"

#include "objectifier.h"

#define CONTROLLER 0
#define TX_PKT 1
#define RX_PKT 2
#define SLEEP 3
#define HANDSHAKE 10
#define DIE 11

using status_t = MPI_Status;


void mpi_init(int *world_size, int *world_rank, int *name_len, char *processor_name) {
    MPI_Init(nullptr, nullptr);

    MPI_Comm_size(MPI_COMM_WORLD, world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, world_rank);
    MPI_Get_processor_name(processor_name, name_len);
}

void mpi_deinit() {
    MPI_Finalize();
}


status_t mpi_probe_any() {
    status_t status{};
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    return status;
}


int mpi_send(const std::vector<octet> &buf, const int dest, const int tag) {
    return MPI_Send(&buf.front(), static_cast<int>(buf.size()), MPI_UNSIGNED_CHAR, dest, tag, MPI_COMM_WORLD);
}

status_t mpi_recv(std::vector<octet> &buf, int source, int tag) {
    status_t status{};
    MPI_Probe(source, tag, MPI_COMM_WORLD, &status);

    int count;
    MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &count);

    buf.resize(static_cast<unsigned long>(count));
    MPI_Recv(&buf.front(), count, MPI_UNSIGNED_CHAR, source, tag, MPI_COMM_WORLD, &status);
    return status;
}

int mpi_send(const std::vector<octet> *buf, const int dest, const int tag) {
    return mpi_send(*buf, dest, tag);
}

status_t mpi_recv(std::vector<octet> *buf, int source, int tag) {
    return mpi_recv(*buf, source, tag);
}

int mpi_send(const unsigned long buf, const int dest, const int tag) {
    return MPI_Send(&buf, 1, MPI_UNSIGNED_LONG, dest, tag, MPI_COMM_WORLD);
}

status_t mpi_recv(unsigned long *buf, int source, int tag) {
    status_t status{};
    MPI_Recv(buf, 1, MPI_UNSIGNED_LONG, source, tag, MPI_COMM_WORLD, &status);
    return status;
}

int mpi_send(const int buf, const int dest, const int tag) {
    return MPI_Send(&buf, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

status_t mpi_recv(int *buf, int source, int tag) {
    status_t status{};
    MPI_Recv(buf, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
    return status;
}


#endif /* MANETSIMS_SHARED_H */
