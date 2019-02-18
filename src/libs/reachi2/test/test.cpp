#include <catch2/catch.hpp>
#include <Eigen/Cholesky>
#include <iostream>

#include "reachi2/math.h"

TEST_CASE("ol", "[d]") {
    Eigen::MatrixXd matrix{};

    Eigen::LDLT<Eigen::Ref<Eigen::MatrixXd>> ldlt(matrix);

    std::cout << matrix << std::endl;
}