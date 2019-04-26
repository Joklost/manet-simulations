#include "coordinator.h"

#include <iostream>

int main(int argc, char *argv[]) {
    bool debug{};
    bool plots{};
    std::string log{};

    for (auto i = 1; i < argc; i++) {
        if (std::string{"--debug"} == std::string{argv[i]}) {
            debug = true;
        } else if (std::string{"--plots"} == std::string{argv[i]}) {
            plots = true;
        } else {
            log = std::string{argv[i]};
        }
    }

    if (log.empty()) {
        log = "coordinator/gridlog_4x4_rssi.txt";
    }

    Coordinator coordr{log.c_str(), debug, plots};
    coordr.run();

    return 0;
}