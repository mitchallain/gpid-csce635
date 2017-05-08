%Austin Burch
%4-25-17

%
% 635 AI Robotics: Project
% This file generates the structures for each simulation run
%

num_trials = 15;

% 1:baseline case
for i=1:num_trials %populate baseline data

    sim_params(i).inputs.em2vic_x = 40;
    sim_params(i).inputs.em2vic_y = 40;
    sim_params(i).inputs.theta_i = 180*pi/180;
    sim_params(i).inputs.current_x = 0;
    sim_params(i).inputs.current_y = 0;
    sim_params(i).inputs.throttle = 3;

    sim_params(i).outputs.t=[];
    sim_params(i).outputs.u=[];
    sim_params(i).outputs.wt1=[];
    sim_params(i).outputs.wt2=[];
    sim_params(i).outputs.wt3=[];
    sim_params(i).outputs.r=[];
    sim_params(i).outputs.y=[];
    sim_params(i).outputs.em_x=[];
    sim_params(i).outputs.em_y=[];

    sim_params(i).PIDNN.mem=200;
    sim_params(i).PIDNN.step=[-1e-3, -1e-5, -1e-2];
    sim_params(i).PIDNN.train_bool=1;
    
    sim_params(i).other.path_type=4;%1:curved1, 2:curved2, 3:curved3, 4:straight
    sim_params(i).other.LOS_radius=10;
    
    % test matrix
    switch i

        % three cases of inital heading of EMILY
        % case 1 = baseline
        case 2
            sim_params(i).inputs.theta_i = 135*pi/180; %perpendicular to victim
        case 3
            sim_params(i).inputs.theta_i = 225*pi/180; %facing away from victim


        % three cases of inital position of EMILY
        case 4
            sim_params(i).inputs.em2vic_x = 25; %shortest trip
            sim_params(i).inputs.em2vic_y = 25;
        case 5
            sim_params(i).inputs.em2vic_x = 100;
            sim_params(i).inputs.em2vic_y = 100; %longest trip

        % six cases of bounded current
        case 6
            sim_params(i).inputs.current_x = -0.3; %low speed, perpedicular to victim
            sim_params(i).inputs.current_y = 0.3;
        case 7
            sim_params(i).inputs.current_x = -0.3; %low speed, away from victim
            sim_params(i).inputs.current_y = -0.3;
        case 8
            sim_params(i).inputs.current_x = 0.6; %high speed, toward victim
            sim_params(i).inputs.current_y = 0.6;
        case 9
            sim_params(i).inputs.current_x = -0.6; %high speed, perpedicular to victim
            sim_params(i).inputs.current_y = 0.6;
        case 10
            sim_params(i).inputs.current_x = -0.6; %high speed, away from victim
            sim_params(i).inputs.current_y = -0.6;


        % three cases of curved paths
        case 11
            sim_params(i).other.path_type=1; %radius = xx
        case 12
            sim_params(i).other.path_type=2; %radius = xx
        case 13
            sim_params(i).other.path_type=3; %radius = xx

        % two cases of throttle
        case 14
            sim_params(i).inputs.throttle=1.5;
        case 15
            sim_params(i).inputs.throttle=6;

%         % two cases of memory size
%         case 16
%             sim_params(i).PIDNN.mem=50;
%         case 17
%             sim_params(i).PIDNN.mem=400;

%         % two cases of step size
%         case 20
%             sim_params(i).PIDNN.step=0.01;
%         case 21
%             sim_params(i).PIDNN.step=0.001;

%         % training off
%         case 22
%             sim_params(i).other.training_bool=0;

        otherwise
            
    end
end
