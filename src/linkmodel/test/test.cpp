#include <catch.hpp>

#include "linkmodel/cholesky.h"

TEST_CASE("Compute the Cholesky decomposition (slow)", "[math]") {
    vecvec<double> matrix{{25.0, 15.0, -5.0},
                          {15.0, 18.0, 0.0},
                          {-5.0, 0.0,  11.0}};

    vecvec<double> result{{5.0,  0.0, 0.0},
                          {3.0,  3.0, 0.0},
                          {-1.0, 1.0, 3.0}};
    REQUIRE(result == slow_cholesky(matrix));
}
