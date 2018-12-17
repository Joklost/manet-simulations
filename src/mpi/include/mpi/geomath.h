#ifndef MANETSIMS_GEOMATH_H
#define MANETSIMS_GEOMATH_H

#include "node.h"

#define EARTH_RADIUS_KM 6371.0

double deg2rad(double deg);

Location deg2rad(Location &pos);

double rad2deg(double rad);

Location rad2deg(Location &pos);

double distance_between(const Node &from, const Node &to);

double distance_between(const Location &from, const Location &to);

double bearing_between(const Node &from, const Node &to);

double bearing_between(const Location &from, const Location &to);

double angle_between(const Location &origin, const Location &pos1, const Location &pos2);

double angle_between(const Node &origin, const Node &node1, const Node &node2);

Location move_location(const Location &location, double distance /*kilometers */, double bearing /* degrees */);


#endif /* MANETSIMS_GEOMATH_H */
