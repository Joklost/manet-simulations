#include <cmath>
#include <mpilib/geomath.h>
#include <vector>
#include <iterator>
#include <algorithm>

double deg2rad(const double deg) {
    return (deg * M_PI / 180);
}

Location deg2rad(const Location pos) {
    auto lat = deg2rad(pos.latitude);
    auto lon = deg2rad(pos.longitude);
    return Location{pos.time, lat, lon};
}

double rad2deg(const double rad) {
    return (rad * 180 / M_PI);
}

Location rad2deg(const Location pos) {
    auto lat = rad2deg(pos.latitude);
    auto lon = rad2deg(pos.longitude);
    return Location{pos.time, lat, lon};
}


double distance_between(const Node &from, const Node &to) {
    return distance_between(from.get_location(), to.get_location());
}

double distance_between(const Location &from, const Location &to) {
    auto u = std::sin((deg2rad(to.latitude) - deg2rad(from.latitude)) / 2);
    auto v = std::sin((deg2rad(to.longitude) - deg2rad(from.longitude)) / 2);
    return 2.0 * EARTH_RADIUS_KM *
           std::asin(std::sqrt(u * u + std::cos(deg2rad(from.latitude)) * std::cos(deg2rad(to.latitude)) * v * v));
}

double bearing_between(const Node &from, const Node &to) {
    return bearing_between(from.get_location(), to.get_location());
}

double bearing_between(const Location &from, const Location &to) {
    auto y = std::sin(deg2rad(to.longitude) - deg2rad(from.longitude)) * std::cos(deg2rad(to.latitude));
    auto x = std::cos(deg2rad(from.latitude)) * std::sin(deg2rad(to.latitude)) -
             std::sin(deg2rad(from.latitude)) * std::cos(deg2rad(to.latitude)) *
             std::cos(deg2rad(to.longitude) - deg2rad(from.longitude));
    auto brng = rad2deg(std::atan2(y, x));
    return ((int) brng + 180) % 360;
}

double angle_between(const Location &origin, const Location &pos1, const Location &pos2) {
    std::vector<Location> positions{pos1, pos2};
    std::vector<double> courses;

    auto lat_org = deg2rad(origin.latitude);
    auto lon_org = deg2rad(origin.longitude);

    std::for_each(positions.cbegin(), positions.cend(), [&lat_org, &lon_org, &courses](auto element) {
        auto lat_pos = deg2rad(element.latitude);
        auto lon_pos = deg2rad(element.longitude);

        auto val = std::atan2(std::sin(lon_org - lon_pos) * std::cos(lat_pos),
                              std::cos(lat_org) * std::sin(lat_pos) -
                              std::sin(lat_org) * std::cos(lat_pos) * std::cos(lon_org - lon_pos));

        courses.emplace_back(std::fmod(val, 2 * M_PI));
    });

    return rad2deg(
            std::acos(std::cos(courses[0]) * std::cos(courses[1]) + std::sin(courses[0]) * std::sin(courses[1])));
}

double angle_between(const Node &origin, const Node &node1, const Node &node2) {
    return angle_between(origin.get_location(), node1.get_location(), node2.get_location());
}

