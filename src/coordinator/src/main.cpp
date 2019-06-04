#include "coordinator.h"

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

    auto start = std::chrono::high_resolution_clock::now();
    coordr.run();
    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    coordr.c->info("Elapsed time: {} ms", elapsed_time);

    return 0;
}