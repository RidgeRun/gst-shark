

# Configuration
RESULT = 0;
FONTSIZE = 14;
LINEWIDTH = 1;
# CPU_USAGE_AVERAGE = 0 Doesn't displays the average CPU usage.
# CPU_USAGE_AVERAGE = 1 Displays also the average CPU usage.
# CPU_USAGE_AVERAGE = 2 Displays only the average CPU usage.
CPU_USAGE_AVERAGE = 1;

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


if (1 == CPU_USAGE_AVERAGE) || (2 == CPU_USAGE_AVERAGE)
    # Create cpu name list
    cpu_name_list{1} = 'CPU average';
    # Compute the average CPU usage
    value_mat(1,:) = mean(value_mat(2:end,:));
end

if (0 == CPU_USAGE_AVERAGE) || (1 == CPU_USAGE_AVERAGE)
    for cpu_idx = 1 : (cpu_num)
        cpu_name_list{cpu_idx + 1} = sprintf('CPU %d',cpu_idx);
    end
end

figure('Name','CPU usage')


# Show plot legend
switch CPU_USAGE_AVERAGE
    case 0
        plot(timestamp_mat(2:end,:)',value_mat(2:end,:)','linewidth',LINEWIDTH)
        legend(cpu_name_list{2:end})
    case 1
        plot(timestamp_mat',value_mat','linewidth',LINEWIDTH)
        legend(cpu_name_list)
    case 2
        plot(timestamp_mat(1,:)',value_mat(1,:)','linewidth',LINEWIDTH)
        legend(cpu_name_list{1})
   otherwise
      plot(timestamp_mat',value_mat','linewidth',LINEWIDTH)
      legend(cpu_name_list)
end


tracer.cpuusage.timestamp_mat = timestamp_mat;
tracer.cpuusage.cpu = value_mat;
tracer.cpuusage.cpu_name_list = cpu_name_list;

title('CPU usage','fontsize',FONTSIZE)
xlabel('time (seconds)','fontsize',FONTSIZE)
ylabel('Usage (%)','fontsize',FONTSIZE)




