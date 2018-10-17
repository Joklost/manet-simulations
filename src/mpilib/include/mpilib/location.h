#ifndef MANETSIMS_LOCATION_H
#define MANETSIMS_LOCATION_H

#include <ostream>
#include <cmath>

class Location {
public:
    int time{};

    double latitude{};
    double longitude{};

    bool operator==(const Location &rhs) const;

    bool operator!=(const Location &rhs) const;

    bool operator<(const Location &rhs) const;

    bool operator>(const Location &rhs) const;

    bool operator<=(const Location &rhs) const;

    bool operator>=(const Location &rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const Location &location);

    void move(int time, double distance /*kilometers */, double bearing /* degrees */);

};

#endif //MANETSIMS_LOCATION_H
