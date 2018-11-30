#! /usr/bin/octave -qf

# Configuration
RESULT = 0;
FONTSIZE = 14;
LINEWIDTH = 1;

# Constants
TRUE = 1;
FALSE = 0;

[pad_name_list, timestamp_mat, bitrate_mat] = load_serie_timestamp_value('bitrate.mat');

if ((1 == length(timestamp_mat)) && (0 == timestamp_mat))
    return
end

tracer.bitrate.timestamp_mat = timestamp_mat;
tracer.bitrate.bitrate_mat = bitrate_mat;
tracer.bitrate.pad_name_list = pad_name_list;
