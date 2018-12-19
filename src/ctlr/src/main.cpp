#include "ctlr.h"

#include <iostream>

int main(int argc, char *argv[]) {
    auto debug = argc > 1 && std::string{"--debug"} == std::string{argv[1]};

    Controller ctrl{debug};
    ctrl.run();

    return 0;
}