#include <iostream>
#include <reachi/cholesky.h>
#include <reachi/datagen.h>
#include <reachi/math.h>
#include <mpilib/helpers.h>

int main(int argc, char *argv[]) {
    auto upper = Location{57.01266813458001, 10.994625734716218};
    auto lower = Location{57.0117698, 10.9929758};
    //auto nodes = generate_nodes(static_cast<unsigned long>(atoi(argv[1])), upper, lower);
    auto nodes = generate_nodes(8, upper, lower);
    auto links = create_link_vector(nodes, 1);


    auto begin = std::chrono::steady_clock::now();
    auto res = generate_correlation_matrix_slow(links);
    //std::cout << res << std::endl;
    auto std_deviation_slow = std::pow(11.4, 2);
    auto sigma_slow = res * std_deviation_slow;
    auto temp_slow = slow_cholesky(sigma_slow);
    auto end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
    std::cout << temp_slow << std::endl;

    begin = std::chrono::steady_clock::now();
    res = generate_correlation_matrix_vector(links);
    //std::cout << res << std::endl;
    std_deviation_slow = std::pow(11.4, 2);
    sigma_slow = res * std_deviation_slow;
    temp_slow = slow_cholesky(sigma_slow);
    end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
    std::cout << temp_slow << std::endl;




    /* std::cout << "Original cholesky\n" << std::endl;
     auto begin = std::chrono::steady_clock::now();
     auto corr_slow = generate_correlation_matrix_slow(links);
     auto end = std::chrono::steady_clock::now();
     std::cout << "correlation matrix = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;

     auto std_deviation_slow = std::pow(11.4, 2);
     auto sigma_slow = corr_slow * std_deviation_slow;

     begin = std::chrono::steady_clock::now();
     auto temp_slow = slow_cholesky(sigma_slow);
     end = std::chrono::steady_clock::now();
     std::cout << "cholesky = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
     std::cout << "\n\nOur cholesky\n" << std::endl;
     begin = std::chrono::steady_clock::now();
     auto corr = generate_correlation_matrix(links);
     end = std::chrono::steady_clock::now();
     std::cout << "correlation matrix = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;

     auto std_deviation = std::pow(11.4, 2);
     auto sigma = corr * std_deviation;

     begin = std::chrono::steady_clock::now();
     auto temp = cholesky(sigma);
     end = std::chrono::steady_clock::now();
     std::cout << "cholesky = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl; */
}