%Austin Burch
%4-25-17

%
% 635 AI Robotics: Project
% This file generates the structures for each simulation run
%

num_trials = 300;

% 1:baseline case
for i=1:num_trials %populate baseline data

    sim_params(i).inputs.em2vic_x = (rand() - 0.5)*100; % up to 50 m each axis
    sim_params(i).inputs.em2vic_y = (rand() - 0.5)*100;
    sim_params(i).inputs.theta_i = atan2(sim_params(i).inputs.em2vic_y, sim_params(i).inputs.em2vic_x) ...
        + (rand() - 0.5)*pi; % start within 60 deg of victim
    sim_params(i).inputs.theta_i = rand()*2*pi; % all angles
    sim_params(i).inputs.current_x = (rand() - 0.5)*0.4; % up to 2/3 ft/s
    sim_params(i).inputs.current_y = (rand() - 0.5)*0.4; %
    sim_params(i).inputs.throttle=3; % may vary this later

    sim_params(i).outputs.t=[];
    sim_params(i).outputs.u=[];
    sim_params(i).outputs.wt1=[];
    sim_params(i).outputs.wt2=[];
    sim_params(i).outputs.wt3=[];
    sim_params(i).outputs.r=[];
    sim_params(i).outputs.y=[];
    sim_params(i).outputs.em_x=[];
    sim_params(i).outputs.em_y=[];


    sim_params(i).PIDNN.mem=100; % 40 seconds
    sim_params(i).PIDNN.step=[-1e-2, -1e-4, -1e-1];

    sim_params(i).other.train_bool=1;
    sim_params(i).other.path_type=4;%1:curved1, 2:curved2, 3:curved3, 4:straight
    sim_params(i).other.LOS_radius=10;

end
