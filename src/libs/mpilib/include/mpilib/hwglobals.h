#ifndef MANETSIMS_HWGLOBALS_H
#define MANETSIMS_HWGLOBALS_H

#include <spdlog/spdlog.h>
#include <chrono>

using namespace std::chrono_literals;

#ifndef BAUDRATE
#define BAUDRATE 34800
#endif

namespace hardware {
    extern std::chrono::time_point<std::chrono::high_resolution_clock> clock;
    extern std::chrono::microseconds localtime;

    extern std::shared_ptr<spdlog::logger> logger;
    extern bool initialized;
    extern int world_size;
    extern int world_rank;
    extern std::string processor_name;

}



#endif /* MANETSIMS_HWGLOBALS_H */
