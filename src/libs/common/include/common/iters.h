#ifndef COMMON_ITERS_H
#define COMMON_ITERS_H

#include <algorithm>
#include <iterator>

namespace common {
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
}

#endif /* COMMON_ITERS_H */
