#pragma once

#include <set>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <vector>

template<typename T>
std::vector<std::size_t> tag_sort(const std::vector<T>& v)
{
    std::vector<std::size_t> result(v.size());
    std::iota(std::begin(result), std::end(result), 0);
    std::sort(std::begin(result), std::end(result),
              [&v](const auto & lhs, const auto & rhs)
              {
                  return v[lhs] < v[rhs];
              }
    );
    return result;
}

/* Compute the rank that key S(i) would have if it was inserted in K ∪ P and assign this rank as the i-th element of the new sequence */
std::vector<size_t> compute_rank_for_endpoints(std::set<double> & endpoints, std::vector<double> & keyset){
    std::vector<size_t> computed_rank_for_endpoint;

    for (double endpoint: endpoints){
        std::vector<double> keyset_extended;
        std::copy(keyset.begin(), keyset.end(), std::back_inserter(keyset_extended));
        keyset_extended.push_back(endpoint);

        std::cout << std::endl << "Extended keyset" << std::endl;
        for (double i: keyset_extended)
            std::cout << i << ' ';
        std::cout << std::endl;

        //std::vector<size_t> rank = tag_sort(keyset_extended);
        auto iter = std::lower_bound(keyset_extended.begin(), keyset_extended.end(), endpoint);
        int rank = int(iter - keyset_extended.begin());
        std::cout << "Rank of endpoint " << endpoint << ": " << rank << std::endl;

        computed_rank_for_endpoint.push_back(rank);
    }
    return computed_rank_for_endpoint;
}

/* Extract non-occupied keys for a given sequence of legitimate and poisoning keys */
std::set<double> partition_non_occupied_keys(std::vector<double> & K, std::set<double> & P) {

    std::vector <double> keyset;
    std::merge(K.begin(), K.end(), P.begin(), P.end(), std::back_inserter(keyset));

    for (double i: keyset)
        std::cout << i << ' ';
    std::cout << std::endl;


    size_t n = keyset.size();
    std::cout << "Keyset size: " << n << std::endl;


    double lower_bound = keyset[0] - 1;
    double upper_bound = keyset[n-1] + 1;
    std::cout << "Lower bound: " << lower_bound << std::endl;
    std::cout << "Upper bound: " << upper_bound << std::endl;


    std::set <double> endpoints;
    bool is_in_sequence = false;

    for(int i = lower_bound; i <= upper_bound +1; i++) {
        if(std::find(keyset.begin(), keyset.end(), i) == keyset.end() and is_in_sequence == false) {
            /* if key i is at start of sequence */
            is_in_sequence = true;
            endpoints.insert(i);

        } else if(std::find(keyset.begin(), keyset.end(), i) == keyset.end() and is_in_sequence == true and std::find(keyset.begin(), keyset.end(), i+1) != keyset.end()){
            /* if key i is at end of sequence */
            endpoints.insert(i);
            is_in_sequence = false;
        }
    }
    return endpoints;

}

std::vector<double> obtain_poisoning_keys(double poisoning_threshold, std::vector<double> & keyset, std::vector<size_t> &rankset) {
    // Total number of elements
    int n = keyset.size();
    std::cout << std::endl << "Number of legitimate keys: " << n << std::endl;

    // Number of poisoning keys P
    int P = int(poisoning_threshold * n);
    std::cout << "Number of poisoning keys to be generated: " << P << std::endl;

    std::set<double> poisoning_keys;

    while(poisoning_keys.size() < P){
        std::cout << "Current status: " << poisoning_keys.size() << " out of " << P << " poisoning keys generated." << std::endl;

        // Partition the non-occupied keys into subsequences such that each subsequence consists of consecutive non-occupied keys;
        // Extract the endpoints of each subsequence and sort them to construct the new sequence of endpoints S(i), where i <= 2(n + j);

        // S: endpoints
        std::set<double> S = partition_non_occupied_keys(keyset, poisoning_keys);
        std::cout << "Length of endpoints: " << S.size() << std::endl;

        // Endpoint keys are continuously increasing
        std::cout << "Endpoint keys: " << std::endl;
        for (double i: S)
            std::cout << i << ' ';

        // Compute the rank that key S(i) would have if it was inserted in K ∪ P and assign this rank as the i-th element of the new sequence T (i), where i <= 2(n + j) ;
        // T: list_rank
        std::vector<size_t> T = compute_rank_for_endpoints(S, keyset);

        std::cout << std::endl << "Computed ranks of endpoints: " << std::endl;
        for (double i: T)
            std::cout << i << ' ';
        std::cout << std::endl;


        // Compute the effect of choosing S(1) as a poisoning key and inserting it to K ∪ P with the appropriate rank adjustments.
        // Specifically, evaluate the sequences each of which is the mean M for a different variable, e.g., K, R, KR. Compute MK (1), MK2 (1), MKR(1), and L(1) ;

        // TODO
        break;

    }


    std::vector<double> vect;
    vect.push_back(0.0);
    return vect;
}

/*
    Performs poisoning of dataset
*/
std::vector<double> perform_poisoning(std::vector<double> & legit_keys) {
    //std::vector<double> poisoning_keys
    std::cout << "Legitimate keys: " << std::endl;
    for (double i: legit_keys)
        std::cout << i << ' ';

    std::vector<size_t> legit_ranks = tag_sort(legit_keys);

    std::cout << std::endl << "Legitimate ranks: " << std::endl;

    for (size_t i: legit_ranks)
        std::cout << i << ' ';

    obtain_poisoning_keys(0.1, legit_keys, legit_ranks);

    std::sort(legit_keys.begin(), legit_keys.end());
    return legit_keys;

}

