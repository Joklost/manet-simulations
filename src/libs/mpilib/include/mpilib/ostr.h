#ifndef MANETSIMS_OSTR_H
#define MANETSIMS_OSTR_H

#include <ostream>
#include <vector>
#include <iterator>
#include <mpilib/defines.h>

std::ostream &operator<<(std::ostream &os, std::vector<octet> buffer);

std::ostream &operator<<(std::ostream &os, std::vector<octet> *buffer);

template<typename T>
std::ostream &operator<<(std::ostream &os, std::pair<T, T> pair) {
    os << "{" << pair.first << ", " << pair.second << "}";

    return os;
}

#endif /* MANETSIMS_OSTR_H */
