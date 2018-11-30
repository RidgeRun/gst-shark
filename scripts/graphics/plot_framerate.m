#! /usr/bin/octave -qf

[element_name_list, timestamp_mat, time_mat] = load_serie_timestamp_value('framerate.mat');

if ((1 == length(timestamp_mat)) && (0 == timestamp_mat))
    return
end

tracer.framerate.timestamp_mat = timestamp_mat;
tracer.framerate.fps_mat = time_mat;
tracer.framerate.element_name_list = element_name_list;
