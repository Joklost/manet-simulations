#include <mpilib/helpers.h>
#include <cmath>


std::string mpilib::processor_name(const char *processor_name, int world_rank) {
    auto sid = std::string(4 - std::to_string(world_rank).length(), '0') + std::to_string(world_rank);
    return std::string{processor_name} + sid;
}

std::chrono::microseconds mpilib::compute_transmission_time(unsigned long baudrate, unsigned long octets) {
    return std::chrono::microseconds(static_cast<unsigned long>((1.0 / baudrate) * (octets * 8) * 1000 * 1000));
}

