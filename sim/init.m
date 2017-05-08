% % Initialize network for testing
% clear all
% 
% load('annparam.mat');
% load('param.mat');
% load('input.mat');
% 
% trial_params.PIDNN.alpha = 1e-9;
% trial_params.PIDNN.mem = 1500;

global net
net = py.gradient_tuning.PIDNN();

global mem
mem = py.int(trial_params.PIDNN.mem);

global trainer
trainer = py.gradient_tuning.Trainer(net, mem, trial_params.PIDNN.step);

% Carry over from previous trials
trainer.network.weights = [0.7, 0.01, 1];

