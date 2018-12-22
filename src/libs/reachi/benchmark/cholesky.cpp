#include <iostream>

#include <reachi/cholesky.h>
#include <reachi/datagen.h>
#include <reachi/math.h>
#include <reachi/svd.h>
#include <mpilib/helpers.h>
#include <mpilib/objectifier.h>


struct packet {
    int id;
    std::vector<double> data;
};


int main(int argc, char *argv[]) {
    mpilib::geo::Location upper{57.0134, 9.99008};
    mpilib::geo::Location lower{57.0044, 10.0066};

    //auto nodes = generate_nodes(static_cast<unsigned long>(atoi(argv[1])), upper, lower);
    auto nodes = reachi::data::generate_nodes(10, upper, lower);
    auto links = reachi::data::create_link_vector(nodes, 1);

    packet p{};
    p.data.emplace_back(2.2);
    auto res = mpilib::serialise(p);
    std::cout << res << std::endl;
}