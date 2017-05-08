/*
 * file: gpid.hpp
 * by: Austin Burch
 * date: 5/8/17 
 * license: MIT
 * contact: ajburch92@gmail.com
 */

#ifndef GPID_HPP      
#define GPID_HPP          

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <string.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <numeric>

#define PI 3.14159265 	/* pi */
#define GPID_LAYER_NEURONS 3
#define GPID_MEMORY_SIZE 200 // learning memory
#define P_LEARNING_RATE -0.000000001
#define I_LEARNING_RATE -0.000000001
#define D_LEARNING_RATE -0.00000001

using namespace std;

struct neuron
{
    double *weights; // neuron input weights
    double output; //output value
    char type;
    double prev_output;
    double prev_input;
    
    neuron(); //Constructor
    void create(int input_count);//Allocates memory and initializes values
};

struct layer
{
    neuron **neurons;//array of neurons
    int layer_neuron_count;//number of neurons
    double *layer_input;//layer input
    int layer_input_count;//number of layer inputs
    
    layer();//mem and init
    void create(int layer_inputs_count, int layer_neurons_ct);//memory and init
    void gpid_calculate(double error);//applies transfer functions
    void output_calculate();//applies transfer functions
    double clip(double n); //clips the output values
};

class gpid
{
private:
    //allocate layers
    layer output_layer;
    layer gpid_layer;
    int iter;
    double error;
    double measurement;
    int mem_count;
    double cost;
    double delta_sum;
    
    deque<double> learning_rates;
    deque<double> gradients;
    deque<double> measurement_vec;
    deque<double> P_out_vec;
    deque<double> I_out_vec;
    deque<double> D_out_vec;
    deque<double> P_delta_vec;
    deque<double> I_delta_vec;
    deque<double> D_delta_vec;
    deque<double> error_vec;
    deque<double> output_vec;
    deque<double> cost_vec;

    void read_gpid_setup();
    void save_data();
    double invert(double a);
    double diff_wrap(double a, double b);
    double mod_doubles(double a, double b);   
public:
    bool train_bool;
    // reference, measurement, input, and output vectors ~
    gpid();//memory, init, net structure
    double propagate_net(double error, double measurement);
};

#endif /* GPID_HPP */
