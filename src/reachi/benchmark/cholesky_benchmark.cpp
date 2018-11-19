#include <iostream>
#include <reachi/cholesky.h>
#include "../src/datagen.h"
#include "../src/math.cpp"

int main(int argc, char *argv[]) {
    auto upper = Location{57.01266813458001, 9.994625734716218};
    auto lower = Location{57.0117698, 9.9929758};
    auto nodes = generate_nodes(static_cast<unsigned long>(atoi(argv[1])), upper, lower);

    auto links = generate_links(nodes);

    auto corr = generate_correlation_matrix(links);
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = corr * std_deviation;

    //auto temp = cholesky(sigma);
    std::cout << measure<>::execution(cholesky, sigma) << std::endl;
}