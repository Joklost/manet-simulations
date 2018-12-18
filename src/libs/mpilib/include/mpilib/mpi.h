#ifndef MANETSIMS_MPI_H
#define MANETSIMS_MPI_H

#include <mpich/mpi.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>

#include <mpilib/objectifier.h>
#include <mpilib/ostr.h>
#include <mpilib/defines.h>
#include <mpilib/helpers.h>
#include <mpilib/location.h>

struct Status {
    int count;
    int source;
    int tag;
    int error;

    Status() = default;

    explicit Status(MPI_Status status) : count(status.count_lo),
                                         source(status.MPI_SOURCE),
                                         tag(status.MPI_TAG),
                                         error(status.MPI_ERROR) {}
};

void mpi_init(int *world_size, int *world_rank, int *name_len, char *processor_name);

void mpi_deinit();

Status mpi_probe_any();

Status mpi_probe(int source, int tag);

int mpi_send(const std::vector<octet> &buf, int dest, int tag);

Status mpi_recv(std::vector<octet> &buf, int source, int tag);

int mpi_send(const std::vector<octet> *buf, int dest, int tag);

Status mpi_recv(std::vector<octet> *buf, int source, int tag);

int mpi_send(unsigned long buf, int dest, int tag);

Status mpi_recv(unsigned long *buf, int source, int tag);

int mpi_send(int buf, int dest, int tag);

Status mpi_recv(int *buf, int source, int tag);


#endif /* MANETSIMS_MPI_H */
