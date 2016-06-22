#! /usr/bin/octave -qf

# Configuration
RESULT = 0;
FONTSIZE = 14;
LINEWIDTH = 1;

# Constants
TRUE = 1;
FALSE = 0;

[element_name_list, timestamp_mat, time_mat] = load_serie_timestamp_value('scheduling.mat');

tracer.scheduling.timestamp_mat = timestamp_mat;
tracer.scheduling.time_mat = time_mat;
tracer.scheduling.pad_name_list = element_name_list;
