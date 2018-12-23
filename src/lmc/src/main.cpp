#include "lmc.h"


int main(int argc, char *argv[]) {
    auto debug = argc > 1 && std::string{"--debug"} == std::string{argv[1]};

    LinkModelComputer lmc{debug};
    lmc.run();

    return 0;
}