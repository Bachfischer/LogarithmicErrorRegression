#pragma once

#include <iostream>
#include "./competitors/ALEX/src/core/alex.h"

#define KEY_TYPE double
#define PAYLOAD_TYPE double

/*
    Calculates the error for a single element for a certain linear function
*/
template<ERROR_TYPE E, bool ROUND = true, bool BOUNDED = true>
inline double calculate_error_single_element_alex(std::vector<double>& data, alex::Alex<KEY_TYPE, PAYLOAD_TYPE> & index, int i){
    double x = data[i];
    double y = i;
    auto it = index.lower_bound(x);
    return 0;
    // Cannot calculate error since lower_bound always return correct position
    //return apply_errorfn<E, ROUND, BOUNDED>(low_pos, y, data.size()-1);
}

/*
    Calculates the total error for a linear function
*/
template<ERROR_TYPE E, bool CORRECT = true, bool BOUNDED = true>
long double calculate_error_alex(std::vector<double>& data, alex::Alex<KEY_TYPE, PAYLOAD_TYPE> & index) {
    long double total_error = 0;
    for (long i = 0; i < data.size(); i++) {
        total_error += calculate_error_single_element_alex<E, CORRECT, BOUNDED>(data, index, i);
    }
    return total_error;
}

/*
    Benchmark ALEX by measuring lookup times
*/
long benchmark_lookup_alex(std::vector<double> & data, std::vector<double> lookups, alex::Alex<KEY_TYPE, PAYLOAD_TYPE> & index){
    int data_size = data.size()-1;
    auto start = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < lookups.size(); i++){
        //DoNotOptimize(index.lower_bound(lookups[i]));
        auto payload = index.get_payload(lookups[i]);
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);

    return duration.count();
}

std::vector<long> perform_benchmark(alex::Alex<KEY_TYPE, PAYLOAD_TYPE> & index, std::vector<double>& data, std::vector<double>& lookups, int num){
    std::vector<long> measurements;
    for (int i = 0; i < num; i++){
        long time = benchmark_lookup_alex(data, lookups, index);
        measurements.push_back(time);
    }

    std::sort(measurements.begin(), measurements.end());
    return measurements;
}

void benchmark_alex(std::vector<double> & data, std::vector<double> & lookups, std::string regression_name, std::string data_name, double poisoning_threshold, std::string outfile){

    // Combine pre-defined keys with randomly generated payloads (can not pass a simple vector to ALEX)
    auto values = new std::pair<KEY_TYPE, PAYLOAD_TYPE>[data.size()];
    std::mt19937_64 gen_payload(std::random_device{}());
    for (int i = 0; i < data.size(); i++) {
        values[i].first = data[i];
        values[i].second = static_cast<PAYLOAD_TYPE>(gen_payload());
    }
    auto start = std::chrono::high_resolution_clock::now();

    // Create ALEX and bulk load
    alex::Alex<KEY_TYPE, PAYLOAD_TYPE> index;
    index.bulk_load(values, data.size());

    auto stop = std::chrono::high_resolution_clock::now();
    long build_time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

    std::vector<long> measurements = perform_benchmark(index, data, lookups, 30);

    double mean = 0;
    double median = measurements[measurements.size()/2];
    for(int i = 0; i < measurements.size(); i++){
        mean += measurements[i];
    }
    mean /= measurements.size();

    mean /= lookups.size();
    median /= lookups.size();

    long log_error = calculate_error_alex<LogNorm, true, false>(data, index);
    long d_log_error = calculate_error_alex<DiscreteLogNorm, true, false>(data, index);
    long mse_error = calculate_error_alex<L2Norm, true, false>(data, index);

    std::ofstream file;
    file.open(outfile, std::ios_base::app);
    file << regression_name << ";" << data_name << ";" << poisoning_threshold << ";" << data.size() << ";" << lookups.size() << ";" << mean << ";" << median << ";" << log_error << ";" << d_log_error << ";" << mse_error << ";" << build_time << std::endl;
    file.close();

    std::cout << regression_name << " data_name:" << data_name << "; poisoning_threshold: " << poisoning_threshold << "; data size:" << data.size() << " lookups size:" << lookups.size() << " mean lookup ns:" << mean << " median lookup ns:" << median << " log error:" << log_error << " discrete log error:" << d_log_error << " mse error:" << mse_error << " build time:" << build_time << std::endl;

    delete[] values;
}