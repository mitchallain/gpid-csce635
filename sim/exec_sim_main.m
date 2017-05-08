clear all; close all; clc
%
% Austin Burch
% Mitchell Allain
% 4-25-17
%
% 635 AI Robotics: Project
% This file runs the structures generation, runs each simulation trial,
% saves data and saves data of interest to the working directory.
%

load('param.mat') % load EMILY dynamic model parameters
run structures_random % generate simulation trial structures
% run structures % structured trials

num_trials = 1;
trial_params = sim_params(1);
init;

format = 'mm-dd-yy_HH-MM-SS';

for i=1:num_trials
    fn1 = sprintf('data/GPID_trial_%03d_out_', i);
    fn2 = sprintf('data/GPID_trial_%03d_meta_', i);
    date = datestr(now,format);
    filename1 = strcat(fn1, date, '.csv');
    filename2 = strcat(fn2, date, '.csv');
    trial_params = sim_params(i);
    sprintf('Victim: %.2f, %.2f\nAngle %.2f\nCurrent: %.2f, %.2f', ...
        trial_params.inputs.em2vic_x, trial_params.inputs.em2vic_y, ...
        trial_params.inputs.theta_i, trial_params.inputs.current_x, ...
        trial_params.inputs.current_y)

    % Need this for better csv writing
    M = [fieldnames(trial_params.other)' fieldnames(trial_params.inputs)' fieldnames(trial_params.PIDNN)'; ...
    struct2cell(trial_params.other)' struct2cell(trial_params.inputs)' struct2cell(trial_params.PIDNN)'];
    save = struct(M{:});

    struct2csv(save,filename2); % trial metadata

    sim('sim_GPID_LOS')
    
    trial_params.outputs.t = tout;
    trial_params.outputs.u = u;
    trial_params.outputs.y = y;
    trial_params.outputs.r = r;
    trial_params.outputs.em_x = em_x;
    trial_params.outputs.em_y = em_y;
    trial_params.outputs.wt1 = permute(wt1, [3, 2, 1]);
    trial_params.outputs.wt2 = permute(wt2, [3, 2, 1]);
    trial_params.outputs.wt3 = permute(wt3, [3, 2, 1]);

    struct2csv(trial_params.outputs,filename1) % trial data
    
%%%%%%%%%%%%%%%%%%%%%%%%% P Trial %%%%%%%%%%%%%%%%%%%%%%%%%%

    fn1 = sprintf('data/P_trial_%03d_out_',i);
    fn2 = sprintf('data/P_trial_%03d_meta_',i);
    filename1 = strcat(fn1, date, '.csv');
    filename2 = strcat(fn2, date, '.csv');

    % Need this for better csv writing
    M = [fieldnames(trial_params.other)' fieldnames(trial_params.inputs)'; ...
        struct2cell(trial_params.other)' struct2cell(trial_params.inputs)'];
    save = struct(M{:});
    
    struct2csv(save,filename2);

%     init;
    sim('sim_P_LOS')
    
    trial_params.outputs.t = tout;
    trial_params.outputs.u = u;
    trial_params.outputs.y = y;
    trial_params.outputs.r = r;
    trial_params.outputs.em_x = em_x;
    trial_params.outputs.em_y = em_y;
    
    rmfield(trial_params.outputs, 'wt1');
    rmfield(trial_params.outputs, 'wt2');
    rmfield(trial_params.outputs, 'wt3');

    struct2csv(trial_params.outputs,filename1)
    
    clear u y r em_x em_y wt1 wt2 wt3
    clear date cost tout wts save filename1 filename2 fn1 fn2 M
    
    intertrial;
end