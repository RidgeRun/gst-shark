
function plot_tracer(tracer,savefig,format,legend_location)
    TRUE=1;
    
    FONTSIZE = 14;
    LINEWIDTH = 1;
    # CPU_USAGE_AVERAGE = 0 Doesn't displays the average CPU usage.
    # CPU_USAGE_AVERAGE = 1 Displays also the average CPU usage.
    # CPU_USAGE_AVERAGE = 2 Displays only the average CPU usage.
    CPU_USAGE_AVERAGE = 1;
    
    # Plot CPU usage
    if (1 == isfield(tracer,'cpuusage'))
    
        figure('Name','CPU usage')

        # Show plot legend
        switch CPU_USAGE_AVERAGE
            case 0
                plot(tracer.cpuusage.timestamp_mat(:,2:end),tracer.cpuusage.cpu_mat(:,2:end),'linewidth',LINEWIDTH)
                legend(str2latex(tracer.cpuusage.cpu_name_list{2:end}),'Location',legend_location)
            case 1
                plot(tracer.cpuusage.timestamp_mat,tracer.cpuusage.cpu_mat,'linewidth',LINEWIDTH)
                legend(str2latex(tracer.cpuusage.cpu_name_list),'Location',legend_location)
            case 2
                plot(tracer.cpuusage.timestamp_mat(1,:),tracer.cpuusage.cpu_mat(1,:),'linewidth',LINEWIDTH)
                legend(str2latex(tracer.cpuusage.cpu_name_list{1}),'Location',legend_location)
           otherwise
              plot(tracer.cpuusage.timestamp_mat,tracer.cpuusage.cpu_mat,'linewidth',LINEWIDTH)
              legend(str2latex(tracer.cpuusage.cpu_name_list),'Location',legend_location)
        end

        # Calculate the greatest time value
        timestamp_max = max(max(tracer.cpuusage.timestamp_mat));

        title('CPU usage','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('Usage (%)','fontsize',FONTSIZE)
        xlim([0,timestamp_max])
        
        if (TRUE == savefig)
            disp('Save cpuusage figure...')
            switch format
                case 'pdf'
                    print tracer -dpdf -append
                case 'png'
                    print('cpuusage','-dpng');
                otherwise
                    printf('octave: WARN: %s is not supported',format)
            end
        end
    end
    
    # Plot framerate
    if (1 == isfield(tracer,'framerate'))
        # Calculate the greatest time value
        timestamp_max = max(max(tracer.framerate.timestamp_mat));

        figure('Name','Frame rate')
        plot(tracer.framerate.timestamp_mat,tracer.framerate.fps_mat,'linewidth',LINEWIDTH)
        title('Frame rate','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('Frame per second','fontsize',FONTSIZE)
        legend(str2latex(tracer.framerate.element_name_list),'Location',legend_location)
        xlim([0,timestamp_max])
        
        if (TRUE == savefig)
            disp('Save framerate figure...')
            switch format
                case 'pdf'
                    print tracer -dpdf -append
                case 'png'
                    print('framerate','-dpng');
                otherwise
                    printf('octave: WARN: %s is not supported',format)
            end
        end
    end
    
    # Plot Interlatency
    if (1 == isfield(tracer,'interlatency'))
        # Calculate the greatest time value
        timestamp_max = max(max(tracer.interlatency.timestamp_mat));
        
        figure('Name','Interlatency')
        plot(tracer.interlatency.timestamp_mat',tracer.interlatency.time_mat','linewidth',LINEWIDTH)
        title('Interlatency','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('time (nanoseconds)','fontsize',FONTSIZE)
        legend(str2latex(tracer.interlatency.pad_name_list),'Location',legend_location)
        xlim([0,timestamp_max])
        
        if (TRUE == savefig)
            disp('Save interlatency figure...')
            switch format
                case 'pdf'
                    print tracer -dpdf -append
                case 'png'
                    print('interlatency','-dpng');
                otherwise
                    printf('octave: WARN: %s is not supported',format)
            end
        end
    end
    
    # Plot Processing time
    if (1 == isfield(tracer,'proctime'))
        # Calculate the greatest time value
        timestamp_max = max(max(tracer.proctime.timestamp_mat));

        figure('Name','Processing time')
        plot(tracer.proctime.timestamp_mat',tracer.proctime.time_mat','linewidth',LINEWIDTH)
        title('Processing time','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('time (nanoseconds)','fontsize',FONTSIZE)
        legend(str2latex(tracer.proctime.element_name_list),'Location',legend_location)
        xlim([0,timestamp_max])
        
        if (TRUE == savefig)
            disp('Save proctime figure...')
            switch format
                case 'pdf'
                    print tracer -dpdf -append
                case 'png'
                    print('proctime','-dpng');
                otherwise
                    printf('octave: WARN: %s is not supported',format)
            end
        end
    end
    
    # Plot Scheduling
    if (1 == isfield(tracer,'scheduling'))
        # Calculate the greatest time value
        timestamp_max = max(max(tracer.scheduling.timestamp_mat));

        figure('Name','Scheduling')
        plot(tracer.scheduling.timestamp_mat',tracer.scheduling.time_mat','linewidth',LINEWIDTH)
        title('Scheduling','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('time (nanoseconds)','fontsize',FONTSIZE)
        legend(str2latex(tracer.scheduling.pad_name_list),'Location',legend_location)
        xlim([0,timestamp_max])
        
        if (TRUE == savefig)
            disp('Save scheduling figure...')
            switch format
                case 'pdf'
                    print tracer -dpdf -append
                case 'png'
                    print('scheduling','-dpng');
                otherwise
                    printf('octave: WARN: %s is not supported',format)
            end
        end
    end
    
    if ((1 == isfield(tracer,'cpuusage')) && (1 == isfield(tracer,'framerate')))
        # Create legend name list 
        legend_list = tracer.framerate.element_name_list;
        legend_list{end+1} = tracer.cpuusage.cpu_name_list{1};

        timestamp_max = max(max(max(tracer.framerate.timestamp_mat)),max(tracer.cpuusage.timestamp_mat(:,1)));

        figure('Name','Frame rate and CPU usage')
        [hAx,hLine1,hLine2] = plotyy(tracer.framerate.timestamp_mat,tracer.framerate.fps_mat,tracer.cpuusage.timestamp_mat(:,1),tracer.cpuusage.cpu_mat(:,1));
        legend(str2latex(legend_list),'Location',legend_location)

        title('Frame rate and CPU usage','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel(hAx(1),'FPS','fontsize',FONTSIZE)
        ylabel(hAx(2),'CPU usage (%)','fontsize',FONTSIZE)
        xlim([0,timestamp_max])

        if (TRUE == savefig)
            disp('Save cpuusage vs framerate figure...')
            switch format
                case 'pdf'
                    print tracer -dpdf -append
                case 'png'
                    print('cpuusage_framerate','-dpng');
                otherwise
                    printf('octave: WARN: %s is not supported',format)
            end
        end
    end
end

# Convert normal string to latex format
function string_list = str2latex(string_list)
    for string_list_idx = 1 : length(string_list)
        string_field = char(string_list{string_list_idx});
        string_field_new = 'x';
        string_field_idx = 1;
        for idx = 1 : length(string_field)
            if (string_field(idx) == '_')
                string_field_new(string_field_idx) = '\';
                string_field_idx = string_field_idx + 1;
            end
            string_field_new(string_field_idx) = string_field(idx);
            string_field_idx = string_field_idx + 1;
        end
        string_list{string_list_idx} = string_field_new;
    end
end
