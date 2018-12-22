#ifndef MANETSIMS_HELPERS_H
#define MANETSIMS_HELPERS_H

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>
#include <iterator>
#include <chrono>
#include <functional>
#include <spdlog/spdlog.h>

#include "ostr.h"

namespace mpilib {

#define LENGTH 8

    template<typename T>
    void log_packet(std::shared_ptr<spdlog::logger> &c, const char *prefix, std::vector<T> &packet) {
        for (auto i = 0; i < (packet.size() / LENGTH) + 1; ++i) {
            if (((LENGTH * i + LENGTH) > packet.size())) {
                c->debug("{}{}", prefix, std::vector<T>{packet.begin() + (LENGTH * i), packet.end()});
            } else {
                c->debug("{}{}", prefix,
                         std::vector<T>{packet.begin() + (LENGTH * i), packet.begin() + ((LENGTH * i) + LENGTH)});
            }
        }
    }

    template<typename T>
    void log_packet(std::shared_ptr<spdlog::logger> &c, const char *prefix, std::vector<T> *packet) {
        log_packet(c, prefix, *packet);
    }


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
    bool is_equal(T a, T b) {
        return fabs(a - b) < std::numeric_limits<T>::epsilon();
    }

    template<typename T>
    bool is_equal(T a, T b, T epsilon) {
        return fabs(a - b) < epsilon;
    }

    uint64_t generate_link_id(uint32_t id1, uint32_t id2);

    std::string processor_name(const char *processor_name, int world_rank);
}

#endif /* MANETSIMS_HELPERS_H */
