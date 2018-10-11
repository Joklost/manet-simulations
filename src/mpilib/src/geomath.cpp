#include <cmath>
#include <mpilib/geomath.h>

double deg2rad(const double deg) {
    return (deg * M_PI / 180);
}

double rad2deg(const double rad) {
    return (rad * 180 / M_PI);
}

double distance_between(const Location from, const Location to) {
    auto u = std::sin((deg2rad(to.latitude) - deg2rad(from.latitude)) / 2);
    auto v = std::sin((deg2rad(to.longitude) - deg2rad(from.longitude)) / 2);
    return 2.0 * EARTH_RADIUS_KM *
           std::asin(std::sqrt(u * u + std::cos(deg2rad(from.latitude)) * std::cos(deg2rad(to.latitude)) * v * v));
}

double bearing_between(const Location from, const Location to) {
    auto y = std::sin(deg2rad(to.longitude) - deg2rad(from.longitude)) * std::cos(deg2rad(to.latitude));
    auto x = std::cos(deg2rad(from.latitude)) * std::sin(deg2rad(to.latitude)) -
             std::sin(deg2rad(from.latitude)) * std::cos(deg2rad(to.latitude)) *
             std::cos(deg2rad(to.longitude) - deg2rad(from.longitude));
    auto brng = rad2deg(std::atan2(y, x));
    return ((int) brng + 180) % 360;
}

