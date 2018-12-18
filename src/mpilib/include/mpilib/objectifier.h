#ifndef MANETSIMS_OBJECTIFIER_H
#define MANETSIMS_OBJECTIFIER_H

#include <memory>
#include <vector>
#include <mpilib/defines.h>

template<typename T>
std::vector<octet> serialise(const T &object) {
    std::vector<octet> octets(sizeof(T));

    const auto *begin = reinterpret_cast<const octet *>(std::addressof(object));
    const auto *end = begin + sizeof(T);
    std::copy(begin, end, std::begin(octets));

    return octets;
}

template<typename T>
T &deserialise(const std::vector<octet> &bytes, T &object) {
    /* http://en.cppreference.com/w/cpp/types/is_trivially_copyable */
    static_assert(std::is_trivially_copyable<T>::value, "not a TriviallyCopyable type");

    auto *begin_object = reinterpret_cast<octet *>( std::addressof(object));
    std::copy(std::begin(bytes), std::end(bytes), begin_object);

    return object;
}

template<typename T>
T deserialise(const std::vector<octet> &bytes) {
    /* http://en.cppreference.com/w/cpp/types/is_trivially_copyable */
    static_assert(std::is_trivially_copyable<T>::value, "not a TriviallyCopyable type");

    T object{};
    auto *begin_object = reinterpret_cast<octet *>( std::addressof(object));
    std::copy(std::begin(bytes), std::end(bytes), begin_object);

    return object;
}



#endif /* MANETSIMS_OBJECTIFIER_H */
