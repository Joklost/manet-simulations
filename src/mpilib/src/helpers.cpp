#include <mpilib/helpers.h>
#include <cmath>


uint64_t generate_link_id(const uint32_t id1, const uint32_t id2) {
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