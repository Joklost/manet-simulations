#include <iostream>

#include <reachi/cholesky.h>
#include <reachi/datagen.h>
#include <reachi/math.h>
#include <reachi/svd.h>
#include <mpilib/helpers.h>

int main(int argc, char *argv[]) {
    mpilib::geo::Location upper{57.0134, 9.99008};
    mpilib::geo::Location lower{57.0044, 10.0066};

    //auto nodes = generate_nodes(static_cast<unsigned long>(atoi(argv[1])), upper, lower);
    auto nodes = generate_nodes(10, upper, lower);
    auto links = create_link_vector(nodes, 1);

    // testing original implementation
    /*auto begin = std::chrono::steady_clock::now();

    auto corr_org = generate_correlation_matrix_slow(links);
    auto std_deviation_org = std::pow(11.4, 2);
    auto sigma_org = corr_org * std_deviation_org;
    auto cholesky_res_org = slow_cholesky(sigma_org);

    auto end = std::chrono::steady_clock::now();
    std::cout << "Original code: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;*/


    // testing our implementation
    /*   auto begin_1 = std::chrono::steady_clock::now();


       auto corr_our = generate_correlation_matrix(links);
       auto std_deviation_our = std::pow(11.4, 2);
       auto sigma_our = corr_our * std_deviation_our;
       auto cholesky_res_our = cholesky(sigma_our);

       auto end_1 = std::chrono::steady_clock::now();
       std::cout << "Our code: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_1 - begin_1).count() << std::endl;*/
    //std::cout << cholesky_res_our << std::endl;



    vecvec<double> data{{2, 5, 3},
                        /*{1, 2, 1},
                        {4, 1, 1},
                        {3, 5, 2},
                        {5, 3, 1},
                        {4, 5, 5},*/
                        {2, 4, 2},
                        {2, 2, 5}};
    auto begin_1 = std::chrono::steady_clock::now();

    std::vector<double> singular_values, u, v {};
    auto res = svd(data);
    auto end_1 = std::chrono::steady_clock::now();
    std::cout << "Our code: " << std::chrono::duration_cast<std::chrono::microseconds>(end_1 - begin_1).count()
              << std::endl;
}