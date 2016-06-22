

# Configuration
RESULT = 0;

# Constants
TRUE = 1;
FALSE = 0;

# Open tracer data
fileID = fopen('cpuusage.mat');


[timestamp count] = fscanf(fileID,'[%s]');

# Compute how many CPUs has each event
cpu_num = 0;
while (1 == count)
    [cpu_val, count] = fscanf(fileID,'%f,"');
    cpu_num = cpu_num + 1;
end
cpu_num = cpu_num - 1;

# Move to the beginning of a file
frewind(fileID)
# Compute how many evens has the log
event_count = 0;
count = 1;
while (1 == count)
    [char_val, count] = fread(fileID,1,'char');
    if (char_val == '[')
        event_count = event_count + 1;
    end
end


# Creata matrix to store the data
# Use cpu_num + 1 to add the average value
value_mat = nan(cpu_num + 1,event_count);
timestamp_mat = nan(cpu_num + 1,event_count);


frewind(fileID)

cpu_idx = 1;
count = 1;
for event_idx = 1 : event_count

    [timestamp count] = fscanf(fileID,'[%s]');
    if (count == 0)
        break
    end
    [timestamp_array] = sscanf(timestamp,'%d:%d:%f]');
    timestamp_val = timestamp_array(3) + (timestamp_array(2) * 60) + timestamp_array(1);
    timestamp_mat(1:end,event_idx) = timestamp_val;
    
    for cpu_idx = 2 : (cpu_num)
        [cpu_val, count] = fscanf(fileID,'%f,"');
        value_mat(cpu_idx,event_idx) = cpu_val;
    end
    # Store the last cpu value
    [cpu_val, count] = fscanf(fileID,'%f\n"');
    value_mat(cpu_num + 1,event_idx) = cpu_val;
    
end
    
fclose(fileID);


cpu_name_list{1} = 'CPU average';
# Compute the average CPU usage
value_mat(1,:) = mean(value_mat(2:end,:));
for cpu_idx = 1 : (cpu_num)
    cpu_name_list{cpu_idx + 1} = sprintf('CPU %d',cpu_idx);
end

tracer.cpuusage.timestamp_mat = timestamp_mat;
tracer.cpuusage.cpu_mat = value_mat;
tracer.cpuusage.cpu_name_list = cpu_name_list;
