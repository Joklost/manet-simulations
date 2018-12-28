#include "ctrlr.h"

#include <iostream>

int main(int argc, char *argv[]) {
    auto debug = argc > 1 && std::string{"--debug"} == std::string{argv[1]};

    static_assert(std::is_trivially_copyable<mpilib::Link>::value, "not a TriviallyCopyable type");


    Controller ctrl{debug};
    ctrl.run();

    return 0;
}