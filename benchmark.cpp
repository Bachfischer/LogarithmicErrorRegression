#include <iostream>
#include <fstream>
#include "src/log_regression.h"
#include "src/competitor_regression.h"
#include "src/irls.h"
#include "src/poisoning.h"

#include "src/helpers/regression_benchmark.h"
#include "src/helpers/io_handler.h"
#include "src/theil_sen.h"
#include "src/fast_brute_force.h"

int main(int argc, char *argv[]){
    srand(time(NULL));

    std::vector<double> legitimate_data = parse_arguments(argc, argv);

    // Perform poisoning
    std::vector<double> poisoning_thresholds = { 0.05, 0.1, 0.15, 0.2};

    for (auto poisoning_threshold : poisoning_thresholds) {

        std::vector<double> poisoned_data = perform_poisoning(legitimate_data, poisoning_threshold);

        std::string data_name = argv[1];
        std::string legitimate_outfile = "results/benchmark_legitimate.csv";
        std::string poisoned_outfile = "results/benchmark_poisoned.csv";

        int num_lookups = 1000000;

        std::vector<double> legit_lookups;
        legit_lookups.resize(num_lookups);
        for (int i = 0; i < legit_lookups.size(); i++){
            legit_lookups[i] = legitimate_data[rand()%legitimate_data.size()];
        }

        std::cout << "Benchmark results for legitimate data: " << std::endl;
        benchmark_regression<simple_linear_regression_stable>(legitimate_data,legit_lookups,"SLR",data_name, poisoning_threshold, legitimate_outfile);
        benchmark_regression<create_regression_tournament_selection<LogNorm>>(legitimate_data,legit_lookups,"LogTE",data_name,poisoning_threshold, legitimate_outfile);
        benchmark_regression<create_regression_tournament_selection<FastDiscreteLogNorm>>(legitimate_data,legit_lookups,"DLogTE",data_name,poisoning_threshold, legitimate_outfile);
        benchmark_regression<build_regression_direct_descent>(legitimate_data,legit_lookups,"2P",data_name, poisoning_threshold, legitimate_outfile);
        benchmark_regression<theil_sen>(legitimate_data,legit_lookups,"TheilSen",data_name, poisoning_threshold, legitimate_outfile);
        benchmark_regression<create_regression_optimal<L1Norm>>(legitimate_data,legit_lookups,"LAD",data_name,poisoning_threshold, legitimate_outfile);
        std::cout << std::endl << std::endl;


        std::vector<double> poisoned_lookups;
        poisoned_lookups.resize(num_lookups);
        for (int i = 0; i < poisoned_lookups.size(); i++){
            poisoned_lookups[i] = poisoned_data[rand()%poisoned_data.size()];
        }
        std::cout << "Benchmark results for poisoned data: " << std::endl;
        benchmark_regression<simple_linear_regression_stable>(poisoned_data,poisoned_lookups,"SLR",data_name, poisoning_threshold,poisoned_outfile);
        benchmark_regression<create_regression_tournament_selection<LogNorm>>(poisoned_data,poisoned_lookups,"LogTE",data_name, poisoning_threshold,poisoned_outfile);
        benchmark_regression<create_regression_tournament_selection<FastDiscreteLogNorm>>(poisoned_data,poisoned_lookups,"DLogTE",data_name,poisoning_threshold, poisoned_outfile);
        benchmark_regression<build_regression_direct_descent>(poisoned_data,poisoned_lookups,"2P",data_name,poisoning_threshold, poisoned_outfile);
        benchmark_regression<theil_sen>(poisoned_data,poisoned_lookups,"TheilSen",data_name, poisoning_threshold,poisoned_outfile);
        benchmark_regression<create_regression_optimal<L1Norm>>(poisoned_data,poisoned_lookups,"LAD",data_name,poisoning_threshold, poisoned_outfile);

    }
}
