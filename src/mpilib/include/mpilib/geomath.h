#ifndef MANETSIMS_GEOMATH_H
#define MANETSIMS_GEOMATH_H

#include "node.h"

#define EARTH_RADIUS_KM 6371.0

double deg2rad(double deg);

Location deg2rad(Location pos);

double rad2deg(double rad);

Location rad2deg(Location pos);

double distance_between(Location from, Location to);

double bearing_between(Location from, Location to);

double angle_between(Location origin, Location pos1, Location pos2);

double angle_between(Node origin, Node node1, Node node2);

Location move_location(const Location &location, double distance /*kilometers */, double bearing /* degrees */);


#endif /* MANETSIMS_GEOMATH_H */
