#! /usr/bin/octave -qf

[element_name_list, timestamp_mat, buffer_mat] = load_serie_timestamp_value('buffer.mat');

if ((1 == length(timestamp_mat)) && (0 == timestamp_mat))
    return
end

tracer.buffer.timestamp_mat = timestamp_mat;
tracer.buffer.buffer_mat = buffer_mat;
tracer.buffer.element_name_list = element_name_list;
