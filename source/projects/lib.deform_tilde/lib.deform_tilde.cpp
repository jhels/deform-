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
    static const int sequence_length = 1000;              // length of function sequence. 0 means identity function only.
    static const int degree = 10;                        // max degree of polynomials, too large might cause clicks. 0 = constant.
    const number coefficient_range[2] = {-10, 10}; // range of polynomial coefficients, too large might cause clicks.
    number poly_intervals[pieces+1];             // stores joining points x1,x2,...,x_pieces of polys
    number coefficients[pieces][degree+1][sequence_length+1]; // stores matrix of poly coefficients, initialised at 0.
    number endpoint_one[pieces-1];
    number endpoint_two[pieces-1];

    number get_random(number a, number b) {
        static std::default_random_engine e;
        static std::uniform_real_distribution<> dis(a, b); // range [-1, 1]
        return dis(e);
    }

    number polyval(number poly_coefficients[], number x, int degree) {
    number output = 0;
    for (int i = 0; i < degree + 1; i++) { 
        output += poly_coefficients[i] * std::pow(x, i);  // a_i * x^i
    }
    return output;

    // a + bx + cx^2 + dx^3 <-> poly_coefficients = [a, b, c, d]
    //
    //if f is the polynomial with coefficients poly_coefficients[], calculate f(x)    
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

            for (int i = 0; i < pieces; i++) {           //sets final row fn to random values in chosen coefficient_range
                for (int j = 0; j < degree + 1; j++) {
                    coefficients[i][j][sequence_length] = get_random(coefficient_range[0], coefficient_range[1]);
                }
            }
                    
            //GLUE POLYNOMIALS TOGETHER

            number endpoint_one[pieces-1];
            number endpoint_two[pieces-1];

            for (int i = 1; i < pieces; i++) { //calculates joining points
                for (int j = 0; j < degree + 1; j++) {
                endpoint_one[i-1] += coefficients[i-1][j][sequence_length] * std::pow(poly_intervals[i], j);  // a_i * x^i
                endpoint_two[i-1] += coefficients[i][j][sequence_length] * std::pow(poly_intervals[i], j);  // a_i * x^i
                }
            }

            for (int i = 1; i < pieces; i++) {
                coefficients[i][degree][sequence_length] += endpoint_one[i-1] - endpoint_two[i-1]; //glues next piece continuously onto previous piece
                }
            
            //NORMALISE FINAL CURVE

            int region = 0; //region of poly_intervals that input falls into. region = 0 means input is in [-1,x1]. region = pieces means input is in [xpieces, 1].
                
            number temp_coefficients[degree+1]; //1d array to hold coefficients, for using polyval

            for (int i = 0; i < degree + 1; i++) { //sets up polyval
                temp_coefficients[i] = coefficients[0][i][sequence_length];
            }
            number maximum = polyval(temp_coefficients,-1, degree);

            number minimum = maximum;
            number sampler;

            for (number j = 0; j <= 10000; j++) {
                sampler = -1.0 + j/10000; // equivalent to np.linspace(-1, 1, num=10000)
                for (int i = 0; i < pieces; i++) {
                    if (sampler < poly_intervals[i+1] && sampler >= poly_intervals[i]) {
                        region = i;
                    }
                }
                for (int i = 0; i < degree + 1; i++) { //sets up polyval
                temp_coefficients[i] = coefficients[region][i][sequence_length];
                }
                maximum = std::max(polyval(temp_coefficients,sampler, degree), maximum);
                minimum = std::min(polyval(temp_coefficients,sampler, degree), minimum);
            }

            for (int i = 0; i < pieces; i++) {
                coefficients[i][degree][sequence_length] -= minimum;
                for (int j = 0; j < degree + 1; j++) {
                    coefficients[i][j][sequence_length] = (coefficients[i][j][sequence_length])*2/(maximum - minimum);
                }
                coefficients[i][degree][sequence_length] -= 1;
            }

            //INTERPOLATE POLYNOMIAL COEFFICIENTS OF MIDDLE CURVES

            for (int interp = 1; interp < sequence_length; interp++) {
                for (int i = 0; i < pieces; i++) {
                    for (int j = 0; j < degree + 1; j++) {
                        coefficients[i][j][interp] = (coefficients[i][j][sequence_length] - coefficients[i][j][0]) * interp / sequence_length + coefficients[i][j][0];
                    }
                }
            }

            //NORMALISE INTERPOLATED CURVES

            for (int interp = 1; interp < sequence_length; interp++) {

                for (int i = 0; i < degree + 1; i++) { //sets up polyval
                    temp_coefficients[i] = coefficients[0][i][interp];
                }
                maximum = polyval(temp_coefficients,-1, degree);

                minimum = maximum;

                for (number j = 0; j <= 10000; j++) {
                    sampler = -1 + j/10000; // equivalent to np.linspace(-1, 1, num=10000)
                    for (int i = 0; i < pieces; i++) {
                        if (sampler < poly_intervals[i+1] && sampler >= poly_intervals[i]) {
                            region = i;
                        }
                    }
                    for (int i = 0; i < degree + 1; i++) { //sets up polyval
                    temp_coefficients[i] = coefficients[region][i][interp];
                    }
                    maximum = std::max(polyval(temp_coefficients,sampler, degree), maximum);
                    minimum = std::min(polyval(temp_coefficients,sampler, degree), minimum);
                }

                for (int i = 0; i < pieces; i++) {
                    coefficients[i][degree][interp] -= minimum;
                    for (int j = 0; j < degree + 1; j++) {
                        coefficients[i][j][interp] = (coefficients[i][j][interp])*2/(maximum - minimum);
                    }
                    coefficients[i][degree][interp] -= 1;
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
        int region = 0; 

        for (int i = 0; i < pieces; i++) {  // sets region based on sample_in value
            if (sample_in >= poly_intervals[i] && sample_in < poly_intervals[i + 1]) {
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

