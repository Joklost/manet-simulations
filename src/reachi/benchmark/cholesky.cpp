#include <iostream>
#include <reachi/cholesky.h>
#include <reachi/datagen.h>
#include <reachi/math.h>
#include <mpilib/helpers.h>

int main(int argc, char *argv[]) {
    auto upper = Location{57.01266813458001, 9.994625734716218};
    auto lower = Location{57.0117698, 9.9929758};
    auto nodes = generate_nodes(50, upper, lower);
    auto links = create_link_vector(nodes, 0.101);


    //auto begin = std::chrono::steady_clock::now();
    auto corr = generate_correlation_matrix(links);
    auto std_deviation = std::pow(11.4, 2);
    auto sigma = corr * std_deviation;

    auto temp = cholesky(sigma);
    //std::cout << measure<>::execution(cholesky, sigma) << std::endl;

    //auto end = std::chrono::steady_clock::now();
    //std::cout << "Time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
}