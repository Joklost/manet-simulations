#ifndef MANETSIMS_LOCATION_H
#define MANETSIMS_LOCATION_H

#include <ostream>

class Location {
public:
    int time = 0;

    double latitude;
    double longitude;

    bool operator==(const Location &rhs) const {
        return latitude == rhs.latitude &&
               longitude == rhs.longitude;
    }

    bool operator!=(const Location &rhs) const {
        return !(rhs == *this);
    }

    bool operator<(const Location &rhs) const {
        if (latitude < rhs.latitude)
            return true;
        if (rhs.latitude < latitude)
            return false;
        return longitude < rhs.longitude;
    }

    bool operator>(const Location &rhs) const {
        return rhs < *this;
    }

    bool operator<=(const Location &rhs) const {
        return !(rhs < *this);
    }

    bool operator>=(const Location &rhs) const {
        return !(*this < rhs);
    }

    friend std::ostream &operator<<(std::ostream &os, const Location &location) {
        os << "latitude: " << location.latitude << " longitude: " << location.longitude;
        return os;
    }
};

#endif //MANETSIMS_LOCATION_H
