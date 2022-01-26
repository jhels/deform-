#include <cstdlib>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>

// EVENTUAL TODO FOR MAX/MSP EXTERNAL

// Want to be able to set the random seed with an attribute/message.

// Want to output final_curve into visual display, so we can see it - could use [multislider], [plot~] or [jit.graph].

// Want to output coefficients into a [multislider] so we can tweak polynomial coefficients
// manually, insert them back into this external and recalculate and redraw final_curve.

// Want to have an easy way we can apply distortion and other effects to final_curve.

double get_random(double a, double b) {
    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(a, b); // range [-1,1]
    return dis(e);
}

double polyval(double poly_coefficients[], double x, int degree) {
    double output = 0;
    for (int i = 0; i < degree + 1; i++) { 
        output += poly_coefficients[i] * std::pow(x, i);  // a_i * x^i
    }
    return output;

    // a + bx + cx^2 + dx^3 <-> poly_coefficients = [a, b, c, d]
    //
    //if f is the polynomial with coefficients poly_coefficients[], calculate f(x)    
}


int main() {

// MAX/MSP INPUT

    int max_param = 34; //input value, must be between 0 and sequence_length
    double sample_in = 0.1523; //test sample input

//CHANGEME

    const int pieces = 10;                 //number of different polynomials to stitch together
    const int sequence_length = 50;        //length of function sequence. 0 means identity function only.

    const double coefficient_range[2] = {-1,1};    //range of polynomial coefficients, too large might cause clicks
    const int degree = 5;             //max degree of polynomials, too large might cause clicks. 0 = constant.

//GENERATE COEFFICIENT MATRIX

    double poly_intervals[pieces+1];       //stores joining points x1,x2,...,x_pieces of polys
    poly_intervals[0] = -1.0;
    poly_intervals[pieces] = 1.0;

    for (int i = 1; i < pieces; i++){
    poly_intervals[i] = get_random(-1.0,1.0);
    }

    std::sort(poly_intervals, poly_intervals + pieces); //makes poly_intervals = [-1, x1, x2, ..., xpieces, 1]

    double coefficients[pieces][degree+1][sequence_length+1];   //stores matrix of poly coefficients, initialised at 0.

    for (int i = 0; i < pieces; i++){           //sets first row f0 to [0,1,0,0,0,...,0] -> f0 = x.
        for (int j = 0; j < degree + 1; j++){
            coefficients[i][j][0] = 0;
        }
        coefficients[i][1][0] = 1; 
    }
    
    for (int i = 0; i < pieces; i++){           //sets final row fn to random values in chosen coefficient_range
        for (int j = 0; j < degree + 1; j++){
            coefficients[i][j][sequence_length] = get_random(coefficient_range[0], coefficient_range[1]);
        }
    }

//GLUE POLYNOMIALS TOGETHER

    double endpoint_one[pieces-1];
    double endpoint_two[pieces-1];

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
        
    double temp_coefficients[degree+1]; //1d array to hold coefficients, for using polyval

    for (int i = 0; i < degree + 1; i++) { //sets up polyval
        temp_coefficients[i] = coefficients[0][i][sequence_length];
    }
    double maximum = polyval(temp_coefficients,-1, degree);

    double minimum = maximum;
    double sampler;

    for (double j = 0; j <= 10000; j++) {
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

        for (double j = 0; j <= 10000; j++) {
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
     

//CALCULATE VALUE FROM INPUT SAMPLE

    region = 0; //region of poly_intervals that input falls into. 

    for (int i = 0; i < pieces; i++) {  //sets region based on sample_in value
        if (sample_in >= poly_intervals[i] && sample_in < poly_intervals[i+1]) {
            region = i;
        }
    }

    double sample_out = 0;

    for (int i = 0; i < degree + 1; i++) { //sets up polyval
        temp_coefficients[i] = coefficients[region][i][max_param];
    }
    sample_out = polyval(temp_coefficients,sample_in, degree);
    
    std::cout << sample_out;
    
    return 0;
   
    

    // // INFINITE LINEAR OVERSAMPLING

    // // Let "prev" be the previous sample and "sample_in" be the current sample
    // //
    // // Using Linear Oversampling technique we set that
    // //
    // // f(sample_in) = int( f(sample_in) - f(prev) ) / (sample_in - prev)     sample_in != prev
    // //                 
    // //               = sample_out       sample_in = prev.
    // //
    // // We need a matrix of coefficients for int(f). We may neglect the constant of integration without issues.
    
    // double prev = 0;
    // double integral_coefficients[pieces][degree+1][sequence_length+1]; 
    // // Note that integral_coefficients[:][0][:] is x^1 coefficients and integral_coefficients[:][degree][:] is x^(degree + 1).

    // // for (int i = 0; i < pieces; i++){ //generates integral coefficients
    // //     for (int j = 0; j < degree + 1; j++){
    // //         for (int k = 0; k < sequence_length + 1; k++){
    // //         integral_coefficients[i][j][k] = coefficients[i][j][k] / (j+1);
    // //     }
    // // }

    // //CALCULATE INTEGRAL VALUE

    // double integral_out = 0;
    // double prev_integral_out = 0;
    
    // for (int i = 0; i < degree + 1; i++) { 
    //     integral_out += integral_coefficients[region][i][max_param] * std::pow(sample_in, i+1);  // a_i * x^(i+1)
    // }
    // if (sample_in != prev) {
    //     sample_out = (integral_out - prev_integral_out)(sample_in - prev);
    // }
    // std::cout << sample_out;
    // prev_integral_out = integral_out;
    // prev = sample_in;
}


