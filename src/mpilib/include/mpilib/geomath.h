#ifndef MANETSIMS_GEOMATH_H
#define MANETSIMS_GEOMATH_H

#define EARTH_RADIUS_KM 6371.0

struct Location {
    double latitude;
    double longitude;
};

double deg2rad(double deg);

Location deg2rad(Location pos);

double rad2deg(double rad);

Location rad2deg(Location pos);

double distance_between(Location from, Location to);

double bearing_between(Location from, Location to);

double angle_between(Location origin, Location pos1, Location pos2);


#endif /* MANETSIMS_GEOMATH_H */
