
#include "mpi/network.h"
#include "mpi.h"

void init_mpi() {
    if (running) {
        return;
    }

    init();
}

void deinit_mpi() {
    if (!running) {
        return;
    }

    deinit();
}

int register_node(double latitude, double longitude) {
    return 0;
}

