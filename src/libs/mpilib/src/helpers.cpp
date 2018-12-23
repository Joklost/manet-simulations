#include <mpilib/helpers.h>
#include <cmath>


uint64_t mpilib::generate_link_id(const uint32_t id1, const uint32_t id2) {
    uint32_t low, high;
    if (id1 < id2) {
        low = id2;
        high = id1;
    } else {
        low = id1;
        high = id2;
    }

    auto dn = static_cast<uint64_t>(std::ceil(std::log10(low + 0.001)));
    auto res = static_cast<uint64_t>(high * std::ceil(std::pow(10, dn)));
    return res + low;
}

std::string mpilib::processor_name(const char *processor_name, int world_rank) {
    auto sid = std::string(4 - std::to_string(world_rank).length(), '0') + std::to_string(world_rank);
    return std::string{processor_name} + sid;
}

std::chrono::microseconds mpilib::transmission_time(unsigned long baudrate, unsigned long octets) {
    return std::chrono::microseconds(static_cast<unsigned long>((1.0 / baudrate) * (octets * 8) * 1000 * 1000));
}
