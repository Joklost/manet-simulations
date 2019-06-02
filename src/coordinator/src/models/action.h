#ifndef COORDINATOR_ACTION_H
#define COORDINATOR_ACTION_H

#include <ostream>
#include "packet.h"

enum Type {
    /* Ordering is important. */
    Inform = 0,
    Sleep = 1,
    Transmission = 2,
    Listen = 3
};

std::string to_string(Type type);

struct Action {
    Type type{};
    int rank{};
    unsigned long start{};
    unsigned long end{};
    Packet packet{};

    bool is_within(const Action &action) const;

    bool operator==(const Action &rhs) const;

    bool operator!=(const Action &rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const Action &action);
};

bool compare_actions(const Action &left, const Action &right);

#endif //COORDINATOR_ACTION_H
