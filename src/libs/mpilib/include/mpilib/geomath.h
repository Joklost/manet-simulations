#ifndef MANETSIMS_GEOMATH_H
#define MANETSIMS_GEOMATH_H

#include "location.h"

#define EARTH_RADIUS_KM 6371.0
#define KM 1000.0
#define KM_PER_DEGREE 111.320

namespace mpilib {

    namespace geo {

        double deg2rad(double deg);

        Location deg2rad(const Location &pos);

        double rad2deg(double rad);

        Location rad2deg(const Location &pos);

        double distance_between(const Location &from, const Location &to);

        double bearing_between(const Location &from, const Location &to);

        double angle_between(const Location &origin, const Location &pos1, const Location &pos2);

        Location move_location(const Location &location, double distance /*kilometers */, double bearing /* degrees */);

    }

}


#endif /* MANETSIMS_GEOMATH_H */
