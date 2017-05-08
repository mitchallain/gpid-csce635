/*
 * file: gpid.cpp
 * by: Austin Burch
 * date: 5/8/17 
 * license: MIT
 * contact: ajburch92@gmail.com
 */

#include "gpid.hpp"


//NEURON FUNCTIONS//

neuron::neuron() : weights(0), output(0) {

}

void neuron::create(int input_neurons_count) {
    double sign = -1; //to change sign
    double random; //to get random number
    weights = new double[input_neurons_count];
    // start with all Proportional Neuron type, then adjust in layer structure

    prev_input = 0;
    prev_output = 0;
    printf("input_neurons_count: %d\n", input_neurons_count);
    printf("neuron generated\n");
}

//LAYER FUNCTIONS//

layer::layer() : neurons(0), layer_neuron_count(0), layer_input(0) {

}

void layer::create(int layer_input_ct, int layer_neuron_ct) {
    int i;
    neurons = new neuron*[layer_neuron_ct];
    for (i = 0; i < layer_neuron_ct; i++) {
        neurons[i] = new neuron;
        neurons[i]->create(layer_input_ct);
    }

    layer_input = new double[layer_input_ct];
    layer_neuron_count = layer_neuron_ct;
    layer_input_count = layer_input_ct;

    neurons[0]->type = 'P';
    if (layer_neuron_ct == 3) // adjust gpid layer for integral and derivative terms
    {
        neurons[1]->type = 'I';
        neurons[2]->type = 'D';
    }
    printf("layer_input_count: %d\n", layer_input_count);
    printf("layer_neuron_count: %d\n", layer_neuron_count);
    printf("layer generated\n");
}

void layer::gpid_calculate(double error) {
    int i, j;
    double neuron_input_val;

    neuron_input_val = 0; //store the sum of all values here

    // apply neuron transfer functions
    layer_input[0] = error;
    
    // Proportional
    neuron_input_val = neurons[0]->weights[0] * layer_input[0]; //apply input * weight
    neurons[0]->output = clip(neuron_input_val);
    
    // store previous I output
    neuron_input_val = neurons[1]->weights[0] * layer_input[0]; //apply input * weight
    neurons[1]->output = clip(neurons[1]->prev_output + neuron_input_val);
    neurons[1]->prev_output = neurons[1]->output;
    
    // store previous D input
    neuron_input_val = neurons[2]->weights[0] * layer_input[0]; //apply input * weight
    neurons[2]->output = clip(neuron_input_val - neurons[2]->prev_input);
    neurons[2]->prev_input = neuron_input_val;

    printf("neurons[0]->output: %f, neurons[1]->output: %f, neurons[2]->output: %f\n", neurons[0]->output, neurons[1]->output, neurons[2]->output);
    printf("neuron_input_val: %f\n", neuron_input_val);
    printf("gpid layer calculated\n");
}

void layer::output_calculate() {
    int i, j;
    double neuron_input_val;
    neuron_input_val = 0; //store the sum of all values here
    for (j = 0; j < layer_input_count; j++) // for each neuron input
    {
        neuron_input_val += layer_input[j]; //apply input * weight
    }

    neurons[0]->output = clip(neuron_input_val);
    printf("output_layer->output: %f\n", neurons[i]->output);
    printf("output_layer_calculated\n");
}

double layer::clip(double n) {
    double lower = -1;
    double upper = 1;
    return max(lower, std::min(n, upper));
}

//gpid FUNCTIONS

gpid::gpid() {
    int error_input = 1;
    int output_neurons = 1;
    mem_count = 0;
    gpid_layer.create(error_input, GPID_LAYER_NEURONS);
    output_layer.create(GPID_LAYER_NEURONS, output_neurons);

    // adjust starting weight based off mode input    

    read_gpid_setup();

    //setup header for output file stream
    iter = 0; // should change to elapsed time
    FILE *stream;
    stream = fopen("gpid_log.txt", "w+"); //add current date&time
    fprintf(stream, "%s, %s, %s, %s, %s, %s\n",
            "mem_count", "P_neuron:weight", "I_neuron:weight",
            "D_neuron:weight", "output_neuron:output", "GPID:cost");
    fclose(stream);


    // allocate back propagation vectors
    gradients.resize(GPID_LAYER_NEURONS, 0);

    learning_rates.resize(GPID_LAYER_NEURONS, 0);
    learning_rates[0] = P_LEARNING_RATE;
    learning_rates[1] = I_LEARNING_RATE;
    learning_rates[2] = D_LEARNING_RATE;

    measurement_vec.resize(GPID_MEMORY_SIZE,0);
    P_out_vec.resize(GPID_MEMORY_SIZE, 0);
    I_out_vec.resize(GPID_MEMORY_SIZE, 0);
    D_out_vec.resize(GPID_MEMORY_SIZE, 0);
    error_vec.resize(GPID_MEMORY_SIZE, 0);
    output_vec.resize(GPID_MEMORY_SIZE, 0);
    P_delta_vec.resize(GPID_MEMORY_SIZE, 0);
    I_delta_vec.resize(GPID_MEMORY_SIZE, 0);
    D_delta_vec.resize(GPID_MEMORY_SIZE, 0);
    cost_vec.resize(GPID_MEMORY_SIZE,0);
    
    printf("gpid generated\n\n");
}

void gpid::read_gpid_setup() {
    double P_w, I_w, D_w, o_w;
    fstream myfile("gpid_setup.txt", std::ios_base::in);

    myfile >> train_bool >> P_w >> I_w >> D_w >> o_w;
    gpid_layer.neurons[0]->weights[0] = P_w;
    gpid_layer.neurons[1]->weights[0] = I_w;
    gpid_layer.neurons[2]->weights[0] = D_w;
    output_layer.neurons[0]->weights[0] = o_w;

    printf("%d\t%f\t%f\t%f\t%f\n", train_bool, P_w, I_w, D_w, o_w);
    printf("gpid setup file read\n");
}

double gpid::propagate_net(double error, double measurement) {
    // forward propagation
    gpid_layer.gpid_calculate(error); //calc pid layer outputs
    for (int i = 0; i < gpid_layer.layer_neuron_count; i++) //update neuron input values for layer: pid->output 
    {
        output_layer.layer_input[i] = gpid_layer.neurons[i]->output;
        printf("output_layer:neuron_input %f\t", gpid_layer.neurons[i]->output);
    }
    output_layer.output_calculate();

    // roll vectors and store training values
    
    rotate(measurement_vec.begin(), measurement_vec.begin() + 1, measurement_vec.end());
    measurement_vec[GPID_MEMORY_SIZE - 1] = measurement;
    
    rotate(P_out_vec.begin(), P_out_vec.begin() + 1, P_out_vec.end());
    P_out_vec[GPID_MEMORY_SIZE - 1] = gpid_layer.neurons[0]->output;
    
    rotate(I_out_vec.begin(), I_out_vec.begin() + 1, I_out_vec.end());
    I_out_vec[GPID_MEMORY_SIZE - 1] = gpid_layer.neurons[1]->output;
    
    rotate(D_out_vec.begin(), D_out_vec.begin() + 1, D_out_vec.end());
    D_out_vec[GPID_MEMORY_SIZE - 1] = gpid_layer.neurons[2]->output;
    
    rotate(error_vec.begin(), error_vec.begin() + 1, error_vec.end());
    error_vec[GPID_MEMORY_SIZE - 1] = error;
    
    rotate(output_vec.begin(), output_vec.begin() + 1, output_vec.end());
    output_vec[GPID_MEMORY_SIZE - 1] = output_layer.neurons[0]->output;
    printf("output_layer: neuron_output %f\t", output_layer.neurons[0]->output);
 


    // back propagation
    if (mem_count >= 1) {
        
        // roll and calculate deltas
        rotate(P_delta_vec.begin(), P_delta_vec.begin() + 1, P_delta_vec.end());
        P_delta_vec[GPID_MEMORY_SIZE - 1] = error_vec[GPID_MEMORY_SIZE - 1]
                * (diff_wrap(measurement_vec[GPID_MEMORY_SIZE - 1], measurement_vec[GPID_MEMORY_SIZE - 2])
                * invert(diff_wrap(output_vec[GPID_MEMORY_SIZE - 1], output_vec[GPID_MEMORY_SIZE - 2])))
                * P_out_vec[GPID_MEMORY_SIZE - 1];
        printf("P_delta %f\t", P_delta_vec[GPID_MEMORY_SIZE - 1]);

                rotate(I_delta_vec.begin(), I_delta_vec.begin() + 1, I_delta_vec.end());
                I_delta_vec[GPID_MEMORY_SIZE - 1] = error_vec[GPID_MEMORY_SIZE - 1]
                * (diff_wrap(measurement_vec[GPID_MEMORY_SIZE - 1], measurement_vec[GPID_MEMORY_SIZE - 2])
                * invert(diff_wrap(output_vec[GPID_MEMORY_SIZE - 1], output_vec[GPID_MEMORY_SIZE - 2])))
                * I_out_vec[GPID_MEMORY_SIZE - 1];
        printf("I_delta %f\t", I_delta_vec[GPID_MEMORY_SIZE-1]);

        rotate(D_delta_vec.begin(), D_delta_vec.begin() + 1, D_delta_vec.end());
        D_delta_vec[GPID_MEMORY_SIZE - 1] = error_vec[GPID_MEMORY_SIZE - 1]
                * (diff_wrap(measurement_vec[GPID_MEMORY_SIZE - 1], measurement_vec[GPID_MEMORY_SIZE - 2])
                * invert(diff_wrap(output_vec[GPID_MEMORY_SIZE - 1], output_vec[GPID_MEMORY_SIZE - 2])))
                * D_out_vec[GPID_MEMORY_SIZE - 1];
        printf("D_delta %f\t", D_delta_vec[GPID_MEMORY_SIZE-1]);


        if (mem_count >= (GPID_MEMORY_SIZE - 1)) {
            if (train_bool == 1) {
                // train
                printf("\n");
                gradients[0] = -2.0 * accumulate(P_delta_vec.begin(), P_delta_vec.end(), 0.0) / GPID_MEMORY_SIZE; // calc gradient
                gpid_layer.neurons[0]->weights[0] -= learning_rates[0] * gradients[0]; // store new weights
                printf("gradient[0]: %f \t weight[0]: %f \t delta_sum[0]: %f\n", gradients[0], gpid_layer.neurons[0]->weights[0], accumulate(P_delta_vec.begin(), P_delta_vec.end(),0.0));

                gradients[1] = -2.0 * accumulate(I_delta_vec.begin(), I_delta_vec.end(), 0.0) / GPID_MEMORY_SIZE;
                gpid_layer.neurons[1]->weights[0] -= learning_rates[1] * gradients[1];
                printf("gradient[1]: %f \t weight[1]: %f \t delta_sum[1]: %f\n", gradients[1], gpid_layer.neurons[1]->weights[0], accumulate(I_delta_vec.begin(), I_delta_vec.end(),0.0));

                gradients[2] = -2.0 * accumulate(D_delta_vec.begin(), D_delta_vec.end(), 0.0) / GPID_MEMORY_SIZE;
                gpid_layer.neurons[2]->weights[0] -= learning_rates[2] * gradients[2];
                printf("gradient[2]: %f \t weight[2]: %f \t delta_sum[2]: %f\n", gradients[2], gpid_layer.neurons[2]->weights[0], accumulate(D_delta_vec.begin(), D_delta_vec.end(),0.0));
                printf("TRAINING COMPLETE\n");
            }
            mem_count = 0;

        } else {
            mem_count++;
        }
    } else {

        mem_count++;
    }
    // compute cost
    cost_vec[GPID_MEMORY_SIZE - 1] = pow(error, 2.0);
    printf("cost: %f\t", cost_vec[GPID_MEMORY_SIZE -1]);

    for (int i = 0; i < GPID_MEMORY_SIZE; i++) {
        cost += cost_vec[i] / GPID_MEMORY_SIZE;
    }
    printf("cost_sum: %f\t", cost);
    
    save_data();
    printf("propagation finished\n");
    return output_layer.neurons[0]->output;
}

void gpid::save_data() {
    FILE *stream;
    stream = fopen("gpid_log.txt", "a+");

    fprintf(stream, "%d, %f, %f, %f, %f, %f\n",
            mem_count,
            gpid_layer.neurons[0]->weights[0],
            gpid_layer.neurons[1]->weights[0],
            gpid_layer.neurons[2]->weights[0],
            output_layer.neurons[0]->output,
            cost);
    fclose(stream);
    printf("gpid data saved \t");
}

double gpid::diff_wrap(double a, double b) {
    double ang = a - b;// in rad
    ang = mod_doubles((ang + PI),(2.0 * PI)) - PI;
    return ang;
}

double gpid::invert(double a) {
    if (a != 0) {
        return 1.0 / a;
    } else {
        return 0.0;
    }
}

double gpid::mod_doubles(double a, double b) {
    int c = static_cast<int> (a / b);
    return a - static_cast<double> (c) * b;
}