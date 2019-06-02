#include "action.h"

bool Action::is_within(const Action &action) const {
    return action.start <= this->start && this->end <= action.end;
}

bool Action::operator==(const Action &rhs) const {
    return type == rhs.type &&
           rank == rhs.rank &&
           start == rhs.start &&
           end == rhs.end &&
           packet == rhs.packet;
}

bool Action::operator!=(const Action &rhs) const {
    return !(rhs == *this);
}

std::ostream &operator<<(std::ostream &os, const Action &action) {
    os
            << "Action{"
            << "rank: " << action.rank
            << ", type: " << to_string(action.type)
            << ", start: " << action.start
            << ", end: " << action.end
            << ", packet: " << !action.packet.data.empty()
            << "}";
    return os;
}

bool compare_actions(const Action &left, const Action &right) {
    //return left.end < right.end;

    if (left.end == right.end) {
        if (left.type == right.type) {
            return left.rank < right.rank;
        }
        return left.type < right.type;
    }
    return left.end < right.end;
    //return left.end == right.end ? left.type < right.type : left.end < right.end;
}

std::string to_string(Type type) {
    switch (type) {
        case Inform:
            return std::string{"inform"};
        case Sleep:
            return std::string{"sleep"};
        case Transmission:
            return std::string{"transmit"};
        case Listen:
            return std::string{"listen"};
        default:
            return std::string{};
    }

}
