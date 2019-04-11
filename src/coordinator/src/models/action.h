#ifndef COORDINATOR_ACTION_H
#define COORDINATOR_ACTION_H

#include "packet.h"

enum Type {
    Listen, Transmission, Sleep, Inform
};

struct Action {
    Type type{};
    int rank{};
    unsigned long start{};
    unsigned long end{};
    Packet packet{};

    bool is_within(const Action &action) const;

    bool operator==(const Action &rhs) const;

    bool operator!=(const Action &rhs) const;
};

bool compare_actions(const Action &left, const Action &right);

#endif //COORDINATOR_ACTION_H
