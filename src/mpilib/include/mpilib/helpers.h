
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
            /*std::copy((*it).cbegin(), (*it).cend() - 1, std::ostream_iterator<T>(os, ", "));
            os << (*it).back();*/
            os << "\n";
            //if (it != vec.cend()) {
            //}
        }
/*
        for (auto v : vec) {
            os << " {";

            std::copy(v.begin(), v.end() - 1, std::ostream_iterator<T>(os, ", "));
            os << v.back();

            os << " }\n";

        }*/
        //std::copy(vec.begin(), vec.end() - 1, std::ostream_iterator<T>(os, " "));
        //os << vec.back();
    }
    os << "}";
    return os;
}


#endif /* MANETSIMS_HELPERS_H */
