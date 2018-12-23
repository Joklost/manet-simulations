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

namespace mpi {

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

    void init(int *world_size, int *world_rank, int *name_len, char *processor_name);

    void deinit();

    Status probe(int source = MPI_ANY_SOURCE, int tag = MPI_ANY_TAG);

    int send(const std::vector<octet> &buf, int dest, int tag);

    Status recv(std::vector<octet> &buf, int source, int tag);

    int send(const std::vector<octet> *buf, int dest, int tag);

    Status recv(std::vector<octet> *buf, int source, int tag);

    int send(unsigned long buf, int dest, int tag);

    Status recv(unsigned long *buf, int source, int tag);

    int send(long buf, int dest, int tag);

    Status recv(long *buf, int source, int tag);

    int send(int buf, int dest, int tag);

    Status recv(int *buf, int source, int tag);

    template <typename T>
    T recv(int source, int tag) {
        T buf{};
        mpi::recv(&buf, source, tag);
        return buf;
    }
}

#endif /* MANETSIMS_MPI_H */
