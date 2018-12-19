#include <mpilib/hardware.h>


void init_hardware(const Location &loc) {
    if (initialized) {
        return;
    }
    initialized = true;

    int name_len{};
    char name[MPI_MAX_PROCESSOR_NAME];
    mpi_init(&world_size, &world_rank, &name_len, name);
    processor_name = name + std::to_string(world_rank);

    /* Subtract ctlr from world size. */
    world_size = world_size - 1;

    /* Handshake */
    int number;
    mpi_recv(&number, CTLR, HANDSHAKE);
    if (mpi_send(number, CTLR, HANDSHAKE) != MPI_SUCCESS) {
        deinit_hardware();
        return;
    } else {
        set_location(loc);
    }
}

void deinit_hardware() {
    if (!initialized) {
        return;
    }

    mpi_send(world_rank, CTLR, DIE);

    mpi_deinit();

    initialized = false;
}

void sleep(unsigned long time) {
    mpi_send(time, CTLR, SLEEP);

    unsigned long new_time;
    mpi_recv(&new_time, CTLR, SLEEP_ACK);
}

bool set_location(const Location &loc) {
    if (!initialized) {
        return false;
    }

    auto buffer = serialise(loc);
    return mpi_send(buffer, CTLR, LOCATION) == MPI_SUCCESS;
}

unsigned long get_id() {
    if (!initialized) {
        return 0;
    }
    return static_cast<unsigned long>(world_rank);
}

unsigned long get_world_size() {
    if (!initialized) {
        return 0;
    }

    mpi_send(world_rank, CTLR, WORLD_SIZE_REQ);

    unsigned long size;
    mpi_recv(&size, CTLR, WORLD_SIZE_RSP);

    return size;
}

unsigned long get_local_time(unsigned long id) {
    if (!initialized) {
        return 0;
    }

    mpi_send(id, CTLR, LOCAL_TIME_REQ);

    unsigned long local_time;
    mpi_recv(&local_time, CTLR, LOCAL_TIME_RSP);

    return local_time;
}
