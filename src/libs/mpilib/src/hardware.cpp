#include <mpilib/hardware.h>


void hardware::init(const mpilib::geo::Location &loc) {
    if (hardware::initialized) {
        return;
    }
    hardware::initialized = true;

    int name_len{};
    char name[MPI_MAX_PROCESSOR_NAME];
    mpi::init(&hardware::world_size, &hardware::world_rank, &name_len, name);
    processor_name = name + std::to_string(hardware::world_rank);

    /* Subtract ctlr from world size. */
    hardware::world_size = hardware::world_size - 1;

    /* Handshake */
    int number;
    mpi::recv(&number, CTLR, HANDSHAKE);
    if (mpi::send(number, CTLR, HANDSHAKE) != MPI_SUCCESS) {
        hardware::deinit();
        return;
    } else {
        hardware::set_location(loc);
    }
}

void hardware::deinit() {
    if (!hardware::initialized) {
        return;
    }

    mpi::send(hardware::world_rank, CTLR, DIE);

    mpi::deinit();

    hardware::initialized = false;
}

void hardware::sleep(unsigned long time) {
    mpi::send(time, CTLR, SLEEP);

    unsigned long new_time;
    mpi::recv(&new_time, CTLR, SLEEP_ACK);
}

bool hardware::set_location(const mpilib::geo::Location &loc) {
    if (!hardware::initialized) {
        return false;
    }

    auto buffer = mpilib::serialise<mpilib::geo::Location>(loc);
    return mpi::send(buffer, CTLR, LOCATION) == MPI_SUCCESS;
}

unsigned long hardware::get_id() {
    if (!hardware::initialized) {
        return 0;
    }
    return static_cast<unsigned long>(hardware::world_rank);
}

unsigned long hardware::get_world_size() {
    if (!hardware::initialized) {
        return 0;
    }

    return static_cast<unsigned long>(hardware::world_size);
}
