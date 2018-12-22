#ifndef MANETSIMS_HWGLOBALS_H
#define MANETSIMS_HWGLOBALS_H

#include <spdlog/spdlog.h>

namespace hardware {
    extern std::shared_ptr<spdlog::logger> logger;
    extern unsigned long localtime;
    extern bool initialized;
    extern int world_size;
    extern int world_rank;
    extern std::string processor_name;

}



#endif /* MANETSIMS_HWGLOBALS_H */
