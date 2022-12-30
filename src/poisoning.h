#pragma once

#include <set>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>

static inline double computeSquare (double x) { return x*x; }

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

template<typename T>
double calculate_mean(const std::vector<T>& vector)
{
    double sum = std::accumulate(vector.begin(), vector.end(), 0.0);
    double mean = sum / vector.size();
    return mean;
}

/*
    Compute the rank that key S(i) would have if it was inserted in K ∪ P and assign this rank as the i-th element of the new sequence
*/
std::vector<size_t> compute_rank_for_endpoints(std::vector<double> & endpoints, std::vector<double> & keyset){
    std::vector<size_t> computed_rank_for_endpoint;

    for (double endpoint: endpoints){
        std::vector<double> keyset_extended;
        std::copy(keyset.begin(), keyset.end(), std::back_inserter(keyset_extended));
        keyset_extended.push_back(endpoint);

        /*
        std::cout << std::endl << "Extended keyset" << std::endl;
        for (double i: keyset_extended)
            std::cout << i << ' ';
        std::cout << std::endl;
         */

        auto iter = std::lower_bound(keyset_extended.begin(), keyset_extended.end(), endpoint);
        int rank = int(iter - keyset_extended.begin());
        /*
        std::cout << "Rank of endpoint " << endpoint << ": " << rank << std::endl;
        */

        computed_rank_for_endpoint.push_back(rank);
    }
    return computed_rank_for_endpoint;
}

/*
    Extract non-occupied keys for a given sequence of legitimate and poisoning keys
*/
std::vector<double> partition_non_occupied_keys(std::vector<double> & K, std::set<double> & P) {

    std::vector <double> keyset;
    std::merge(K.begin(), K.end(), P.begin(), P.end(), std::back_inserter(keyset));
    /*
    for (double i: keyset)
        std::cout << i << ' ';
    std::cout << std::endl;
    */

    size_t n = keyset.size();
    //std::cout << "Keyset size: " << n << std::endl;


    double lower_bound = keyset[0] - 1;
    double upper_bound = keyset[n-1] + 1;
    /*
    std::cout << "Lower bound: " << lower_bound << std::endl;
    std::cout << "Upper bound: " << upper_bound << std::endl;
    */

    std::vector <double> endpoints;
    bool is_in_sequence = false;

    for(int i = lower_bound; i <= upper_bound +1; i++) {
        if(std::find(keyset.begin(), keyset.end(), i) == keyset.end() and is_in_sequence == false) {
            /* if key i is at start of sequence */
            is_in_sequence = true;
            endpoints.push_back(i);

        } else if(std::find(keyset.begin(), keyset.end(), i) == keyset.end() and is_in_sequence == true and std::find(keyset.begin(), keyset.end(), i+1) != keyset.end()){
            /* if key i is at end of sequence */
            endpoints.push_back(i);
            is_in_sequence = false;
        }
    }
    return endpoints;

}

/*
    Implementation of the greedy poisoning attack on regression models as described in Kornaropoulos et al. ("The Price of Tailoring the Index to Your Data: Poisoning Attacks on Learned Index Structures")
 */
std::set<double> obtain_poisoning_keys(double poisoning_threshold, std::vector<double> & keyset, std::vector<size_t> &rankset) {
    // Total number of elements
    int n = keyset.size();
    std::cout << std::endl << "Number of legitimate keys: " << n << std::endl;

    // Number of poisoning keys P
    int P = int(poisoning_threshold * n);
    std::cout << "Number of poisoning keys to be generated: " << P << std::endl;

    std::set<double> poisoning_keys;

    while(poisoning_keys.size() < P){
        //std::cout << "Current status: " << poisoning_keys.size() << " out of " << P << " poisoning keys generated." << std::endl;

        // Partition the non-occupied keys into subsequences such that each subsequence consists of consecutive non-occupied keys;
        // Extract the endpoints of each subsequence and sort them to construct the new sequence of endpoints S(i), where i <= 2(n + j);

        // S: endpoints
        std::vector<double> S = partition_non_occupied_keys(keyset, poisoning_keys);
        //std::cout << "Length of endpoints: " << S.size() << std::endl;

        // Endpoint keys are continuously increasing
        /*
        std::cout << "Endpoint keys: " << std::endl;
        for (double i: S)
            std::cout << i << ' ';
        */

        // Compute the rank that key S(i) would have if it was inserted in K ∪ P and assign this rank as the i-th element of the new sequence T (i), where i <= 2(n + j) ;
        // T: list_rank
        std::vector<size_t> T = compute_rank_for_endpoints(S, keyset);

        /*
        std::cout << std::endl << "Computed ranks of endpoints: " << std::endl;
        for (double i: T)
            std::cout << i << ' ';
        std::cout << std::endl;
        */

        // Compute the effect of choosing S(1) as a poisoning key and inserting it to K ∪ P with the appropriate rank adjustments.
        // Specifically, evaluate the sequences each of which is the mean M for a different variable, e.g., K, R, KR. Compute MK (1), MK2 (1), MKR(1), and L(1) ;
        std::map<size_t, double> delta_S;
        std::map<size_t, double> M_K;
        std::map<size_t, double> M_K_square;
        std::map<size_t, double> M_R;
        std::map<size_t, double> M_R_square;
        std::map<size_t, double> M_KR;
        std::map<size_t, double> L;

        /*
         Calculate M_K(1), M_R(1) etc.
         Insert first potential poisoning key
        */
        std::vector<double> current_keyset;
        std::copy(keyset.begin(), keyset.end(), std::back_inserter(current_keyset));

        current_keyset.push_back(S[0]);
        M_K[0] = calculate_mean(current_keyset);

        std::vector<double> current_rankset;
        std::copy(rankset.begin(), rankset.end(), std::back_inserter(current_rankset));

        current_keyset.push_back(T[0]);
        M_R[0] = calculate_mean(current_rankset);

        std::vector<double> current_keyset_squared;
        current_keyset_squared = current_keyset;
        std::transform(current_keyset_squared.begin(), current_keyset_squared.end(), current_keyset_squared.begin(), computeSquare);
        M_K_square[0] = calculate_mean(current_keyset_squared);

        std::vector<double> current_rankset_squared;
        current_rankset_squared = current_rankset;
        std::transform(current_rankset_squared.begin(), current_rankset_squared.end(), current_rankset_squared.begin(), computeSquare);
        M_R_square[0] = calculate_mean(current_rankset_squared);

        std::vector<double> current_keyset_rankset;
        std::transform( current_keyset.begin(), current_keyset.end(),
                        current_rankset.begin(), std::back_inserter(current_keyset_rankset),
                        std::multiplies<double>() );

        M_KR[0] = calculate_mean(current_keyset_rankset);

        double nominator = pow( (M_KR[0] - (M_K[0] * M_R[0])), 2);
        double denominator = pow( M_K_square[0] - (M_K[0]), 2);
        L[0] = - (nominator / denominator) + M_R_square[0] - pow(M_R[0], 2);


        for(int i = 1; i < S.size() -1; i++) {
             // Calculate M_K(i), M_R(i) etc.
            delta_S[i] = S[i+1] - S[i] ;

            M_K[i] = M_K[i-1] + (delta_S[i] / n) ;
            /*
            std::cout <<  "M_K_square: " <<  M_K_square[i-1] << std::endl;
            std::cout << "S[i]: " << S[i] << std::endl;
            std::cout << "delta_S[i]: " << delta_S[i] << std::endl;
            */

            M_K_square[i] = M_K_square[i-1] + ( (( 2 * S[i] + delta_S[i]) * delta_S[i]) / (n + 1) );

            M_R[i] = (n + 2) / 2;
            M_R_square[i] = ((n+2)*(2*n+3)) / 6;
            M_KR[i] = M_KR[i-1] + ( T[i-1] * delta_S[i]) / (n + 1);

            nominator = pow( (M_KR[i] - M_K[i]*M_R[i]), 2);
            denominator = pow(M_K_square[i] - (M_K[i]), 2);
            L[i] = - (nominator / denominator) + M_R_square[i] - pow(M_R[i], 2);
        }

        // get argmax of items in L

        int optimal_key_index =  std::distance(L.begin(),std::max_element(L.begin(), L.end()));
        std::cout << "Generated poisoning key: " <<  S[optimal_key_index] << std::endl;
        poisoning_keys.insert(S[optimal_key_index]);
    }
    return poisoning_keys;
}

/*
    Generate poisoning keys based on legitimate keyset (up to specified poisoning threshold)
*/
std::vector<double> perform_poisoning(std::vector<double> & legit_keys, double poisoning_threshold) {


    std::cout << "Legitimate keys: " << std::endl;
    for (double i: legit_keys)
        std::cout << i << ' ';


    std::vector<size_t> legit_ranks = tag_sort(legit_keys);


    std::cout << std::endl << "Legitimate ranks: " << std::endl;

    for (size_t i: legit_ranks)
        std::cout << i << ' ';


    std::set<double> poisoning_keys = obtain_poisoning_keys(poisoning_threshold, legit_keys, legit_ranks);

    /*
     * Merge poisoning keys with legitimate keys to generated poisoned keyset
     */
    std::vector <double> poisoned_keyset;
    std::merge(legit_keys.begin(), legit_keys.end(), poisoning_keys.begin(), poisoning_keys.end(), std::back_inserter(poisoned_keyset));
    std::sort(poisoned_keyset.begin(), poisoned_keyset.end());


    std::cout << "Poisoned keyset: " << std::endl;
    for (double i: poisoned_keyset)
        std::cout << i << ' ';
    std::cout << std::endl;

    return poisoned_keyset;

}

