#ifndef MANETSIMS_OBJECTIFIER_H
#define MANETSIMS_OBJECTIFIER_H

#include <memory>
#include <cstdint>

#include "mpi.h"

using octet = unsigned char;

template<typename T>
using octetarray = std::array<octet, sizeof(T)>;

template<typename T>
std::string hex(const octetarray<T> bytes) {
    return hex(bytes.data(), bytes.size());
}

template<typename T>
octetarray<T> serialise(const T &object) {
    octetarray<T> bytes;

    const auto *begin = reinterpret_cast< const octet * >( std::addressof(object));
    const auto *end = begin + sizeof(T);
    std::copy(begin, end, std::begin(bytes));

    return bytes;
}

template<typename T>
T &deserialise(const octetarray<T> &bytes, T &object) {
    /* http://en.cppreference.com/w/cpp/types/is_trivially_copyable */
    static_assert(std::is_trivially_copyable<T>::value, "not a TriviallyCopyable type");

    auto *begin_object = reinterpret_cast< octet * >( std::addressof(object));
    std::copy(std::begin(bytes), std::end(bytes), begin_object);

    return object;
}


static constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


std::ostream &operator<<(std::ostream &os, std::vector<octet> &buffer) {
    for (const auto &item : buffer) {
        os << " 0x" << hexmap[(item & 0xF0) >> 4] << hexmap[item & 0x0F];
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, std::vector<octet> *buffer) {
    return os << *buffer;
}


#endif /* MANETSIMS_OBJECTIFIER_H */
