#ifndef GEO_LOCATION_H
#define GEO_LOCATION_H

#include <ostream>
#include <cmath>

namespace geo {

    class Location {
    public:
        Location();

        Location(double latitude, double longitude);

        Location(double time, double latitude, double longitude);

        bool operator==(const Location &rhs) const;

        bool operator!=(const Location &rhs) const;

        bool operator<(const Location &rhs) const;

        bool operator>(const Location &rhs) const;

        bool operator<=(const Location &rhs) const;

        bool operator>=(const Location &rhs) const;

        friend std::ostream &operator<<(std::ostream &os, const Location &location) {
            os << "Location{latitude: " << std::fixed << location.get_latitude() << ", longitude: "
               << std::fixed << location.get_longitude() << "}";
            return os;
        }

        void move(double time, double distance /*kilometers */, double bearing /* degrees */);

        double get_latitude() const;

        double get_longitude() const;

        double get_time() const;

        void set_time(double time);

    private:
        double time{};
        double latitude{};
        double longitude{};
    };

    Location random_location(const Location &upper_bound, const Location &lower_bound);

    Location square(const Location &corner, double diag /* kilometers */);

}


#endif //GEO_LOCATION_H
