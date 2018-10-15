
#ifndef MANETSIMS_HELPERS_H
#define MANETSIMS_HELPERS_H

#include <algorithm>
#include <iostream>
#include <vector>
#include <iterator>

template<typename T>
using vecvec = std::vector<std::vector<T>>;

/**
 * Wrapper for the std::for_each
 * function, that iterates a counter and invokes
 * the included binary function, accepting the
 * counter and the element from the iterable.
 * @tparam InputIt An iterable
 * @tparam BinaryFunction A function taking two parameters
 * @param first The beginning of the iterator
 * @param last The end of the iterator
 * @param start Where to start the counter
 * @param func The function called with each element
 */
template<class InputIt, class BinaryFunction>
void enumerate(const InputIt first, const InputIt last, int start, const BinaryFunction &func) {
    auto counter = start;
    std::for_each(first, last, [&counter, &func](auto element) {
        func(counter, element);
        counter++;
    });
};

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


template<typename T>
bool compare_vectors(std::vector<T> a, std::vector<T> b, T margin) {
    if (a.size() != b.size()) return false;
    for (auto i = 0; i < a.size(); i++) {
        if (a[i] != Approx(b[i]).margin(margin)) {
            std::cout << a[i] << " Should == " << b[i] << std::endl;
            return false;
        }
    }
    return true;
}


template<typename T>
bool compare_vecvecs(vecvec<T> a, vecvec<T> b, T margin) {
    if (a.size() != b.size()) return false;
    for (auto i = 0; i < a.size(); i++) {
        if (!compare_vectors(a[i], b[i], margin)) return false;
    }
    return true;
}

#endif /* MANETSIMS_HELPERS_H */
