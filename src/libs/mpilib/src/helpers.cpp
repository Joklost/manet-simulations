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

std::chrono::microseconds mpilib::compute_transmission_time(unsigned long baudrate, unsigned long octets) {
    return std::chrono::microseconds(static_cast<unsigned long>((1.0 / baudrate) * (octets * 8) * 1000 * 1000));
}

std::vector<std::string> mpilib::split(const std::string &string, const std::string &delim) {
    std::vector<std::string> tokens{};
    size_t prev{}, pos{};

    do {
        pos = string.find(delim, prev);
        if (pos == std::string::npos) {
            pos = string.length();
        }

        std::string token = string.substr(prev, pos - prev);
        if (!token.empty()) {
            tokens.push_back(token);
        }
        prev = pos + delim.length();
    } while (pos < string.length() && prev < string.length());
    return tokens;
}
