/// @file
/// @ingroup    minexamples
/// @copyright  Copyright 2018 The Min-DevKit Authors. All rights reserved.
/// @license    Use of this source code is governed by the MIT License found in the License.md file.

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>
#include "c74_min.h"

using namespace c74::min;

class deform : public object<deform>, public sample_operator<2, 1> {
private:
    // int i_seed;
    static const int pieces = 10;                       // number of different polynomials to stitch together
    static const int sequence_length = 50;              // length of function sequence. 0 means identity function only.
    static const int degree = 5;                        // max degree of polynomials, too large might cause clicks. 0 = constant.
    const number coefficient_range[2] = {-1, 1}; // range of polynomial coefficients, too large might cause clicks.
    number poly_intervals[pieces+1];             // stores joining points x1,x2,...,x_pieces of polys
    number coefficients[pieces][degree+1][sequence_length+1]; // stores matrix of poly coefficients, initialised at 0.

    number get_random(number a, number b) {
        static std::default_random_engine e;
        static std::uniform_real_distribution<> dis(a, b); // range [-1, 1]
        return dis(e);
    }

public:
    MIN_DESCRIPTION { "TODO" };
    MIN_TAGS        { "audio, fx" };
    MIN_AUTHOR      { "us" };
    MIN_RELATED     { "jit.peek~" };

    inlet<>  m_inlet_audio      { this, "(signal) audio" };
    inlet<>  m_inlet_fn         { this, "(signal) function index" };
    outlet<> m_outlet_main      { this, "(signal) Sample value at index", "signal" };

    // attribute<number> seed { this, "seed", 0,
    //     description {"seed"},
    //     setter { MIN_FUNCTION {
    //         number in = args[0];
    //         i_seed = static_cast<int>(in);
    //         return args;
    //     }}
    // };

    // compute fns only when the class is loaded the first time
    message<> dspsetup { this, "dspsetup",
        MIN_FUNCTION {
            // GENERATE COEFFICIENT MATRIX

            poly_intervals[0] = -1.0;
            poly_intervals[pieces] = 1.0;

            for (int i = 1; i < pieces; i++) {
                poly_intervals[i] = get_random(-1.0,1.0);
            }

            std::sort(poly_intervals, poly_intervals + pieces); //makes poly_intervals = [-1, x1, x2, ..., xpieces, 1]

            for (int i = 0; i < pieces; i++) {           //sets first row f0 to [0,1,0,0,0,...,0] -> f0 = x.
                for (int j = 0; j < degree + 1; j++) {
                    coefficients[i][j][0] = 0;
                }
                coefficients[i][1][0] = 1;
            }

            for (int i = 0; i < pieces; i++){           //sets final row fn to random values in chosen coefficient_range
                for (int j = 0; j < degree + 1; j++){
                    coefficients[i][j][sequence_length] = get_random(coefficient_range[0], coefficient_range[1]);
                }
            }

            // RESCALE COEFFICIENT MATRIX (UNFINISHED)

            // We need to find max(|f|) so we can rescale the polynomial to have max magnitude 1.
            // This value is either at endpoints or at turning points.

            // number endpoints_out[2] = {0, 0};
            // number max_abs_f = 0;
            // for (int i = 0; i < degree + 1; i++) { //calculates values of f_sequence_length(-1) and f_sequence_length(1)
            //     endpoints_out[0] += coefficients[0][i][sequence_length] * std::pow(sample_in, i);
            //     endpoints_out[1] += coefficients[pieces - 1][i][sequence_length] * std::pow(sample_in, i);
            //     max_abs_f = std::max(std::abs(endpoints_out[0]),std::abs(endpoints_out[1]));
            // }
            // if (max_abs_f != 1){}

            // If |f(endpoints)| != 1, we need to find turning points in case max(|f|) is achieved elsewhere.
            //
            // See this link for one possible algorithm: https://stackoverflow.com/questions/21367517/algorithm-to-find-the-maximum-minimum-of-a-polynomial-without-graphing

            //     max(|f|) is the max of |f(endpoints)| and |f(turning points)|.
            //     Once found, simply divide the coefficients by max(|f|) to obtain our rescaled function.

            for (int interp = 1; interp < sequence_length; interp++) { //sets midrow coefficients to interpolated values
                for (int i = 0; i < pieces; i++) {
                    for (int j = 0; j < degree + 1; j++) {
                        coefficients[i][j][interp] = (coefficients[i][j][sequence_length] - coefficients[i][j][0]) * interp / sequence_length;
                    }
                }
            }

            return {};
        }
    };

    sample operator()(sample sample_in, sample fn) {
        int fn_index = static_cast<int>(fn);

        if (fn_index < 0) {
            fn_index = 0;
        } else if (fn_index >= sequence_length) {
            fn_index = sequence_length - 1;
        }

        // Region of poly_intervals that sample_in falls into.
        //   region = 0 means sample_in is in [-1,x1].
        //   region = pieces means sample_in is in [xpieces, 1].
        int region; 

        for (int i = 0; i < pieces; i++) {  // sets region based on sample_in value
            if (sample_in < poly_intervals[i+1]) {
                region = i;
            }
        }

        number sample_out = 0;

        for (int i = 0; i < degree + 1; i++) {
            sample_out += coefficients[region][i][fn_index] * std::pow(sample_in, i);  // a_i * x^i
        }

        return sample_out;
    }
};


MIN_EXTERNAL(deform);

