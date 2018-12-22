#ifndef MANETSIMS_LOCATION_H
#define MANETSIMS_LOCATION_H

#include <ostream>
#include <cmath>

namespace mpilib {

    namespace geo {

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

            friend std::ostream &operator<<(std::ostream &os, const Location &location) {
                os << "{latitude: " << location.get_latitude() << ", longitude: "
                   << location.get_longitude() << "}";
                return os;
            }

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

        Location random_location(const Location &upper_bound, const Location &lower_bound);

        Location square(const Location &corner, double diag /* kilometers */);

    }

}

#endif //MANETSIMS_LOCATION_H
