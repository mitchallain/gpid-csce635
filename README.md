# Project 4: Machine Learning for PID Control

This repository contains the supporting files for our CSCE 635 project submission. This includes:
- MATLAB, Python, and C++ code
- Simulink models
- Simulation data
- Jupyter notebooks for data analysis
- Report in LaTeX

![p-v-gpid](/jupyter/p_v_pidnn.gif)

## [C++ Code - `src`](./report)

The souce code was developed for implementation into EMILY's existing platform. The GPID class is a stand alone class that can be utilized for multiple applications. The GPID class can be simply used by generating an object, and then calling the propagate_net method each loop with error and measurement as inputs, respectively. The Control class is an adaptation from the existing Control class to implement the GPID and LOS control algorithms. 

## [MATLAB and Simulink Code - `sim`](/sim)

The simulations were executed with a proof-of-concept implementation of the GPID algorithm in Python, which can be found in the module [`gradient_tuning.py`](/sim/gradient_tuning.py). The two models [`sim_GPID_LOS.slx`](/sim/sim_GPID_LOS.slx) and [`sim_P_LOS.slx`](/sim/sim_P_LOS.slx) contain the simulation dynamics and **must be executed by running** [`exec_sim_main.m`](/sim/).

The simulations require the following MATLAB m-files:
- [`init.m`](/sim/init.m), which initializes the Python network and trainer objects.
- [`intertrial.m`](/sim/intertrial.m), which clears the network memory between trials
- [`structures.m`](/sim/structures.m) or [`structures_random.m`](/sim/structures_random.m), which intializes environment variables and parameters for the trials
- [`update_wrap.m`](/sim/update_wrap.m) which updates the Python network trainer object and logs the states
- [`param.mat`](/sim/param.mat), contains EMILY's physical parameters

The simulations also require the Simulink library [`emily.slx`](/sim/emily.slx), which contains the definitions of the dynamic model and LOS controllers.

## [Jupyter Notebooks - `jupyter`](/jupyter)
The jupyter notebooks serve to document the analysis and the results in a single framework. The following notebooks are included:
- [`analysis-dev.ipynb`](/jupyter/analysis-dev.ipynb), which develops the tools for data analysis
- [`analysis.ipynb`](/jupyter/analysis.ipynb), which analyzes the trial data
- [`net-analysis.ipynb`](/jupyter/net-analysis.ipynb), which studies the properties of the network over many training trials
- [`analysis-final.ipynb`](/jupyter/analysis-final.ipynb), contains comparison of different network parameters and the results of structured trials

## [LaTeX - `report`](/report)
The report `.tex` file and supporting figures.


