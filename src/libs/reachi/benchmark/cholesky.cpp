#include <iostream>

#include <reachi/cholesky.h>
#include <reachi/datagen.h>
#include <reachi/math.h>
#include <reachi/svd.h>
#include <reachi/linkmodel.h>
#include <reachi/clustering.h>
#include <mpilib/helpers.h>
#include <mpilib/objectifier.h>


int main(int argc, char *argv[]) {
    mpilib::geo::Location upper{57.01266813458001, 10.994625734716218};
    auto lower = mpilib::geo::square(upper, 5_km);

    //auto nodes = generate_nodes(static_cast<unsigned long>(atoi(argv[1])), upper, lower);
    auto step = 10ul;
    auto steps = 20ul;
    for (auto i = 0ul; i < steps; ++i) {
        auto nodes = reachi::data::generate_nodes(i * step + step, upper, lower);
        reachi::Optics optics{};
        auto eps = 0.01;
        auto minpts = 2;
        auto link_threshold = 0_km;

        auto ordering = optics.compute_ordering(nodes, eps, minpts);
        auto clusters = optics.cluster(ordering);
        auto links = reachi::data::create_link_vector(clusters, link_threshold);

        std::cout << "clusters: " << clusters.size() << std::endl;
        std::cout << "links: " << links.size() << std::endl;
        auto corr = reachi::math::generate_correlation_matrix(links);

        auto std_deviation = std::pow(STANDARD_DEVIATION, 2);
        auto sigma = std_deviation * corr;

        if (!reachi::cholesky::is_positive_definite(sigma)) {
            std::cout << "ensuring psd" << std::endl;
            auto spdstart = std::chrono::high_resolution_clock::now();
            sigma = reachi::linkmodel::ensure_positive_definiteness(sigma);
            auto spdduration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - spdstart);
            std::cout << "spdduration: " << spdduration.count() << std::endl;
        }

        auto start = std::chrono::high_resolution_clock::now();
        auto c = reachi::cholesky::cholesky(sigma);
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
        std::cout << "duration: " << duration.count() << "\n" << std::endl;

    }

}