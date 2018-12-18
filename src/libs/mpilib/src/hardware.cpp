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
    mpi_recv(&number, CTRL, HANDSHAKE);
    if (mpi_send(number, CTRL, HANDSHAKE) != MPI_SUCCESS) {
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

    mpi_send(world_rank, CTRL, DIE);

    mpi_deinit();

    initialized = false;
}

void sleep(unsigned long time) {
    mpi_send(time, CTRL, SLEEP);

    unsigned long new_time;
    mpi_recv(&new_time, CTRL, SLEEP_ACK);
}

bool set_location(const Location &loc) {
    auto buffer = serialise(loc);
    return mpi_send(buffer, CTRL, LOCATION) == MPI_SUCCESS;
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
    return static_cast<unsigned long>(world_size - 1);
}
