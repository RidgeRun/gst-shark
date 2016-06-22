#! /usr/bin/octave -qf

# Configuration
RESULT = 0;
FONTSIZE = 14;
LINEWIDTH = 1;

# Constants
TRUE = 1;
FALSE = 0;

[element_name_list, timestamp_mat, fps_mat] = load_serie_timestamp_value('framerate.mat');

# Calculate the greatest time value
timestamp_max = max(max(timestamp_mat));

figure('Name','Frame rate')
plot(timestamp_mat',fps_mat','linewidth',LINEWIDTH)
title('Frame rate','fontsize',FONTSIZE)
xlabel('time (seconds)','fontsize',FONTSIZE)
ylabel('Frame per second','fontsize',FONTSIZE)
legend(element_name_list)
xlim([0,timestamp_max])

tracer.framerate.timestamp_mat = timestamp_mat;
tracer.framerate.fps_mat = fps_mat;
tracer.framerate.element_name_list = element_name_list;
