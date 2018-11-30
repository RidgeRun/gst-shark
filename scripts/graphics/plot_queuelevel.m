#! /usr/bin/octave -qf

[element_name_list, timestamp_mat, size_buffers_mat] = load_serie_timestamp_value('queuelevel.mat');

if ((1 == length(timestamp_mat)) && (0 == timestamp_mat))
    return
end

tracer.queuelevel.timestamp_mat = timestamp_mat;
tracer.queuelevel.size_buffers_mat = size_buffers_mat;
tracer.queuelevel.element_name_list = element_name_list;
