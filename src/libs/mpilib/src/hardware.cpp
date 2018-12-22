#include <mpilib/hardware.h>


void hardware::init(const mpilib::geo::Location &loc, bool debug) {
    if (hardware::initialized) {
        return;
    }

    hardware::initialized = true;

    int name_len{};
    char name[MPI_MAX_PROCESSOR_NAME];
    mpi::init(&hardware::world_size, &hardware::world_rank, &name_len, name);
    hardware::processor_name = mpilib::processor_name(name, hardware::get_id());

    hardware::logger = spdlog::stdout_color_st(hardware::processor_name);
    if (debug) {
        hardware::logger->set_level(spdlog::level::debug);
    }

    hardware::logger->debug("init()");

    /* Subtract ctlr from world size. */
    hardware::world_size = hardware::world_size - 1;

    /* Handshake */
    auto magic = mpi::recv<int>(CTLR, HANDSHAKE);
    if (mpi::send(magic, CTLR, HANDSHAKE) == MPI_SUCCESS) {
        hardware::set_location(loc);
    } else {
        hardware::deinit();
        return;
    }
}

void hardware::deinit() {
    if (!hardware::initialized) {
        return;
    }

    hardware::logger->debug("deinit()");
    mpi::send(hardware::world_rank, CTLR, DIE);
    mpi::deinit();
    hardware::initialized = false;
}

void hardware::sleep(unsigned long duration) {
    hardware::logger->debug("sleep(duration={})", duration);
    mpi::send(hardware::localtime, CTLR, SLEEP);
    mpi::send(duration, CTLR, SLEEP_DURATION);

    auto new_time = mpi::recv<unsigned long>(CTLR, SLEEP_ACK);
    hardware::localtime = new_time;
}

bool hardware::set_location(const mpilib::geo::Location &loc) {
    if (!hardware::initialized) {
        return false;
    }

    hardware::logger->debug("set_location(loc={})", loc);

    auto buffer = mpilib::serialise<mpilib::geo::Location>(loc);
    return mpi::send(buffer, CTLR, SET_LOCATION) == MPI_SUCCESS;
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
