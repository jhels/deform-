#include "NumCpp.hpp"
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>

int main() {

    //CHANGEME

    nc::uint32 bitrate = pow(2,10);         //set bitrate here

    nc::uint32 pieces = 10;                 //number of different polynomials to stitch together
    nc::uint32 sequence_length = 50;        //length of function sequence. 0 means identity function only.

    float coefficient_range[2] = {-1,1};    //range of polynomial coefficients, too large might cause clicks
    nc::uint32 poly_degree = 3;             //max degree of polynomials, too large might cause clicks

    //INITIALISE CORE

    double step = 2.0/bitrate;
    nc::NdArray<double> A = nc::linspace(-1.0,1.0,bitrate, true);   //range of possible amplitude values at chosen bitrate
    nc::uint32 size = nc::size(A);                                  //size of range of the functions
    nc::NdArray<double> functions = nc::empty<double>({size,sequence_length+1});  //Rows 0,1,2,... are w, f_1(w), f_2(w), f_3(w),...,f_sequence_length(w).

    //INITIALISE FUNCTION GENERATOR

    nc::NdArray<double> poly_intervals = nc::zeros<double>(pieces-1);                   //stores joining points of polys
    nc::NdArray<double> random_coefficients = nc::zeros<double>({pieces, poly_degree}); //stores matrix of poly coefficients

    nc::NdArray<double> endpoints = {-1,1+step};    //used to make poly_intervals have endpoints [-1,1]
    poly_intervals = nc::sort(nc::append(nc::random::uniform({pieces-1, 1},-1.0, 1.0), endpoints));  //piecewise interval points

    std::srand(std::time(nullptr)); // use current time as seed for random generator
    int seed = std::rand();
    nc::random::seed(seed);
    random_coefficients = nc::random::uniform<double>({pieces,poly_degree}, coefficient_range[0], coefficient_range[1]) ; //random matrix to draw polynomials from

    nc::NdArray<double> final_curve = nc::empty<double>(size, 1);  //final curve will be stored here
    
    //GENERATE FINAL_CURVE
    
    nc::NdArray<double> y = nc::empty<double>(size, 1);
    int current_index = 0;
    int prev_thingy_index = 0;

    for (int i = 0; i < pieces; i++){
        nc::NdArray<double> x = nc::arange(poly_intervals[i],poly_intervals[i+1], step);
        nc::NdArray<double> ys = nc::zeros<double>(nc::size(x));
        nc::polynomial::Poly1d<double> yPoly(random_coefficients[i, random_coefficients.cSlice()], false);
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

    final_curve = final_curve - nc::min(final_curve)[0];                                        //scales curve
    final_curve = final_curve * (0.5 * (nc::max(final_curve)[0] - nc::min(final_curve)[0]));    //to have max point 1
    final_curve = final_curve - 1.0;                                                            //and min point -1

    for (int i = 0; i < size; i++){                             
        functions(i,0) = A[i];                          //Row 0 is identity function
        functions(i,sequence_length) = final_curve[i];  //Final row is final function
    }    

    for (int interp = 0; interp < sequence_length + 1; interp = interp + 1) {       //makes intermediate rows move smoothly from identity to final function
        for (int function_domain = 0; function_domain < size; function_domain = function_domain + 1){
            functions(function_domain,interp) = functions(function_domain,0) + interp * (functions(function_domain,sequence_length) - functions(function_domain,0))/sequence_length;
        }
    }

    for (int i = 0; i < sequence_length; i++) {         //outputs functions matrix to console
        nc::NdArray<double> fn = functions(functions.rSlice(), i);
        for (int j = 0; j < size; j++) {
            std::cout << fn[j];
            std::cout << "\t";
        }
        if (i < sequence_length - 1) {
            std::cout << "\n";
        }
    }
    return 0;
}