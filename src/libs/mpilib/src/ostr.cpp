
#include <mpilib/ostr.h>

static constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

std::ostream &operator<<(std::ostream &os, std::vector<octet> buffer) {
    for (const auto &item : buffer) {
        os << " 0x" << hexmap[(item & 0xF0) >> 4] << hexmap[item & 0x0F];
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, std::vector<octet> *buffer) {
    return os << *buffer;
}

