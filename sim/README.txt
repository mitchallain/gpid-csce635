This folder contains all simulation files necessary for the testing stage of the project.

Running the simulation:
- Open script exec_sim_main.m
- Specify num_trials
- run script exec_sim_main.m



-----------------FILES------------------------------

---------
emily.slx
---------
Simulink library for EMILY simulation. MUST BE in simulation directory. Can be connected to the library browser by executing.
>> set_param(gcs,'EnableLBRepository','on');


---------------
sim_P_LOS.slx
---------------
Line of sight reference with classic P controller


---------------
sim_GPID_LOS.slx
---------------
Line of sight reference with GPID controller


------------
structures.m
------------
Creates test parameter structures


--------------------
structures_random.m
--------------------
Creates random test parameter structures


---------------
exec_sim_main.m
---------------
Execute tests


---------
param.mat
---------
Physical parameter structure, 'param', for EMILY


------
init.m
------
Initializes the neural network.


------------------
intertrial.m
------------------
reset the memory of the GPID controller, 
(should be run between trials)




