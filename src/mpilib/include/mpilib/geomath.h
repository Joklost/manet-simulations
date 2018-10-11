#ifndef MANETSIMS_GEOMATH_H
#define MANETSIMS_GEOMATH_H

#define EARTH_RADIUS_KM 6371.0

struct Location {
    double latitude;
    double longitude;
};

double deg2rad(double deg);

double rad2deg(double rad);

double distance_between(Location from, Location to);

double bearing_between(Location from, Location to);


#endif /* MANETSIMS_GEOMATH_H */
