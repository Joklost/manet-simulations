#include "ctrl.h"

#include <iostream>

int main(int argc, char *argv[]) {
    auto debug = argc > 1 && std::string{"--debug"} == std::string{argv[1]};

    std::cout << argv[1] << std::endl;

    Controller ctrl{debug};
    ctrl.run();

    return 0;
}