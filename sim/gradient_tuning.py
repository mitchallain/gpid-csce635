#! /usr/bin/env python

##########################################################################################
# neural.py
#
# Test some classes for PIDNN
#
# NOTE:
#
# Created: April 19, 2017
#   - Mitchell Allain
#   - allain.mitch@gmail.com
#
# Modified:
#   * April 29, 2017 - more efficient weight training
#
##########################################################################################

import numpy as np
from collections import deque
import pickle
import pdb
import logging

# LOG
logging.basicConfig(filename='GTNN.log', level=logging.DEBUG)


class PIDNN():
    ''' Modified PIDNN to support heading angle computation

    TODO:

    Args:
        alpha (float): training rate

    Attributes:

    Methods:
        compute_output(inputs): computes output of network with current gains
        load_weights(file): loads network weights from a file
        backprop(error): backpropagates error to train network weights
        '''
    def __init__(self, filename=''):
        self.n0 = Pneuron(0, 0)
        self.n1 = Ineuron(0, 1)
        self.n2 = Dneuron(0, 2)
        self.n3 = Pneuron(1, 3)

        # Bit of a shame to fix the network structure, but quicker
        self.neurons = [self.n0, self.n1, self.n2, self.n3]

        if filename != '':
            with open(filename, 'rb') as openfile:
                self.weights = pickle.load(openfile)
        else:
            self.good_weights()

    def good_weights(self):
        self.weights = np.array([0.01, 0.001, 1])

    def new_weights(self, weights):
        self.weights = weights

    def reset_neuron_states(self):
        for neuron in self.neurons:
            neuron.reset_initial_conditions()

    def compute_output(self, error):
        x0 = self.n0.compute(error)
        x1 = self.n1.compute(error)
        x2 = self.n2.compute(error)

        u3 = self.weights[0] * x0 + self.weights[1] * x1 + self.weights[2] * x2
        x3 = self.n3.compute(u3)

        self.inputs = [error, u3]
        self.outputs = [x0, x1, x2, x3]

        return x3

    def save_weights(self, name):
        with open(name + '.pkl', 'wb') as savefile:
            pickle.dump(self.weights, savefile)


class Pneuron():
    def __init__(self, layer, neuron):
        self.layer = layer
        self.neuron = neuron

    def reset_initial_conditions(self):
        return

    def compute(self, u):
        return np.clip(u, -10, 10)


class Ineuron():
    def __init__(self, layer, neuron):
        self.layer = layer
        self.neuron = neuron
        self.x_prev = 0

    def reset_initial_conditions(self):
        self.x_prev = 0

    def compute(self, u):
        self.x_prev = np.clip(self.x_prev + u, -10, 10)
        return self.x_prev


class Dneuron():
    def __init__(self, layer, neuron):
        self.layer = layer
        self.neuron = neuron
        self.u_prev = 0

    def reset_initial_conditions(self):
        self.u_prev = 0

    def compute(self, u):
        output = np.clip(u - self.u_prev, -10, 10)
        self.u_prev = u
        return output


class Trainer():
    def __init__(self, network, memory=100, rate=[1e-9, 1e-9, 1e-8], savemode=False):
        self.network = network
        self.memory = memory
        self.rate = np.array(rate)
        self.savemode = savemode

        self.output_mem = []
        self.input_mem = []
        for i in xrange(4):
            self.output_mem.append(deque([], maxlen=memory))
        for i in xrange(2):
            self.input_mem.append(deque([], maxlen=memory))

        # These are redundant but convenient
        self.error_mem = deque([], maxlen=memory)
        self.measurement_mem = deque([], maxlen=memory)
        self.delta_mem = []

        for i in xrange(3):
            self.delta_mem.append(deque([], maxlen=memory))

        self.cost_mem = deque([], maxlen=memory)
        self.cost = 0

    def update(self, error, measurement):
        out = self.network.compute_output(error)
        self.measurement_mem.append(measurement)
        self.error_mem.append(error)

        for i in xrange(4):
            self.output_mem[i].append(self.network.outputs[i])
        for i in xrange(2):
            self.input_mem[i].append(self.network.inputs[i])

        # If queue is full, backpropagate
        self.backprop()

        return out

    def backprop(self):
        ### NOTE: delta here is slightly different than Huallin
        ###       the neuron outputs are lumped in for convenience before summing

        ## UPDATE WEIGHTS FROM HIDDEN LAYER TO OUTPUT LAYER
        for j in xrange(3):
            if len(self.error_mem) > 1:  # more than one sample
                self.delta_mem[j].append((self.error_mem[-1]) * (diff_wrap(self.measurement_mem[-1], self.measurement_mem[-2])
                                         * invert(diff_wrap(self.output_mem[3][-1], self.output_mem[3][-2])))
                                         * self.output_mem[j][-1])

            if len(self.delta_mem[j]) == self.memory:  # ready to train
                # pdb.set_trace()
                delta = - 2.0 * sum(self.delta_mem[j]) / self.memory
                logging.info('Mode 2: Delta weight %i, 5: %.2f' % (j, delta))
                self.network.weights[j] -= self.rate[j] * delta
                logging.info('Updated weight %i, 5: %.2f' % (j, self.network.weights[j]))

        ## COMPUTE COST TO CHECK FOR CONVERGENCE
        self.cost_mem.append(self.error_mem[-1]**2)
        self.cost = sum(self.cost_mem) / self.memory

    def empty_queues(self):
        for i in xrange(4):
            self.output_mem[i].clear()
        for i in xrange(2):
            self.input_mem[i].clear()

        # These are redundant but convenient
        self.measurement_mem.clear()
        self.error_mem.clear()

        for i in xrange(3):
            self.delta_mem[i].clear()
        self.cost_mem.clear


def invert(a):
    if a != 0:
        return 1.0 / a
    return 0.0


def diff_wrap(a1, a2):
    ang = float(a1 - a2)

    ang_wrap = ((ang + np.pi) % (2.0 * np.pi)) - np.pi
    return ang_wrap


def reset(tr):
    tr.empty_queues()
    tr.network.reset_neuron_states()
    tr.network.good_weights()


def construct():
    ''' For debugging, constructs a trainer with a net'''
    nt1 = PIDNN()
    tr1 = Trainer(nt1, 200, [1e-9, 1e-9, 1e-8])
    return tr1


def fill(tr, num=200):
    ''' For debugging, fills the net with samples'''
    a = range(200)
    b = [ai + 200 for ai in a]
    for i in a:
        tr.update(i, b[i])
