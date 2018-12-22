#ifndef MANETSIMS_REACHI_OSTR_H
#define MANETSIMS_REACHI_OSTR_H

#include <ostream>

#include <mpilib/ostr.h>

#include "node.h"
#include "linalg.h"

template<typename T>
std::ostream &operator<<(std::ostream &os, reachi::linalg::vec<T> vec) {
    os << "{";
    if (vec.size() != 0) {
        std::copy(vec.begin(), vec.end() - 1, std::ostream_iterator<T>(os, ", "));
        os << vec.back();
    }
    os << "}";
    return os;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, reachi::linalg::vecvec<T> vec) {
    os << "{\n";
    if (vec.size() != 0) {
        for (auto it = vec.cbegin(); it != vec.cend(); ++it) {
            os << (*it);
            os << "\n";
        }

    }
    os << "}";
    return os;
}

#endif /* MANETSIMS_REACHI_OSTR_H */
