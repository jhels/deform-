/// @file       Audio DSP waveshaper external for Max/MSP. Uses a stochastically
///             generated spline to generate its transfer function.
/// @ingroup    minexamples
/// @copyright  Copyright 2018 The Min-DevKit Authors. All rights reserved.
/// @license    Use of this source code is governed by the MIT License found in the License.md file.

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>
#include "c74_min.h"

using namespace c74::min;   // Max/MSP external

/**
 * Class in format required by Max/MSP (c74_min library: https://github.com/Cycling74/min-devkit)
 */
class deform : public object<deform>, public sample_operator<2, 1> {

private:

    static const int pieces = 10;
    // number of different polynomials to stitch together

    static const int seq_length = 127;
    // length of function sequence. 0 means identity function only

    static const int degree = 10;
    // max degree of polynomials, too large might cause clicks. 0 = constant.

    const number coefficient_range[2] = {-10, 10};
    // range of polynomial coefficients, too large might cause clicks

    number poly_intervals[pieces+1];
    // stores joining points x1,x2,...,x_pieces of polys

    number coefficients[pieces][degree+1][seq_length+1];
    // stores matrix of polys. a + bx + cx^2 + dx^3 <-> poly_coefficients = [a, b, c, d]

    number endpoint_one[pieces-1]; // stores the endpoints of
    number endpoint_two[pieces-1]; // each piecewise polynomial

    /**
     * Generates random numbers in specified
     * range, using the uniform distribution.
     *
     * @param a lower bound for numbers
     * @param b upper bound for numbers
     *
     * @return : random number in range [a,b]
     */
    number get_random(number a, number b) {
        if (seed = 0) {
            seed = rand(); // if seed = 0, randomly generate coefficients
        }
        static std::default_random_engine e(seed);
        static std::uniform_real_distribution<> dis(a, b); // source numbers from range [a, b]
        return dis(e);
    }

    /**
     * Calculates f(x) where f(x) is a polynomial with coefficients poly_coefficients[].
     *
     * @param poly_coefficients coefficients of the polynomial. e.g. [1, 2, 3] -> 1 + 2x + 3x^2.
     * @param x number to evaluate f(x) at
     * @param degree degree of the given polynomial
     *
     * @return : the value of f(x) at the point x
     */
    number poly_eval(number poly_coefficients[], number x, int degree) {
        number output = 0;
        for (int i = 0; i < degree + 1; i++) {
            output += poly_coefficients[i] * std::pow(x, i);  // a_i * x^i
        }
        return output;
    }

    /**
     * Populates poly_intervals[] with a series of endpoints for each polynomial, in order.
     */
    void populate_poly_intervals() {
        poly_intervals[0] = -1.0;
        poly_intervals[pieces] = 1.0;

        for (int i = 1; i < pieces; i++) {
            poly_intervals[i] = get_random(-1.0,1.0);
            // Sets poly_intervals as [-1, r1, r2, ..., r_pieces, 1] where -1 < r_i < 1
        }

        std::sort(poly_intervals, poly_intervals + pieces);
        // sets poly_intervals = [-1, x1, x2, ..., x_pieces, 1] with -1 < x1 < x2, ..., x_pieces < 1
    }

    /**
     * For each piecewise polynomial f^i, sets the value of f^i_0
     *  and f^i_degree by populating coefficients[i][][] as follows:
     *
     * [[0, 1, 0, 0, 0, ..., 0]         ->     f_0(x) = x
     *  [0, 0, 0, 0, 0, ..., 0]
     *              ⋮
     *  [0, 0, 0, 0, 0, ..., 0]
     *  [r0,r1,r2,r3,r4,...,r_degree]]  ->     f_degree(x) = r_0 + r_1 x + ... + r_degree x^degree
     *
     *  where the r_i are random values in the range coefficient_range.
     */
    void initialise_coefficients() {

        for (int i = 0; i < pieces; i++) {           // set first row f0 to [0,1,0,0,0,...,0] (so f_0(x) = x
            for (int j = 0; j < degree + 1; j++) {
                coefficients[i][j][0] = 0;
            }
            coefficients[i][1][0] = 1;
        }

        for (int i = 0; i < pieces; i++) {           // sets final row fn to random values in chosen coefficient_range
            for (int j = 0; j < degree + 1; j++) {
                coefficients[i][j][seq_length] = get_random(coefficient_range[0], coefficient_range[1]);
            }
        }
    }

    /**
     * For each piecewise polynomial f^i_degree,  adjusts its coefficients
     * to ensure that the spline which results from joining all the piecewise
     * polynomials together is continuous.
     *
     * To be run after initialise_coefficients().
     */
    void glue_polys() {
        number endpoint_one[pieces-1];
        number endpoint_two[pieces-1];

        for (int i = 1; i < pieces; i++) { // calculates joining points
            for (int j = 0; j < degree + 1; j++) {
                endpoint_one[i-1] += coefficients[i-1][j][seq_length] * std::pow(poly_intervals[i], j); // a_i * x^i
                endpoint_two[i-1] += coefficients[i][j][seq_length] * std::pow(poly_intervals[i], j);   // a_i * x^i
            }
        }

        for (int i = 1; i < pieces; i++) {
            coefficients[i][degree][seq_length] += endpoint_one[i-1] - endpoint_two[i-1];
            // glues next piece continuously onto previous piece
        }
    }

    /**
     * Adjusts coefficients of each piecewise polynomial f^i_degree
     * so that spline which results from joining all the piecewise
     * polynomials together is in the range [-1, 1].
     *
     * Does this by estimating the maximum and minimum of each polynomial via
     * discrete sampling, then adjusting the range accordingly.
     *
     * To normalise the final spline, run after glue_polys().
     * To normalise the middle splines, run after initialise_middle_curves().
     *
     * @param curve curve to normalise. For example, normalise_curve(seq_length) will
     *              normalise the final spline f_seq_length(x). normalise_curve(1) will
     *              normalise the first middle spline after f_0(x) = x
     */

    void normalise_curve(int curve) {

        int region = 0;
        // region of poly_intervals that input falls into.
        // region = 0 means input is in [-1,x1].
        // region = pieces means input is in [x_pieces, 1].

        number temp_coefficients[degree+1]; // array to hold coefficients of f^i_degree, while
                                            // we normalise it. So we can use poly_eval().

        number maximum = poly_eval(temp_coefficients,-1, degree);   // store maximum and minimum
        number minimum = maximum;                                   // of each polynomial

        number sampler; // samples small intervals across [-1,1] to estimate the minimum and maximum

        for (number j = 0; j <= 20000; j++) {
            sampler = -1.0 + j/10000; // sample for x in [-1, -1 + 1/10000, -1 + 2/10000, ..., 1 - 1/10000, 1]

            // Extremely inefficient way of determining which interval sampler belongs to.
            // For the default value pieces = 10, it doesn't slow things down too much.
            // However,  e.g. binary search could be used instead to speed things up, if needed.
            for (int i = 0; i < pieces; i++) {
                if (sampler < poly_intervals[i+1] && sampler >= poly_intervals[i]) {
                    region = i;
                }
            }

            for (int i = 0; i < degree + 1; i++) { // Load the polynomial belonging to the region sampler is in.
                temp_coefficients[i] = coefficients[region][i][seq_length];
            }
            maximum = std::max(poly_eval(temp_coefficients,sampler, degree), maximum); // Check if the sampled value is
            minimum = std::min(poly_eval(temp_coefficients,sampler, degree), minimum); // a maximum or minimum so far.
        }

        // Having determined the max and min, rescale spline so that min = -1 and max = 1.
        for (int i = 0; i < pieces; i++) {
            coefficients[i][degree][seq_length] -= minimum; // Sets min = 0 temporarily.
            for (int j = 0; j < degree + 1; j++) {
                coefficients[i][j][seq_length] = (coefficients[i][j][seq_length]) * 2 / (maximum - minimum);
                // Rescales whole spline so that min = 0 and max = 2 temporarily
            }
            coefficients[i][degree][seq_length] -= 1; // Shift whole spline again so min = -1 and max = 1.
        }
    }

    /**
     * Sets values for the curves between the effect being off (corresponding to f_0(x) = x)
     * and the final spline f_seq_length(x). There are a total of seq_length - 1 of these curves
     * (default = 126).
     *
     * Does this by linearly interpolating between coefficients of f_0(x) = x
     * and the coefficients of each piecewise polynomial of f^i_pieces(x).
     */
    void initialise_middle_splines() {
        for (int interp = 1; interp < seq_length; interp++) { // middle splines are in range [1, seq_length - 1]

            for (int i = 0; i < pieces; i++) {
                for (int j = 0; j < degree + 1; j++) {
                    coefficients[i][j][interp] = (coefficients[i][j][seq_length] -
                            coefficients[i][j][0]) * interp / seq_length + coefficients[i][j][0];
                    // linear interpolation between f_0 and f_seq_length
                }
            }
        }
    }


/**
 * Defines the object as seen by Max/MSP.
 */
public:
    MIN_DESCRIPTION { "TODO" };
    MIN_TAGS        { "audio, fx" };
    MIN_AUTHOR      { "us" };
    MIN_RELATED     { "jit.peek~" };

    inlet<>  m_inlet_audio      { this, "(signal) audio" };
    inlet<>  m_inlet_fn         { this, "(signal) function index (0-127)" };
    outlet<> m_outlet_main      { this, "(signal) Sample value at index", "signal" };
    // Inlet 1: audio to deform
    // Inlet 2: deformation amount (0-127)
    // Outlet : deformed audio

    /**
     * Sets random seed.
     */
    attribute<number> seed{ this, "seed", 0,
                            description {"seed"},
                            setter { MIN_FUNCTION {
                                    number in = args[0];
                                    int i_seed = static_cast<int>(in);
                                    return args;
                            }}
    };

    /**
     * Contains functions which are only computed
     * when the object is initially loaded.
     */
    message<> dspsetup { this, "dspsetup",
        MIN_FUNCTION {

            populate_poly_intervals();      // set intervals of each piecewise polynomial

            initialise_coefficients();      // set coefficients of each piecewise polynomial

            glue_polys();                   // adjusts coefficients so the resultant spline,
                                            // f_seq_length(x), is continuous

            normalise_curve(seq_length);    // adjusts coefficients of spline to be in range [-1, 1]

            initalise_middle_splines();     // sets appropriate values for the splines between
                                            // f_0 = x and the final spline f_seq_length, using
                                            // linear interpolation

             for (int interp = 1; interp < seq_length; interp++) {
                 normalise_curve(interp);   // adjusts coefficients of middle splines to be in range [-1,1].
             }

             return {};
         }
    };

    /**
     * Calculate the appropriate, deformed output, given an input sample
     * and a value fn selecting which curve to use. Linearly interpolates between floating-point fn
     * values, allowing for a smooth sound when changing continuously from e.g. 0 to 1.
     *
     *
     * @param sample_in the audio sample to deform
     * @param fn which curve to use. fn = 0 means no deformation, fn = seq_length means maximal deformation.
     *           Accepts floats and changing continuously will result in smooth audio, no clicks.
     *
     * @return : the deformed audio sample
     */
    sample operator()(sample sample_in, sample fn) {
        int fn_index = floor(fn);                   // Apply linear interpolation to float index
        number index_interp = 1 - std::fmod(fn, 1); //

        // clip to range [0, seq_length]
        if (fn_index < 0) {
            fn_index = 0;
        } else if (fn_index > seq_length) {
            fn_index = seq_length;
        }

        int region = 0;
        // Region of poly_intervals that sample_in falls into.
        // region = 0 means sample_in is in [-1,x1].
        // region = pieces means sample_in is in [x_pieces, 1].


        // While acceptable in normalise_curve, this method is not efficient here.
        // A better method may result in large performance gains.
        for (int i = 0; i < pieces; i++) {
            if (sample_in >= poly_intervals[i] && sample_in < poly_intervals[i + 1]) {
                // sets region based on sample_in value
                region = i;
            }
        }

        number sample_out = 0;

        // calculate linearly interpolated output value using appropriate polynomial
        for (int i = 0; i < degree + 1; i++) {
            sample_out += index_interp * coefficients[region][i][fn_index] * std::pow(sample_in, i)
                    + (1 - index_interp) * coefficients[region][i][fn_index+1] * std::pow(sample_in, i);
        }

        return sample_out;
    }
};


MIN_EXTERNAL(deform);
