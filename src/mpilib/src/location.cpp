#include <mpilib/geomath.h>
#include <mpilib/location.h>

bool Location::operator==(const Location &rhs) const {
    return latitude == rhs.get_latitude() &&
           longitude == rhs.get_longitude();
}

bool Location::operator!=(const Location &rhs) const {
    return !(rhs == *this);
}

bool Location::operator<(const Location &rhs) const {
    if (latitude < rhs.get_latitude())
        return true;
    if (rhs.get_latitude() < latitude)
        return false;
    return longitude < rhs.get_longitude();
}

bool Location::operator>(const Location &rhs) const {
    return rhs < *this;
}

bool Location::operator<=(const Location &rhs) const {
    return !(rhs < *this);
}

bool Location::operator>=(const Location &rhs) const {
    return !(*this < rhs);
}

std::ostream &operator<<(std::ostream &os, const Location &location) {
    os << " latitude: " << location.get_latitude() << ", longitude: "
       << location.get_longitude();
    return os;
}

/* https://stackoverflow.com/questions/7222382/get-lat-long-given-current-point-distance-and-bearing */
void Location::move(const int time, const double distance, const double bearing) {
    auto lat_origin = deg2rad(this->latitude);
    auto lon_origin = deg2rad(this->longitude);
    auto brng = deg2rad(bearing);

    auto lat_dest = std::asin(std::sin(lat_origin) * std::cos(distance / EARTH_RADIUS_KM) +
                              std::cos(lat_origin) * std::sin(distance / EARTH_RADIUS_KM) * cos(brng));
    auto lon_dest = lon_origin +
                    std::atan2(std::sin(brng) * std::sin(distance / EARTH_RADIUS_KM) * std::cos(lat_origin),
                               std::cos(distance / EARTH_RADIUS_KM) - std::sin(lat_origin) * std::sin(lat_dest));

    this->latitude = rad2deg(lat_dest);
    this->longitude = rad2deg(lon_dest);
}

double Location::get_latitude() const {
    return latitude;
}

double Location::get_longitude() const {
    return longitude;
}

int Location::get_time() const {
    return time;
}

void Location::set_time(int time) {
    Location::time = time;
}

Location::Location(double latitude, double longitude) : latitude(latitude), longitude(longitude), time(0) {}

Location::Location(int time, double latitude, double longitude) : time(time), latitude(latitude),
                                                                  longitude(longitude) {}

Location::Location() {

}
