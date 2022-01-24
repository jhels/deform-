#include "NumCpp.hpp"
#include <cstdlib>
#include <iostream>
#include <cmath>

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

    nc::NdArray A = nc::arange<double>(-1,1+step,step);                   //Range of possible amplitude values at chosen bitrate
    nc::uint32 size = nc::size(A);

    nc::NdArray functions = nc::zeros<double>({size,sequence_length+1});  //Rows 0,1,2,... are w, f_1(w), f_2(w), f_3(w),...,f_sequence(w).
    nc::NdArray poly_intervals = nc::zeros<double>(pieces-1);             //stores joining points of polys
    nc::NdArray random_coefficients = nc::zeros<double>({pieces, poly_degree});


    //GENERATE CANDIDATE FUNCTIONS

    functions(functions.rSlice(),0) = A;    //Row 0 is identity function

    nc::NdArray<double> endpoints = {-1,1};
    poly_intervals = nc::sort(nc::append(nc::random::uniform({pieces-1, 1},-1.0, 1.0), endpoints));  //the piecewise interval points

    random_coefficients = nc::random::uniform<double>({pieces,poly_degree}, coefficient_range[0], coefficient_range[1]) ; //random matrix to draw polynomials from

    nc::NdArray final_curve = nc::empty<double>(0, 1);  //final curve will be stored here
    nc::NdArray y = nc::empty<double>(0, 1);

    for (int i = 0; i < pieces; i++){
        int last_piece_pos = 0;
        nc::NdArray x = nc::arange<double>(poly_intervals[i],poly_intervals[i+1], step);
        nc::NdArray ys = nc::zeros<double>(nc::size(x));
        for (int j; j < nc::size(x); j++) {
            nc::polynomial::Poly1d<double> yPoly(random_coefficients[i, j], false);
            // ys[j] = nc::polynomial::Poly1d<double>::Poly1d(random_coefficients[i][j], false).operator(x);
            ys[j] = yPoly::operator(x);
            if (i > 0) {
                ys[j] -= nc::polynomial::Poly1d<double>::Poly1d(random_coefficients[i, j], false).operator(poly_intervals[i]) + y[nc::size(y)-1];  //glues next piece continuously onto previous piece
            }
            y = nc::append(y, ys);
        }
        final_curve = nc::append(final_curve,y);
    }
    if (size(final_curve) > size){
        final_curve = final_curve(nc::Slice(0,size));  //makes sure endpoints match A
    }

    final_curve = nc::subtract(nc::multiply(nc::subtract(final_curve, nc::min(final_curve)), 0.5 * (nc::max(final_curve) - nc::min(final_curve))), 1); //linearly rescales x -> 2(x-min)/(max-min) - 1. So max -> 1, min -> -1, everything else is in between.

    functions(functions.rSlice(),sequence_length) = final_curve;

    for (interp = 0; interp < sequence_length + 1; interp = interp + 1) {
        for (function_domain = 0; function_domain < size; function_domain = function_domain + 1){
            functions[function_domain,interp] = functions[function_domain,0] + interp * (functions[function_domain,sequence_length] - functions[function_domain,0])/sequence_length;
        }
    }

    // with open(f'fns.txt', 'w+') as f:
    //     for i in range(sequence_length):
    //         fn = functions[:,i]
    //         f.write('\t'.join(map(lambda x: '%.9f' % x, list(fn))))
    //         if i < sequence_length - 1:
    //            f.write('\n')

    // Not sure how to do this in C++ sorry, converted the bits I know below and left the rest.

    // with open(f'fns.txt', 'w+') as f:                    <- No with in C++
    //     for (i = 0; i < sequence_length; i = i + 1){
    //         fn = functions(functions.rSlice(),i)
    //         f.std::ostream::write('\t'.join(map(lambda x: '%.9f' % x, list(fn))))        <- need to substitute C++ lambda
    //         if (i < sequence_length - 1){
    //            f.std::ostream::write('\n')
    //         }
    //     }

    for (int i = 0; i < sequence_length; i++) {
        nc::zeros<double>({size}) fn = functions(functions.rSlice(), i);
        // f.std::ostream::write('\t'.join(map(lambda x: '%.9f' % x, list(fn))))      //  <- need to substitute C++ lambda
        for (int j = 0; j < size; j++) {
            std::cout << fn[j];
        }
        if (i < sequence_length - 1) {
           // f.std::ostream::write('\n')
            std::cout << "\n";
        }
    }

    return 0;
}