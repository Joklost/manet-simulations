#include "packet.h"

bool Packet::operator==(const Packet &rhs) const {
    return time == rhs.time &&
           data == rhs.data;
}

bool Packet::operator!=(const Packet &rhs) const {
    return !(rhs == *this);
}
