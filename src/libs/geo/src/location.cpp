#include <random>
#include <functional>

#include <geo/geo.h>

bool geo::Location::operator==(const geo::Location &rhs) const {
    return latitude == rhs.get_latitude() &&
           longitude == rhs.get_longitude();
}

bool geo::Location::operator!=(const geo::Location &rhs) const {
    return !(rhs == *this);
}

geo::Location geo::random_location(const geo::Location &upper_bound, const geo::Location &lower_bound) {
    auto lat_min = lower_bound.get_latitude();
    auto lat_max = upper_bound.get_latitude();
    std::random_device rd_lat;
    std::default_random_engine eng_lat(rd_lat());
    std::uniform_real_distribution<double> dist_lat{lat_min, lat_max};
    auto gen_lat = std::bind(dist_lat, eng_lat);

    auto lon_min = lower_bound.get_longitude();
    auto lon_max = upper_bound.get_longitude();
    std::random_device rd_lon;
    std::default_random_engine eng_lon(rd_lon());
    std::uniform_real_distribution<double> dist_lon{lon_min, lon_max};
    auto gen_lon = std::bind(dist_lon, eng_lon);

    return {gen_lat(), gen_lon()};
}

geo::Location geo::square(const geo::Location &corner, double diag) {
    return geo::move_location(move_location(corner, diag, 180), diag, 90);
}

/* https://stackoverflow.com/questions/7222382/get-lat-long-given-current-point-distance-and-bearing */
void geo::Location::move(const unsigned long new_time, const double distance, const double bearing) {
    auto lat_origin = geo::deg2rad(this->latitude);
    auto lon_origin = geo::deg2rad(this->longitude);
    auto brng = geo::deg2rad(bearing);

    auto lat_dest = std::asin(std::sin(lat_origin) * std::cos(distance / EARTH_RADIUS_KM) +
                              std::cos(lat_origin) * std::sin(distance / EARTH_RADIUS_KM) * cos(brng));
    auto lon_dest = lon_origin +
                    std::atan2(std::sin(brng) * std::sin(distance / EARTH_RADIUS_KM) * std::cos(lat_origin),
                               std::cos(distance / EARTH_RADIUS_KM) - std::sin(lat_origin) * std::sin(lat_dest));

    this->latitude = geo::rad2deg(lat_dest);
    this->longitude = geo::rad2deg(lon_dest);
}

double geo::Location::get_latitude() const {
    return latitude;
}

double geo::Location::get_longitude() const {
    return longitude;
}

unsigned long geo::Location::get_time() const {
    return time;
}

void geo::Location::set_time(unsigned long new_time) {
    geo::Location::time = new_time;
}

geo::Location::Location(double latitude, double longitude) : latitude(latitude), longitude(longitude),
                                                             time(0) {}

geo::Location::Location(unsigned long time, double latitude, double longitude) : time(time), latitude(latitude),
                                                                                 longitude(longitude) {}

bool geo::Location::operator<(const geo::Location &rhs) const {
    return time < rhs.time;
}

bool geo::Location::operator>(const geo::Location &rhs) const {
    return rhs < *this;
}

bool geo::Location::operator<=(const geo::Location &rhs) const {
    return !(rhs < *this);
}

bool geo::Location::operator>=(const geo::Location &rhs) const {
    return !(*this < rhs);
}

geo::Location::Location() = default;

/**
 * Kiloeter literal.
 * @param m Distance in kilometers.
 * @return m.
 */
double geo::literals::operator ""_km(long double km) {
    return static_cast<double>(km);
}

/**
 * Kiloeter literal.
 * @param m Distance in kilometers.
 * @return m.
 */
double geo::literals::operator ""_km(unsigned long long km) {
    return static_cast<double>(km);
}

/**
 * Meter literal.
 * @param m Distance in meters.
 * @return m converted to kilometers.
 */
double geo::literals::operator ""_m(long double m) {
    return static_cast<double>(m / KM);

}

/**
 * Meter literal.
 * @param m Distance in meters.
 * @return m converted to kilometers.
 */
double geo::literals::operator ""_m(unsigned long long m) {
    return m / KM;
}
