#include <iostream>
#include <fstream>

#include <reachi2/datagen.h>
#include <geo/geomath.h>

void generate_nodes_data_file(const int count, geo::Location upper, geo::Location lower) {
    auto nodes = reachi2::data::generate_nodes(count, upper, lower);
    std::fstream stream("data/nodes", std::ios::out | std::ios::trunc);



    stream.close();
}

int main(int argc, char *argv[]) {
    geo::Location upper{57.01266813458001, 10.994625734716218};
    auto lower = geo::square(upper, 2_km);

    generate_nodes_data_file(20, upper, lower);

    return 0;
}