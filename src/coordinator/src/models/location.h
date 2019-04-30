#ifndef COORDINATOR_LOCATION_H
#define COORDINATOR_LOCATION_H

#include <geo/location.h>

struct Location : public geo::Location {
    Location() = default;
    Location(double time, double latitude, double longitude) : geo::Location(time, latitude, longitude) {}
    std::unordered_map<unsigned long, double> links{};
};
#endif /* COORDINATOR_LOCATION_H */
