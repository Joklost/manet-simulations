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

using namespace std::literals::chrono_literals;

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


    std::string processor_name(const char *processor_name, int world_rank);

    /**
     * Compute the transmission time required for transmitting a packet of size @octets
     * @param baudrate The number of bits the radio can transmit per second
     * @param octets The number of octets to transmit
     * @return Time transmission will take, in microseconds
     */
    std::chrono::microseconds compute_transmission_time(unsigned long baudrate, unsigned long octets);
}

#endif /* MANETSIMS_HELPERS_H */
