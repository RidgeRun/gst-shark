#! /usr/bin/octave -qf

[element_name_list, timestamp_mat, time_mat] = load_serie_timestamp_value('scheduling.mat');

if ((1 == length(timestamp_mat)) && (0 == timestamp_mat))
    return
end

tracer.scheduling.timestamp_mat = timestamp_mat;
tracer.scheduling.time_mat = time_mat;
tracer.scheduling.pad_name_list = element_name_list;
