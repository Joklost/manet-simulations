
#ifndef MANETSIMS_HELPERS_H
#define MANETSIMS_HELPERS_H

#include <algorithm>
#include <iostream>
#include <vector>
#include <iterator>
#include <chrono>
#include <functional>

template<typename TimeT = std::chrono::milliseconds>
struct measure {
    template<typename F, typename ...Args>
    static typename TimeT::rep execution(const F &&func, Args &&... args) {
        auto start = std::chrono::steady_clock::now();
        std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
        auto duration = std::chrono::duration_cast<TimeT>(std::chrono::steady_clock::now() - start);
        return duration.count();
    }

    template<typename F, typename ...Args>
    static auto duration(const F &&func, Args &&... args) {
        auto start = std::chrono::steady_clock::now();
        std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
        return std::chrono::duration_cast<TimeT>(std::chrono::steady_clock::now() - start);
    }
};

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

template<class InputIt, class BinaryFunction>
void for_each_interval(const InputIt first, const InputIt last, size_t interval_size, const BinaryFunction &func) {
    auto to = first;

    while (to != last) {
        auto from = to;

        auto counter = interval_size;
        while (counter > 0 && to != last) {
            ++to;
            --counter;
        }

        func(from, to);
    }
}

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
bool compare_vectors(std::vector<T> a, std::vector<T> b, T epsilon) {
    if (a.size() != b.size()) return false;
    for (auto i = 0; i < a.size(); i++) {
        if (a[i] != Approx(b[i]).margin(epsilon)) {
            std::cout << a[i] << " Should == " << b[i] << std::endl;
            return false;
        }
    }
    return true;
}


template<typename T>
bool compare_vectors(vecvec<T> a, vecvec<T> b, T epsilon) {
    if (a.size() != b.size()) return false;
    for (auto i = 0; i < a.size(); i++) {
        if (!compare_vectors(a[i], b[i], epsilon)) return false;
    }
    return true;
}

uint64_t generate_link_id(const uint32_t id1, const uint32_t id2);

#endif /* MANETSIMS_HELPERS_H */
