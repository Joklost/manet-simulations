#ifndef COMMON_EQUALITY_H
#define COMMON_EQUALITY_H

#include <cmath>
#include <limits>

namespace common {
    template<typename T>
    bool is_equal(T a, T b) {
        return std::fabs(a - b) < std::numeric_limits<T>::epsilon();
    }

    template<typename T>
    bool is_equal(T a, T b, T epsilon) {
        return std::fabs(a - b) < epsilon;
    }

    template<typename T>
    bool is_zero(T a) {
        return is_equal(a, (T) 0.0);
    }
}

#endif /* COMMON_EQUALITY_H */
