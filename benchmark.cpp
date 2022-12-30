#include <iostream>
#include <fstream>
#include "src/log_regression.h"
#include "src/competitor_regression.h"
#include "src/irls.h"
#include "src/poisoning.h"

#include "src/helpers/regression_benchmark.h"
#include "src/helpers/alex_benchmark.h"
#include "src/helpers/pgm_benchmark.h"
#include "src/helpers/io_handler.h"
#include "src/theil_sen.h"
#include "src/fast_brute_force.h"

int main(int argc, char *argv[]){
    srand(time(NULL));

    std::vector<double> legitimate_data = parse_arguments(argc, argv);


    std::cout << "Generating poisoning keys" << std::endl;

    // Generate vector of poisoning thresholds from 0.01, 0.02, .... , 0.2
    std::vector<double> poisoning_thresholds;
    for (double threshold = 0.01; threshold <= 0.21; threshold = threshold + 0.01) {
        poisoning_thresholds.push_back(threshold);
    }

    // Perform poisoning
    for (auto poisoning_threshold : poisoning_thresholds) {

        std::vector<double> poisoned_data = perform_poisoning(legitimate_data, poisoning_threshold);

        std::string data_name = argv[1];
        std::string legitimate_outfile = "results/benchmark_legitimate.csv";
        std::string poisoned_outfile = "results/benchmark_poisoned.csv";

        int num_lookups = 1000000;

        std::vector<double> lookups;
        lookups.resize(num_lookups);
        for (int i = 0; i < lookups.size(); i++){
            lookups[i] = legitimate_data[rand() % legitimate_data.size()];
        }

        std::cout << std::endl << std::endl;
        std::cout << "Benchmark results for legitimate data: " << std::endl;
        benchmark_regression<simple_linear_regression_stable>(legitimate_data,lookups,"SLR",data_name, poisoning_threshold, legitimate_outfile);
        benchmark_regression<create_regression_tournament_selection<LogNorm>>(legitimate_data,lookups,"LogTE",data_name,poisoning_threshold, legitimate_outfile);
        benchmark_regression<create_regression_tournament_selection<FastDiscreteLogNorm>>(legitimate_data,lookups,"DLogTE",data_name,poisoning_threshold, legitimate_outfile);
        benchmark_regression<build_regression_direct_descent>(legitimate_data,lookups,"2P",data_name, poisoning_threshold, legitimate_outfile);
        benchmark_regression<theil_sen>(legitimate_data,lookups,"TheilSen",data_name, poisoning_threshold, legitimate_outfile);
        benchmark_regression<create_regression_optimal<L1Norm>>(legitimate_data,lookups,"LAD",data_name,poisoning_threshold, legitimate_outfile);
        benchmark_alex(legitimate_data, lookups, "ALEX", data_name, poisoning_threshold, legitimate_outfile);
        benchmark_pgm<2>(legitimate_data, lookups, "PGM_2", data_name, poisoning_threshold, legitimate_outfile);
        benchmark_pgm<4>(legitimate_data, lookups, "PGM_4", data_name, poisoning_threshold, legitimate_outfile);
        benchmark_pgm<8>(legitimate_data, lookups, "PGM_8", data_name, poisoning_threshold, legitimate_outfile);
        benchmark_pgm<16>(legitimate_data, lookups, "PGM_16", data_name, poisoning_threshold, legitimate_outfile);
        benchmark_pgm<32>(legitimate_data, lookups, "PGM_32", data_name, poisoning_threshold, legitimate_outfile);
        benchmark_pgm<64>(legitimate_data, lookups, "PGM_64", data_name, poisoning_threshold, legitimate_outfile);




        std::cout << std::endl << std::endl;
        std::cout << "Benchmark results for poisoned data: " << std::endl;
        benchmark_regression<simple_linear_regression_stable>(poisoned_data,lookups,"SLR",data_name, poisoning_threshold,poisoned_outfile);
        benchmark_regression<create_regression_tournament_selection<LogNorm>>(poisoned_data,lookups,"LogTE",data_name, poisoning_threshold,poisoned_outfile);
        benchmark_regression<create_regression_tournament_selection<FastDiscreteLogNorm>>(poisoned_data,lookups,"DLogTE",data_name,poisoning_threshold, poisoned_outfile);
        benchmark_regression<build_regression_direct_descent>(poisoned_data,lookups,"2P",data_name,poisoning_threshold, poisoned_outfile);
        benchmark_regression<theil_sen>(poisoned_data,lookups,"TheilSen",data_name, poisoning_threshold,poisoned_outfile);
        benchmark_regression<create_regression_optimal<L1Norm>>(poisoned_data,lookups,"LAD",data_name,poisoning_threshold, poisoned_outfile);
        benchmark_alex(poisoned_data, lookups, "ALEX", data_name, poisoning_threshold, poisoned_outfile);
        benchmark_pgm<2>(poisoned_data, lookups, "PGM_2", data_name, poisoning_threshold, poisoned_outfile);
        benchmark_pgm<4>(poisoned_data, lookups, "PGM_4", data_name, poisoning_threshold, poisoned_outfile);
        benchmark_pgm<8>(poisoned_data, lookups, "PGM_8", data_name, poisoning_threshold, poisoned_outfile);
        benchmark_pgm<16>(poisoned_data, lookups, "PGM_16", data_name, poisoning_threshold, poisoned_outfile);
        benchmark_pgm<32>(poisoned_data, lookups, "PGM_32", data_name, poisoning_threshold, poisoned_outfile);
        benchmark_pgm<64>(poisoned_data, lookups, "PGM_64", data_name, poisoning_threshold, poisoned_outfile);



    }
}
