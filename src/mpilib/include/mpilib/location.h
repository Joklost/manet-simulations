#ifndef MANETSIMS_LOCATION_H
#define MANETSIMS_LOCATION_H

#include <ostream>
#include <cmath>

class Location {
public:
    Location();

    Location(double latitude, double longitude);

    Location(int time, double latitude, double longitude);

    bool operator==(const Location &rhs) const;

    bool operator!=(const Location &rhs) const;

    bool operator<(const Location &rhs) const;

    bool operator>(const Location &rhs) const;

    bool operator<=(const Location &rhs) const;

    bool operator>=(const Location &rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const Location &location);

    void move(int time, double distance /*kilometers */, double bearing /* degrees */);

    double get_latitude() const;

    double get_longitude() const;

    int get_time() const;

    void set_time(int time);

private:
    int time{};
    double latitude{};
    double longitude{};
};

#endif //MANETSIMS_LOCATION_H
