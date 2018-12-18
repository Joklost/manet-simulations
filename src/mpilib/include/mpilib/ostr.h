#ifndef MANETSIMS_OSTR_H
#define MANETSIMS_OSTR_H

#include <ostream>
#include <vector>
#include <iterator>
#include <mpilib/defines.h>

std::ostream &operator<<(std::ostream &os, std::vector<octet> buffer);

std::ostream &operator<<(std::ostream &os, std::vector<octet> *buffer);

template<typename T>
std::ostream &operator<<(std::ostream &os, std::vector<T> vec) {
    os << "{";
    if (vec.size() != 0) {
        std::copy(vec.begin(), vec.end() - 1, std::ostream_iterator<T>(os, ", "));
        os << vec.back();
    }
    os << "}";
    return os;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, vecvec<T> vec) {
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

template<typename T>
std::ostream &operator<<(std::ostream &os, std::pair<T, T> pair) {
    os << "{" << pair.first << ", " << pair.second << "}";

    return os;
}

#endif /* MANETSIMS_OSTR_H */
