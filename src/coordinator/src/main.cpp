#include "coordinator.h"

#include <iostream>

int main(int argc, char *argv[]) {
    auto debug = argc > 1 && std::string{"--debug"} == std::string{argv[1]};

    Coordinator coordr{debug};
    coordr.run();

    return 0;
}