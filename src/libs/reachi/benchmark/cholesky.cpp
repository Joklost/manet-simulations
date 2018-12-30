#include <iostream>

#include <reachi/cholesky.h>
#include <reachi/datagen.h>
#include <reachi/math.h>
#include <reachi/svd.h>
#include <reachi/linkmodel.h>
#include <reachi/clustering.h>
#include <reachi/ostr.h>
#include <mpilib/helpers.h>
#include <mpilib/objectifier.h>


int main(int argc, char *argv[]) {
    mpilib::geo::Location upper{57.01266813458001, 10.994625734716218};
    auto lower = mpilib::geo::square(upper, 300_m);

    //auto nodes = generate_nodes(static_cast<unsigned long>(atoi(argv[1])), upper, lower);
    auto nodes = reachi::data::generate_nodes(16, upper, lower);
    reachi::Optics optics{};
    auto eps = 0.01;
    auto minpts = 2;
    auto link_threshold = 350_m;


    auto ordering = optics.compute_ordering(nodes, eps, minpts);
    auto clusters = optics.cluster(ordering);
    auto links = reachi::data::create_link_vector(clusters, link_threshold);


    auto link_model = reachi::linkmodel::compute(links);
    std::cout << link_model << std::endl;
}