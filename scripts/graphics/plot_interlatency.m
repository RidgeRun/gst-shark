#! /usr/bin/octave -qf

# Configuration
RESULT = 0;
FONTSIZE = 14;
LINEWIDTH = 1;

# Constants
TRUE = 1;
FALSE = 0;

# Open tracer data
fileID = fopen('interlatency.mat');

count = 1;
# Compute How many series need to be created

[timestamp1 count] = fscanf(fileID,'[%s]');
if (0 == count)
    serie_name_list{1} = "";
    timestamp_mat = 0;
    value_mat = 0;
    return
end
[from_pad count] = fscanf(fileID,'%s\"');
[to_pad count] = fscanf(fileID,'%s\"');
[time count] = fscanf(fileID,'%ld');

# Store the source element
src_pad = from_pad;
pad_name_list = {to_pad};
pad_freq_list = 1;
pad_found = FALSE;

while (count == 1)
    [timestamp count] = fscanf(fileID,'[%s]');
    if (count == 0)
        break
    end
    [from_pad count] = fscanf(fileID,'%s\"');
    [to_pad count] = fscanf(fileID,'%s\"');
    [time count] = fscanf(fileID,'%ld');

    pad_name_list_len = length(pad_name_list);
    for list_idx = 1:pad_name_list_len
        if (1 == strcmp(char(pad_name_list{list_idx}),to_pad))
            pad_freq_list(list_idx) = pad_freq_list(list_idx) + 1;
            pad_found = TRUE;
        end
    end
    if (pad_found == FALSE)
        pad_name_list_len = length(pad_name_list) + 1;
        pad_name_list{pad_name_list_len} = to_pad;
        pad_freq_list(pad_name_list_len) = 1;
    end
    pad_found = FALSE;

end

if (RESULT)
    for list_idx = 1:length(pad_name_list)
        printf('%s %d \n',pad_name_list{list_idx},pad_freq_list(list_idx));
    end
end

# Creata matrix to store the data
timestamp_mat = nan(length(pad_name_list),max(pad_freq_list));
time_mat = nan(size(timestamp_mat));

# Move to the beginning of a file
frewind(fileID)
data_mat_idx_list = ones(1,length(pad_name_list));
pad_name_list_len = length(pad_name_list);
count = 1;
while (count == 1)
    [timestamp count] = fscanf(fileID,'[%s]');
    if (count == 0)
        break
    end
    [from_pad count] = fscanf(fileID,'%s\"');
    [to_pad count] = fscanf(fileID,'%s\"');
    [time count] = fscanf(fileID,'%ld');
    # Match the event with a pad name
    for list_idx = 1:pad_name_list_len
        if (1 == strcmp(char(pad_name_list{list_idx}),to_pad))
            [timestamp_array] = sscanf(timestamp,'%d:%d:%f]');
            timestamp_val = timestamp_array(3) + (timestamp_array(2) * 60) + (timestamp_array(1) * 3600);
            timestamp_mat(list_idx,data_mat_idx_list(list_idx)) = timestamp_val;
            time_mat(list_idx,data_mat_idx_list(list_idx)) = time;
            data_mat_idx_list(list_idx) = data_mat_idx_list(list_idx) + 1;
        end
    end
end

tracer.interlatency.timestamp_mat = timestamp_mat;
tracer.interlatency.time_mat = time_mat;
tracer.interlatency.pad_name_list = pad_name_list;

fclose(fileID);
