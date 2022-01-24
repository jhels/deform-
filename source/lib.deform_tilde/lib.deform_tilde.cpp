/// @file
/// @ingroup    minexamples
/// @copyright  Copyright 2018 The Min-DevKit Authors. All rights reserved.
/// @license    Use of this source code is governed by the MIT License found in the License.md file.

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>
#include "c74_min.h"
#include "NumCpp.hpp"

using namespace c74::min;

class deform : public object<deform>, public sample_operator<2, 1> {
public:
    MIN_DESCRIPTION { "TODO" };
    MIN_TAGS        { "audio, fx" };
    MIN_AUTHOR      { "us" };
    MIN_RELATED     { "jit.peek~" };

    inlet<>  m_inlet_audio      { this, "(signal) audio" };
    inlet<>  m_inlet_fn         { this, "(signal) function index" };
    outlet<> m_outlet_main      { this, "(signal) Sample value at index", "signal" };

    attribute<number> seed { this, "seed", 0,
        description {"seed"},
        setter { MIN_FUNCTION {
            double in = args[0];
            i_seed = static_cast<int>(in);
            return args;
        }}
//        }},
//        getter { MIN_GETTER_FUNCTION {
//            return i_seed;
//        }}
    };

    // compute fns only when the class is loaded the first time
    message<> dspsetup { this, "dspsetup",
        MIN_FUNCTION {
            nc::uint32 bitrate = pow(2, 10);         //set bitrate here

            pieces = 10;                 //number of different polynomials to stitch together
            nc::uint32 sequence_length = 50;        //length of function sequence. 0 means identity function only.

            float coefficient_range[2] = {-1, 1};    //range of polynomial coefficients, too large might cause clicks
            nc::uint32 poly_degree = 3;             //max degree of polynomials, too large might cause clicks

            //INITIALISE CORE

            step = 2.0/bitrate;
            A = nc::linspace(-1.0, 1.0, bitrate, true);   //range of possible amplitude values at chosen bitrate
            size = nc::size(A);                                  //size of range of the functions
            functions = nc::empty<double>({size, sequence_length+1});  //Rows 0,1,2,... are w, f_1(w), f_2(w), f_3(w),...,f_sequence_length(w).

            //INITIALISE FUNCTION GENERATOR

            nc::NdArray<double> poly_intervals = nc::zeros<double>(pieces-1);                   //stores joining points of polys
            nc::NdArray<double> random_coefficients = nc::zeros<double>({pieces, poly_degree}); //stores matrix of poly coefficients

            nc::NdArray<double> endpoints = {-1, 1+step};    //used to make poly_intervals have endpoints [-1,1]
            poly_intervals = nc::sort(nc::append(nc::random::uniform({pieces-1, 1}, -1.0, 1.0), endpoints));  //piecewise interval points

            std::srand(std::time(nullptr)); // use current time as seed for random generator
            i_seed = 42; // std::rand();
            nc::random::seed(i_seed);
            random_coefficients = nc::random::uniform<double>({pieces, poly_degree}, coefficient_range[0], coefficient_range[1]) ; //random matrix to draw polynomials from

            nc::NdArray<double> final_curve = nc::empty<double>(size, 1);  //final curve will be stored here

            //GENERATE FINAL_CURVE

            nc::NdArray<double> y = nc::empty<double>(size, 1);
            int current_index = 0;
            int prev_thingy_index = 0;

            for (int i = 0; i < pieces; i++){
                nc::NdArray<double> x = nc::arange(poly_intervals[i], poly_intervals[i+1], step);
                nc::NdArray<double> ys = nc::zeros<double>(nc::size(x));
                nc::polynomial::Poly1d<double> yPoly(random_coefficients(i, random_coefficients.cSlice()), false);
                for (int j = 0; j < nc::size(x); j++) {
                    if (current_index > nc::size(final_curve)){
                        break;      //ensures final_curve has length "size"
                    }

                    final_curve[current_index] = yPoly(x[j]);       //first polynomial is calculated directly

                    if (i > 0) {
                        final_curve[current_index] -= yPoly(poly_intervals[i]) + final_curve[prev_thingy_index];  //glues next piece continuously onto previous piece
                    }
                    current_index++;
                }
                prev_thingy_index = current_index;
            }

            final_curve = 2.0*(final_curve - nc::min(final_curve)[0])/(nc::max(final_curve)[0] - nc::min(final_curve)[0]) - 1.0; //scales curve to have max point 1 and min point -1

            for (int i = 0; i < size; i++){
                functions(i, 0) = A[i];                          //Row 0 is identity function
                functions(i, sequence_length) = final_curve[i];  //Final row is final function
            }

            for (int interp = 0; interp < sequence_length + 1; interp = interp + 1) {       //makes intermediate rows move smoothly from identity to final function
                for (int function_domain = 0; function_domain < size; function_domain = function_domain + 1){
                    functions(function_domain, interp) = functions(function_domain, 0) + interp * (functions(function_domain, sequence_length) - functions(function_domain, 0))/sequence_length;
                }
            }

            cout << "lib.deform~ loaded bro" << endl;

            return {};
        }
    };

    sample operator()(sample samp, sample fn) {
        int fn_index = static_cast<int>(fn);
        fn_index = (fn_index < pieces) || (fn_index >= 0) ? fn_index : 0;
        // nc::NdArray<double> mask = nc::empty<double>({size, 1});
        // for (int i = 0; i < size; i++) {
        //     mask[i] = (A[i] <= x + step/2) && (A[i] > x - step/2);
        // }
        // sample called_val = nc::where(mask, 0, 0);  // Returns the position of the input amplitude in the amplitude array A
        // called_val = nc::where(mask, A, x)  // Returns the position of the input amplitude in the amplitude array A
        // return(*functions(called_val, fn_index));  //Outputs desired value f(x)


        // double samp = 0.31829; //any double in range [-1,1]
        int sample_pos = 0;
        // int inlet_param = 14; //fn(x) as set from Max inlet

        for (int i = 0; i<size; i++){
            if ((A[i] >= samp - step/2.0) && A[i] < samp + step/2.0){
            sample_pos = i;
            }
        }

        double sample_out = functions(sample_pos, fn_index);

        return sample_out;
    }

private:
    int i_seed;
    double step;
    nc::uint32 pieces;
    nc::uint32 size;
    nc::NdArray<double> A;
    nc::NdArray<double> functions;
};


MIN_EXTERNAL(deform);

