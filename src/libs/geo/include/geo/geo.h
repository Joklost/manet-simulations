#ifndef GEO_GEO_H
#define GEO_GEO_H

#include "location.h"
#include "geomath.h"

namespace geo {

    namespace literals {
        double operator ""_km(long double km);

        double operator ""_km(unsigned long long km);

        double operator ""_m(long double m);

        double operator ""_m(unsigned long long m);
    }

}

using namespace geo::literals;

#endif /* GEO_GEO_H */
