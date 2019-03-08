#include <iostream>

#include <reachi/cholesky.h>
#include <reachi/datagen.h>
#include <reachi/math.h>
#include <reachi/svd.h>
#include <reachi/linkmodel.h>
#include <reachi/ostr.h>
#include <reachi/clustering.h>
#include <mpilib/helpers.h>
#include <mpilib/objectifier.h>

template<class...Durations, class DurationIn>
std::tuple<Durations...> break_down_durations(DurationIn d) {
    std::tuple<Durations...> retval;
    using discard=int[];
    (void) discard{0, (void((
                                    (std::get<Durations>(retval) = std::chrono::duration_cast<Durations>(d)),
                                            (d -= std::chrono::duration_cast<DurationIn>(std::get<Durations>(retval)))
                            )), 0)...};
    return retval;
}

std::string format_duration(std::chrono::microseconds us) {
    auto dur = break_down_durations<std::chrono::seconds, std::chrono::milliseconds, std::chrono::microseconds>(us);
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::get<0>(dur).count()
        << "::"
        << std::setw(3)
        << std::get<1>(dur).count()
        << "::"
        << std::setw(3)
        << std::get<2>(dur).count();
    return oss.str();
}

int main(int argc, char *argv[]) {
    geo::Location upper{57.01266813458001, 10.994625734716218};
    auto lower = geo::square(upper, 5_km);

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

        if (!reachi::cholesky::is_positive_definite(corr)) {
            std::cout << "ensuring psd" << std::endl;
            auto spdstart = std::chrono::high_resolution_clock::now();
            corr = reachi::linkmodel::near_pd(corr);
            auto spdduration = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - spdstart);
            std::cout << "spdduration: " << format_duration(spdduration) << std::endl;
        }

        auto std_deviation = std::pow(STANDARD_DEVIATION, 2);
        auto sigma = std_deviation * corr;

        auto start = std::chrono::high_resolution_clock::now();
        auto c = reachi::cholesky::cholesky(sigma);
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start);
        std::cout << "duration: " << format_duration(duration) << "\n" << std::endl;
    }
}

/*
 * clusters: 100
    links: 4950
    ensuring psd
    spdduration: 3154::408::436
    duration: 51::547::003
 */