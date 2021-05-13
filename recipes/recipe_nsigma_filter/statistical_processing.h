#ifndef STAT_PROCESSING
#define STAT_PROCESSING

#include "Arduino.h"

#include "etl.h"
#include "etl/vector.h"

#include "math.h"

template <typename T>
T sigma_filter(etl::ivector<T> const & vec_in, float n_sigma=3, bool verbose=false){
        static_assert(std::is_signed<T>::value, "signed values only");

    // we first take a coarse mean, in such a way that we cannot overflow
    T mean_coarse {0};

    for (size_t index=0; index<vec_in.size(); index++){
            mean_coarse += vec_in[index] / static_cast<T>(vec_in.size());
    }

    if (verbose){
          Serial.print(F("mean_coarse: ")); Serial.println(mean_coarse);
    }

    // then we need to compute the mean and mean of square when we have removed the coarse mean
    T mean{0};
    T mean_of_squares {0};

    for (size_t index=0; index<vec_in.size(); index++){
            mean += vec_in[index] - mean_coarse;
        mean_of_squares += (vec_in[index] - mean_coarse) * (vec_in[index] - mean_coarse);
    }

    mean /= vec_in.size();
    mean_of_squares /= vec_in.size();

    if (verbose){
          Serial.print(F("mean (without the coarse part): ")); Serial.println(mean);
          Serial.print(F("mean_of_squares (witout the coarse part): ")); Serial.println(mean_of_squares);
    }

    // now we can compute the std estimate 
    T std_estimate {0};

    std_estimate = static_cast<T>(std::sqrt(static_cast<double>(
            mean_of_squares - mean * mean
    )));
    
    if (verbose){
          Serial.print(F("std_estimate: ")); Serial.println(std_estimate);
    }

    // now compute the filtered mean, with a n_sigma filter

    // just the number of sigmas we want to use, as a T type
    T n_sigma_native = static_cast<T>(n_sigma);

    if ((n_sigma_native == 0) || (std_estimate == 0)){
          return (mean + mean_coarse);
    }

    // compute the filtered mean
    T filtered{0};
    int nbr_valid {0};

    for (size_t index=0; index<vec_in.size(); index++){
            T crrt_entry = vec_in[index] - mean_coarse;
        if (verbose){
              Serial.print(F("crrt_entry: ")); Serial.println(crrt_entry);
        }

        // do we have a valid value?
        if (crrt_entry - mean <= n_sigma_native * std_estimate){
                
            if (verbose){
                  Serial.println(F("we use it"));
            }
            nbr_valid += 1;
            filtered += crrt_entry;
        }

        if (verbose){
              Serial.print(F("filtered non normalized: ")); Serial.println(filtered);
        }

    }

    if (verbose){
          Serial.print(F("nbr_valid: ")); Serial.println(nbr_valid);
    }

    if (nbr_valid == 0){
      return (mean + mean_coarse);
    }

    filtered = filtered / nbr_valid;
    filtered += mean_coarse;

    return filtered;
}

#endif
