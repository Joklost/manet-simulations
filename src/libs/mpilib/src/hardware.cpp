#include <mpilib/hardware.h>


void hardware::init(const mpilib::geo::Location &loc, bool debug) {
    if (hardware::initialized) {
        return;
    }

    hardware::initialized = true;

    int name_len{};
    char name[MPI_MAX_PROCESSOR_NAME];
    mpi::init(&hardware::world_size, &hardware::world_rank, &name_len, name);
    hardware::processor_name = mpilib::processor_name(name, hardware::world_rank);

    hardware::logger = spdlog::stdout_color_mt(hardware::processor_name);
    if (debug) {
        hardware::logger->set_level(spdlog::level::debug);
    }

    hardware::logger->debug("init()");

    /* Subtract ctrlr from world size. */
    hardware::world_size = hardware::world_size - 1;

    /* Handshake */
    auto magic = mpi::recv<int>(CTRLR, HANDSHAKE);
    if (mpi::send(magic, CTRLR, HANDSHAKE) == MPI_SUCCESS) {
        hardware::set_location(loc);

        mpi::recv<int>(CTRLR, READY);
        hardware::clock = hardware::now();
        hardware::localtime = 0us;
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
    mpi::send(static_cast<unsigned long>(hardware::localtime.count()), CTRLR, DIE);
    mpi::deinit();
    hardware::initialized = false;
}

void hardware::sleep(std::chrono::microseconds duration) {
    hardware::prepare_localtime();
    hardware::logger->debug("sleep(localtime={}, duration={})", hardware::localtime, duration);

    mpi::send(static_cast<unsigned long>(hardware::localtime.count()), CTRLR, SLEEP);
    mpi::send(static_cast<unsigned long>(duration.count()), CTRLR, SLEEP_DURATION);

    auto new_time = mpi::recv<unsigned long>(CTRLR, SLEEP_ACK);
    hardware::localtime = std::chrono::microseconds{new_time};
    hardware::clock = hardware::now();
}

bool hardware::set_location(const mpilib::geo::Location &loc) {
    if (!hardware::initialized) {
        return false;
    }

    hardware::logger->debug("set_location(loc={})", loc);

    auto buffer = mpilib::serialise<mpilib::geo::Location>(loc);
    return mpi::send(buffer, CTRLR, SET_LOCATION) == MPI_SUCCESS;
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

std::chrono::time_point<std::chrono::high_resolution_clock> hardware::now() {
    return std::chrono::high_resolution_clock::now();
}

void hardware::prepare_localtime() {
    /* Add execution time since last action to our localtime. */
    auto now = hardware::now();
    hardware::localtime = std::chrono::duration_cast<std::chrono::microseconds>(
            (now - hardware::clock) + hardware::localtime
    );
}

void hardware::report_localtime() {
    if (!hardware::initialized) {
        return;
    }

    hardware::prepare_localtime();
    hardware::logger->debug("report_localtime(localtime={})", hardware::localtime);
    mpi::send(static_cast<unsigned long>(hardware::localtime.count()), CTRLR, SET_LOCAL_TIME);
    hardware::clock = hardware::now();
}
