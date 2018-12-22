#ifndef MANETSIMS_HWGLOBALS_H
#define MANETSIMS_HWGLOBALS_H

namespace hardware {

    extern unsigned long my_time;
    extern bool initialized;
    extern int world_size;
    extern int world_rank;
    extern std::string processor_name;

}



#endif /* MANETSIMS_HWGLOBALS_H */
