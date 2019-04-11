#ifndef COORDINATOR_TOPOLOGY_H
#define COORDINATOR_TOPOLOGY_H

#include "link.h"

struct Topology {
    double timestamp{};
    std::vector<Link> links{};
};

#endif //COORDINATOR_TOPOLOGY_H
