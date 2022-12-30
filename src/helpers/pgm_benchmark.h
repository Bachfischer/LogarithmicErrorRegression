#pragma once

#include <chrono>
#include <random>
#include <iostream>
#include <algorithm>
#include <string>
#include "./competitors/PGM/include/pgm/pgm_index.hpp"
#include "../exponential_search.h"

#define KEY_TYPE double

/*
    Calculates the error for a single element for a certain linear function
*/
template<ERROR_TYPE E, bool ROUND = true, bool BOUNDED = true, int EPSILON>
inline double calculate_error_single_element_pgm(std::vector<double>& data, pgm::PGMIndex<KEY_TYPE, EPSILON> & index, int i){
    double x = data[i];
    double y = i;
    auto approx_range = index.search(x);
    auto low_pos = approx_range.lo;

    return apply_errorfn<E, ROUND, BOUNDED>(low_pos, y, data.size()-1);
}

/*
    Calculates the total error for a linear function
*/
template<ERROR_TYPE E, bool CORRECT = true, bool BOUNDED = true, int EPSILON>
long double calculate_error_pgm(std::vector<double>& data, pgm::PGMIndex<KEY_TYPE, EPSILON> & index) {
    long double total_error = 0;
    for (long i = 0; i < data.size(); i++) {
        total_error += calculate_error_single_element_pgm<E, CORRECT, BOUNDED, EPSILON>(data, index, i);
    }
    return total_error;
}

/*
    Benchmark PGM by measuring lookup times
*/
template<int EPSILON>
long benchmark_lookup_pgm(std::vector<double> & data, std::vector<double> lookups, pgm::PGMIndex<KEY_TYPE, EPSILON> & index){
    int data_size = data.size()-1;
    auto start = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < lookups.size(); i++){
        auto approx_range = index.search(lookups[i]);
        DoNotOptimize(binary_search_lower_bound(approx_range.lo, approx_range.hi, lookups[i], data));
    }//

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);

    return duration.count();
}

template<int EPSILON>
std::vector<long> perform_benchmark(pgm::PGMIndex<KEY_TYPE, EPSILON> & index, std::vector<double>& data, std::vector<double>& lookups, int num){
    std::vector<long> measurements;
    for (int i = 0; i < num; i++){
        long time = benchmark_lookup_pgm<EPSILON>(data, lookups, index);
        measurements.push_back(time);
    }

    std::sort(measurements.begin(), measurements.end());
    return measurements;
}

template<int EPSILON>
void benchmark_pgm(std::vector<double> & data, std::vector<double> & lookups, std::string regression_name, std::string data_name, double poisoning_threshold, std::string outfile){

    auto start = std::chrono::high_resolution_clock::now();

    // Create PGM and bulk load
    pgm::PGMIndex<KEY_TYPE, EPSILON> index(data);

    auto stop = std::chrono::high_resolution_clock::now();
    long build_time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

    std::vector<long> measurements = perform_benchmark<EPSILON>(index, data, lookups, 30);

    double mean = 0;
    double median = measurements[measurements.size()/2];
    for(int i = 0; i < measurements.size(); i++){
        mean += measurements[i];
    }
    mean /= measurements.size();

    mean /= lookups.size();
    median /= lookups.size();

    long log_error = calculate_error_pgm<LogNorm, true, false, EPSILON>(data, index);
    long d_log_error = calculate_error_pgm<DiscreteLogNorm, true, false, EPSILON>(data, index);
    long mse_error = calculate_error_pgm<L2Norm, true, false, EPSILON>(data, index);

    std::ofstream file;
    file.open(outfile, std::ios_base::app);
    file << regression_name << ";" << data_name << ";" << poisoning_threshold << ";" << data.size() << ";" << lookups.size() << ";" << mean << ";" << median << ";" << log_error << ";" << d_log_error << ";" << mse_error << ";" << build_time << std::endl;
    file.close();

    std::cout << regression_name << " data_name:" << data_name << "; poisoning_threshold: " << poisoning_threshold << "; data size:" << data.size() << " lookups size:" << lookups.size() << " mean lookup ns:" << mean << " median lookup ns:" << median << " log error:" << log_error << " discrete log error:" << d_log_error << " mse error:" << mse_error << " build time:" << build_time << std::endl;
}