#! /usr/bin/octave -qf

[element_name_list, timestamp_mat, time_mat] = load_serie_timestamp_value('proctime.mat');

if ((1 == length(timestamp_mat)) && (0 == timestamp_mat))
    return
end

tracer.proctime.timestamp_mat = timestamp_mat;
tracer.proctime.time_mat = time_mat;
tracer.proctime.element_name_list = element_name_list;
