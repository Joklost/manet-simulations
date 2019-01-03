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
    hardware::prepare_localtime(0us);
    mpi::send(static_cast<unsigned long>(hardware::localtime.count()), CTRLR, DIE);
    mpi::deinit();
    hardware::initialized = false;
}

std::chrono::microseconds hardware::broadcast(std::vector<octet> data) {
    if (!hardware::initialized) {
        return 0us;
    }

    auto duration = mpilib::transmission_time(BAUDRATE, data.size());
    hardware::prepare_localtime(duration);
    hardware::logger->debug("broadcast(octets={}, localtime={}, duration={})",
                            data.size(), hardware::localtime - duration, duration);
    mpi::send((hardware::localtime - duration).count(), CTRLR, TX_PKT);
    mpi::send(data, CTRLR, TX_PKT_DATA);

    hardware::clock = hardware::now();
    return duration;
}

std::vector<std::vector<octet>> hardware::listen(std::chrono::microseconds duration) {
    if (!hardware::initialized) {
        return std::vector<std::vector<octet>>();
    }

    hardware::prepare_localtime(duration);
    hardware::logger->debug("listen(localtime={}, duration={})", hardware::localtime - duration, duration);

    std::vector<std::vector<octet>> packets{};

    /* Let the ctrlr know that we want to listen for a number of time slots. */
    mpi::send((hardware::localtime - duration).count(), CTRLR, RX_PKT);
    mpi::send(duration.count(), CTRLR, RX_PKT_DURATION);

    /* Wait for ctrlr to respond with number of packets. */
    auto packet_count = mpi::recv<unsigned long>(CTRLR, RX_PKT_COUNT);
    for (auto i = 0; i < packet_count; ++i) {
        auto buffer = mpi::recv<std::vector<octet>>(CTRLR, RX_PKT_DATA);
        packets.emplace_back(buffer);
    }

    hardware::clock = hardware::now();
    return packets;
}

void hardware::sleep(std::chrono::microseconds duration) {
    if (!hardware::initialized) {
        return;
    }

    hardware::prepare_localtime(duration);
    hardware::logger->debug("sleep(localtime={}, duration={})", hardware::localtime - duration, duration);

    mpi::send(static_cast<unsigned long>(hardware::localtime.count()), CTRLR, SLEEP);
    mpi::send(static_cast<unsigned long>(duration.count()), CTRLR, SLEEP_DURATION);

    hardware::clock = hardware::now();
}

void hardware::report_localtime() {
    if (!hardware::initialized) {
        return;
    }

    hardware::prepare_localtime(0us);
    hardware::logger->debug("report_localtime(localtime={})", hardware::localtime);
    mpi::send(static_cast<unsigned long>(hardware::localtime.count()), CTRLR, SET_LOCAL_TIME);

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

void hardware::prepare_localtime(std::chrono::microseconds duration) {
    /* Add execution time since last action to our localtime. */
    auto now = hardware::now();
    hardware::localtime = std::chrono::duration_cast<std::chrono::microseconds>(
            (now - hardware::clock) + hardware::localtime + duration
    );
}