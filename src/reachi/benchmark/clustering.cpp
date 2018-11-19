#include <iostream>
#include <reachi/datagen.h>
#include <reachi/clustering.h>
#include <mpilib/node.h>


int main(int argc, char *argv[]) {
    Location upper{56.8804073,8.675316};
    Location lower{56.7391447,8.8079536};

    auto nodes = generate_nodes(10, upper, lower);

    Optics optics{};

    //std::cout << distance_between(upper, lower) << std::endl;

    auto clusters = optics.compute_clusters(nodes, 1.0, 2);

    for (auto &node : clusters) {
        std::cout << node.get_id() << " : " << node.get_reachability_distance() << std::endl;
    }
}