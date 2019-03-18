
#include "model.h"

bool linkaiders::Action::is_within(const Action &action) const {
    return this->start >= action.start && this->end <= action.end;
}

bool linkaiders::Action::operator==(const linkaiders::Action &rhs) const {
    return type == rhs.type &&
           id == rhs.id &&
           chn == rhs.chn &&
           start == rhs.start;
}

bool linkaiders::Action::operator!=(const linkaiders::Action &rhs) const {
    return !(rhs == *this);
}

