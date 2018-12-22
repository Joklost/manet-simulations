#include <mpilib/mpi.h>


void mpi::init(int *world_size, int *world_rank, int *name_len, char *processor_name) {
    MPI_Init(nullptr, nullptr);

    MPI_Comm_size(MPI_COMM_WORLD, world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, world_rank);
    MPI_Get_processor_name(processor_name, name_len);
}

void mpi::deinit() {
    MPI_Finalize();
}

mpi::Status mpi::probe(int source, int tag) {
    MPI_Status status{};
    MPI_Probe(source, tag, MPI_COMM_WORLD, &status);
    return mpi::Status{status};
}

int mpi::send(const std::vector<octet> &buf, int dest, int tag) {
    return MPI_Send(&buf.front(), static_cast<int>(buf.size()), MPI_UNSIGNED_CHAR, dest, tag, MPI_COMM_WORLD);
}

mpi::Status mpi::recv(std::vector<octet> &buf, int source, int tag) {
    MPI_Status status{};
    MPI_Probe(source, tag, MPI_COMM_WORLD, &status);

    int count;
    MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &count);

    buf.resize(static_cast<unsigned long>(count));
    MPI_Recv(&buf.front(), count, MPI_UNSIGNED_CHAR, source, tag, MPI_COMM_WORLD, &status);
    return mpi::Status{status};
}

int mpi::send(const std::vector<octet> *buf, int dest, int tag) {
    return send(*buf, dest, tag);
}

mpi::Status mpi::recv(std::vector<octet> *buf, int source, int tag) {
    return recv(*buf, source, tag);
}

int mpi::send(const unsigned long buf, const int dest, const int tag) {
    return MPI_Send(&buf, 1, MPI_UNSIGNED_LONG, dest, tag, MPI_COMM_WORLD);
}

mpi::Status mpi::recv(unsigned long *buf, int source, int tag) {
    MPI_Status status{};
    MPI_Recv(buf, 1, MPI_UNSIGNED_LONG, source, tag, MPI_COMM_WORLD, &status);
    return mpi::Status{status};
}

int mpi::send(const int buf, const int dest, const int tag) {
    return MPI_Send(&buf, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

mpi::Status mpi::recv(int *buf, int source, int tag) {
    MPI_Status status{};
    MPI_Recv(buf, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
    return mpi::Status{status};
}