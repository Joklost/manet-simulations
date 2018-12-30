#include <mpilib/geomath.h>
#include <mpilib/location.h>
#include <random>
#include <functional>

bool mpilib::geo::Location::operator==(const mpilib::geo::Location &rhs) const {
    return latitude == rhs.get_latitude() &&
           longitude == rhs.get_longitude();
}

bool mpilib::geo::Location::operator!=(const mpilib::geo::Location &rhs) const {
    return !(rhs == *this);
}

bool mpilib::geo::Location::operator<(const mpilib::geo::Location &rhs) const {
    if (latitude < rhs.get_latitude())
        return true;
    if (rhs.get_latitude() < latitude)
        return false;
    return longitude < rhs.get_longitude();
}

bool mpilib::geo::Location::operator>(const mpilib::geo::Location &rhs) const {
    return rhs < *this;
}

bool mpilib::geo::Location::operator<=(const mpilib::geo::Location &rhs) const {
    return !(rhs < *this);
}

bool mpilib::geo::Location::operator>=(const mpilib::geo::Location &rhs) const {
    return !(*this < rhs);
}

mpilib::geo::Location
mpilib::geo::random_location(const mpilib::geo::Location &upper_bound, const mpilib::geo::Location &lower_bound) {
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

mpilib::geo::Location mpilib::geo::square(const mpilib::geo::Location &corner, double diag) {
    return mpilib::geo::move_location(move_location(corner, diag, 180), diag, 90);
}

/* https://stackoverflow.com/questions/7222382/get-lat-long-given-current-point-distance-and-bearing */
void mpilib::geo::Location::move(const int time, const double distance, const double bearing) {
    auto lat_origin = mpilib::geo::deg2rad(this->latitude);
    auto lon_origin = mpilib::geo::deg2rad(this->longitude);
    auto brng = mpilib::geo::deg2rad(bearing);

    auto lat_dest = std::asin(std::sin(lat_origin) * std::cos(distance / EARTH_RADIUS_KM) +
                              std::cos(lat_origin) * std::sin(distance / EARTH_RADIUS_KM) * cos(brng));
    auto lon_dest = lon_origin +
                    std::atan2(std::sin(brng) * std::sin(distance / EARTH_RADIUS_KM) * std::cos(lat_origin),
                               std::cos(distance / EARTH_RADIUS_KM) - std::sin(lat_origin) * std::sin(lat_dest));

    this->latitude = mpilib::geo::rad2deg(lat_dest);
    this->longitude = mpilib::geo::rad2deg(lon_dest);
}

double mpilib::geo::Location::get_latitude() const {
    return latitude;
}

double mpilib::geo::Location::get_longitude() const {
    return longitude;
}

int mpilib::geo::Location::get_time() const {
    return time;
}

void mpilib::geo::Location::set_time(int time) {
    mpilib::geo::Location::time = time;
}

mpilib::geo::Location::Location(double latitude, double longitude) : latitude(latitude), longitude(longitude),
                                                                     time(0) {}

mpilib::geo::Location::Location(int time, double latitude, double longitude) : time(time), latitude(latitude),
                                                                               longitude(longitude) {}

mpilib::geo::Location::Location() = default;

double mpilib::geo::literals::operator ""_km(long double km) {
    return static_cast<double>(km);
}

double mpilib::geo::literals::operator ""_km(unsigned long long km) {
    return static_cast<double>(km);
}

double mpilib::geo::literals::operator ""_m(long double m) {
    return static_cast<double>(m / KM);

}

double mpilib::geo::literals::operator ""_m(unsigned long long m) {
    return m / KM;
}
