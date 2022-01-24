#include "NumCpp.hpp"
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>

int main() {

    //CHANGEME

    float coefficient_range[2] = {-1,1};    //range of polynomial coefficients, large might cause clicks
    nc::uint32 poly_degree = 10;                   //max degree of polynomials, large might cause clicks

    nc::uint32 pieces = 10;                        //number of different polynomials to stitch together

    nc::uint32 sequence_length = 50;               //length of function sequence. 0 means identity function
                                            //only. Graphs will be plotted for sequence_length < 10 only.

    //INITIALISE

    nc::uint32 bitrate = pow(2,10);
    nc::uint32 samplerate = 44100;
    double step = 2.0/bitrate;

    nc::NdArray<double> A = nc::arange<double>(-1,1+step,step);                   //Range of possible amplitude values at chosen bitrate
    nc::uint32 size = nc::size(A);

    // nc::NdArray<double> functions = nc::zeros<double>({size,sequence_length+1});  //Rows 0,1,2,... are w, f_1(w), f_2(w), f_3(w),...,f_sequence(w).
    nc::NdArray<double> poly_intervals = nc::zeros<double>(pieces-1);             //stores joining points of polys
    nc::NdArray<double> random_coefficients = nc::zeros<double>({pieces, poly_degree});

    // auto functions [51] = {}
    std::vector<nc::NdArray<double>> functions(nc::zeros<double>(size), sequence_length+1);

    //GENERATE CANDIDATE FUNCTIONS

    // functions(functions.rSlice(),0) = A;    //Row 0 is identity function
    functions.at(0) = A;

    nc::NdArray<double> endpoints = {-1,1};
    poly_intervals = nc::sort(nc::append(nc::random::uniform({pieces-1, 1},-1.0, 1.0), endpoints));  //the piecewise interval points

    random_coefficients = nc::random::uniform<double>({pieces,poly_degree}, coefficient_range[0], coefficient_range[1]) ; //random matrix to draw polynomials from

    // nc::NdArray<double> final_curve = nc::empty<double>(0, 1);  //final curve will be stored here
    // nc::NdArray<double> y = nc::empty<double>(0, 1);
    nc::NdArray<double> final_curve = nc::empty<double>(size, 1);  //final curve will be stored here
    // nc::NdArray<double> y = nc::empty<double>(size, 1);

    int current_index = 0;
    int prev_thingy_index = 0;
    for (int i = 0; i < pieces; i++){
        int last_piece_pos = 0;
        nc::NdArray<double> x = nc::arange<double>(poly_intervals[i],poly_intervals[i+1], step);
        nc::NdArray<double> ys = nc::zeros<double>(nc::size(x));
        nc::polynomial::Poly1d<double> yPoly(random_coefficients[i, random_coefficients.cSlice()], false);
        for (int j; j < nc::size(x); j++) {
            // ys[j] = yPoly(x[j]);
            final_curve[current_index] = yPoly(x[j]);
            if (i > 0) {
                // ys[j] -= yPoly(poly_intervals[i]) + y[nc::size(y)-1];  //glues next piece continuously onto previous piece
                // final_curve[current_index] -= yPoly(poly_intervals[i]) + y[nc::size(y)-1];  //glues next piece continuously onto previous piece
                // final_curve[current_index] -= yPoly(poly_intervals[i]) + final_curve[poly_intervals[i]];  //glues next piece continuously onto previous piece
                final_curve[current_index] -= yPoly(poly_intervals[i]) + final_curve[prev_thingy_index];  //glues next piece continuously onto previous piece
            }
            // y = nc::append(y, ys);
            current_index++;
        }
        // final_curve = nc::append(final_curve,y);
        prev_thingy_index = current_index;
    }
    if (nc::size(final_curve) > size){
        final_curve = final_curve(nc::Slice(0,size),0);  //makes sure endpoints match A
    }

    // final_curve = nc::subtract(nc::multiply(nc::subtract(final_curve, nc::min(final_curve)), 0.5 * (nc::max(final_curve) - nc::min(final_curve))), 1); //linearly rescales x -> 2(x-min)/(max-min) - 1. So max -> 1, min -> -1, everything else is in between.
    // final_curve = nc::multiply(nc::subtract(final_curve, nc::min(final_curve)), 0.5 * (nc::max(final_curve) - nc::min(final_curve)));

    final_curve = final_curve - nc::min(final_curve)[0];
    final_curve = final_curve * (0.5 * (nc::max(final_curve)[0] - nc::min(final_curve)[0]));
    final_curve = final_curve - 1.0;

    // final_curve = nc::subtract(final_curve, nc::full(size, 1, nc::min(final_curve)));
    // final_curve = final_curve * (0.5 * (nc::max(final_curve) - nc::min(final_curve)));
    // final_curve = final_curve - 1.0;

    // that last nc::subtract() broke everything -- it doesn't let you do nc::subtract(array, 1) because 1 is an integer instead of an array. but when I changed it to nc::subtract(array, nc::NdArray<double> {1}) the error messages changed and got weird.

    // functions(functions.rSlice(),sequence_length) = final_curve;
    functions.at(sequence_length) = final_curve;
    // std::cout << functions(functions.rSlice(),0);

    for (int k = 0; k < size; k++) {
        // std::cout << final_curve[k];
        // std::cout << functions(functions.rSlice(),sequence_length);
        std::cout << functions.at(sequence_length)[k];
        std::cout << '\t';
    }

    for (int interp = 0; interp < sequence_length + 1; interp = interp + 1) {
        for (int function_domain = 0; function_domain < size; function_domain = function_domain + 1){
            functions[function_domain,interp] = functions[function_domain,0] + interp * (functions[function_domain,sequence_length] - functions[function_domain,0])/sequence_length;
        }
    }

    // for (int i = 0; i < sequence_length; i++) {
    //     nc::NdArray<double> fn = functions(functions.rSlice(), i);
    //     for (int j = 0; j < size; j++) {
    //         std::cout << fn[j];
    //     }
    //     if (i < sequence_length - 1) {
    //         std::cout << "\n";
    //     }
    // }

    return 0;
}