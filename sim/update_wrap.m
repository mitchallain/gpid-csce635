function [y] = update_wrap(u)
% Wrapper for the neural net update call to extract attributes
%   for debugging

global trainer
% global net

trainer.rate = [u(3), u(4), u(5)];

y(1) = trainer.update(u(1), u(2));

wt_list = trainer.network.weights.tolist();

y(5) = wt_list{1};
y(6) = wt_list{2};
y(7) = wt_list{3};

samples = py.len(trainer.output_mem{1});

y(2) = py.list(trainer.output_mem{1}){samples} * y(5);
y(3) = py.list(trainer.output_mem{2}){samples} * y(6);
y(4) = py.list(trainer.output_mem{3}){samples} * y(7);

y(8) = trainer.cost;

end

