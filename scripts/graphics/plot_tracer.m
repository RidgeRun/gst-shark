
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
            case 1
                plot(tracer.cpuusage.timestamp_mat,tracer.cpuusage.cpu_mat,'linewidth',LINEWIDTH)
            case 2
                plot(tracer.cpuusage.timestamp_mat(1,:),tracer.cpuusage.cpu_mat(1,:),'linewidth',LINEWIDTH)
           otherwise
              plot(tracer.cpuusage.timestamp_mat,tracer.cpuusage.cpu_mat,'linewidth',LINEWIDTH)
        end

        # Find time value extrema
        timestamp_max = max(max(tracer.cpuusage.timestamp_mat));
        timestamp_min = min(min(tracer.cpuusage.timestamp_mat));

        title('CPU usage','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('Usage (%)','fontsize',FONTSIZE)
        xlim([timestamp_min,timestamp_max])
        if (0 == strcmp(legend_location,'extern'))
            switch CPU_USAGE_AVERAGE
                case 0
                    legend(str2latex(tracer.cpuusage.cpu_name_list{2:end}),'Location',legend_location)
                case 1
                    legend(str2latex(tracer.cpuusage.cpu_name_list),'Location',legend_location)
                case 2
                    legend(str2latex(tracer.cpuusage.cpu_name_list{1}),'Location',legend_location)
                otherwise
                    legend(str2latex(tracer.cpuusage.cpu_name_list),'Location',legend_location)
            end
        end
        
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
        # Create a new figure if the legend location is extern
        if (1 == strcmp(legend_location,'extern'))
            switch CPU_USAGE_AVERAGE
                case 0
                    plot_legend(tracer.cpuusage.cpu_name_list{2:end},'Cpuusage plot legend',savefig,'cpuusage_legend',format)
                case 1
                    plot_legend(tracer.cpuusage.cpu_name_list,'Cpuusage plot legend',savefig,'cpuusage_legend',format)
                case 2
                    plot_legend(tracer.cpuusage.cpu_name_list{1},'Cpuusage plot legend',savefig,'cpuusage_legend',format)
                otherwise
                    plot_legend(tracer.cpuusage.cpu_name_list,'Cpuusage plot legend',savefig,'cpuusage_legend',format)
            end
            
        end
    end
    
    # Plot framerate
    if (1 == isfield(tracer,'framerate'))
        # Find time value extrema
        timestamp_max = max(max(tracer.framerate.timestamp_mat));
        timestamp_min = min(min(tracer.framerate.timestamp_mat));

        figure('Name','Frame rate')
        plot(tracer.framerate.timestamp_mat',tracer.framerate.fps_mat','linewidth',LINEWIDTH)
        title('Frame rate','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('Frame per second','fontsize',FONTSIZE)
        xlim([timestamp_min,timestamp_max])
        if (0 == strcmp(legend_location,'extern'))
            legend(str2latex(tracer.framerate.element_name_list),'Location',legend_location)
        end
        
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
        # Create a new figure if the legend location is extern
        if (1 == strcmp(legend_location,'extern'))
            plot_legend(tracer.framerate.element_name_list,'Framerate plot legend',savefig,'framerate_legend',format)
        end
    end
    
    # Plot Interlatency
    if (1 == isfield(tracer,'interlatency'))
        # Find time value extrema
        timestamp_max = max(max(tracer.interlatency.timestamp_mat));
        timestamp_min = min(min(tracer.interlatency.timestamp_mat));
        
        figure('Name','Interlatency')
        plot(tracer.interlatency.timestamp_mat',tracer.interlatency.time_mat','linewidth',LINEWIDTH)
        title('Interlatency','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('time (nanoseconds)','fontsize',FONTSIZE)
        xlim([timestamp_min,timestamp_max])
        if (0 == strcmp(legend_location,'extern'))
            legend(str2latex(tracer.interlatency.pad_name_list),'Location',legend_location)
        end
        
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
        # Create a new figure if the legend location is extern
        if (1 == strcmp(legend_location,'extern'))
            plot_legend(tracer.interlatency.pad_name_list,'Interlatency plot legend',savefig,'interlatency_legend',format)
        end
    end
    
    # Plot Processing time
    if (1 == isfield(tracer,'proctime'))
        # Find time value extrema
        timestamp_max = max(max(tracer.proctime.timestamp_mat));
        timestamp_min = min(min(tracer.proctime.timestamp_mat));

        figure('Name','Processing time')
        plot(tracer.proctime.timestamp_mat',tracer.proctime.time_mat','linewidth',LINEWIDTH)
        title('Processing time','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('time (nanoseconds)','fontsize',FONTSIZE)
        xlim([timestamp_min,timestamp_max])
        if (0 == strcmp(legend_location,'extern'))
            legend(str2latex(tracer.proctime.element_name_list),'Location',legend_location)
        end
        
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
        # Create a new figure if the legend location is extern
        if (1 == strcmp(legend_location,'extern'))
            plot_legend(tracer.proctime.element_name_list,'Proctime plot legend',savefig,'proctime_legend',format)
        end
    end
    
    # Plot Scheduling
    if (1 == isfield(tracer,'scheduling'))
        # Find time value extrema
        timestamp_max = max(max(tracer.scheduling.timestamp_mat));
        timestamp_min = min(min(tracer.scheduling.timestamp_mat));

        figure('Name','Schedule')
        plot(tracer.scheduling.timestamp_mat',tracer.scheduling.time_mat','linewidth',LINEWIDTH)
        title('Schedule','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('time (nanoseconds)','fontsize',FONTSIZE)
        xlim([timestamp_min,timestamp_max])
        if (0 == strcmp(legend_location,'extern'))
            legend(str2latex(tracer.scheduling.pad_name_list),'Location',legend_location)
        end
        # Save figure
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
        # Create a new figure if the legend location is extern
        if (1 == strcmp(legend_location,'extern'))
            plot_legend(tracer.scheduling.pad_name_list,'Schedule plot legend',savefig,'schedule_legend',format)
        end
    end

    # Plot Bitrate
    if (1 == isfield(tracer,'bitrate'))
        # Calculate the greatest time value
        timestamp_max = max(max(tracer.bitrate.timestamp_mat));
        timestamp_min = min(min(tracer.bitrate.timestamp_mat));

        figure('Name','Bitrate')
        plot(tracer.bitrate.timestamp_mat',tracer.bitrate.bitrate_mat','linewidth',LINEWIDTH)
        title('Bitrate','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('time (nanoseconds)','fontsize',FONTSIZE)
        xlim([timestamp_min,timestamp_max])
        if (0 == strcmp(legend_location,'extern'))
            legend(str2latex(tracer.bitrate.pad_name_list),'Location',legend_location)
        end
        # Save figure
        if (TRUE == savefig)
            disp('Save bitrate figure...')
            switch format
                case 'pdf'
                    print tracer -dpdf -append
                case 'png'
                    print('bitrate','-dpng');
                otherwise
                    printf('octave: WARN: %s is not supported',format)
            end
        end
        # Create a new figure if the legend location is extern
        if (1 == strcmp(legend_location,'extern'))
            plot_legend(tracer.bitrate.pad_name_list,'Bitrate plot legend',savefig,'bitrate_legend',format)
        end
    end

    # Plot Queuelevel
    if (1 == isfield(tracer,'queuelevel'))
        # Calculate the greatest time value
        timestamp_max = max(max(tracer.queuelevel.timestamp_mat));
        timestamp_min = min(min(tracer.queuelevel.timestamp_mat));

        figure('Name','Queuelevel')
        plot(tracer.queuelevel.timestamp_mat',tracer.queuelevel.size_buffers_mat','linewidth',LINEWIDTH)
        title('Queuelevel','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('Number of Buffers','fontsize',FONTSIZE)
        xlim([timestamp_min,timestamp_max])
        if (0 == strcmp(legend_location,'extern'))
            legend(str2latex(tracer.queuelevel.element_name_list),'Location',legend_location)
        end
        # Save figure
        if (TRUE == savefig)
            disp('Save queuelevel figure...')
            switch format
                case 'pdf'
                    print tracer -dpdf -append
                case 'png'
                    print('queuelevel','-dpng');
                otherwise
                    printf('octave: WARN: %s is not supported',format)
            end
        end
        # Create a new figure if the legend location is extern
        if (1 == strcmp(legend_location,'extern'))
            plot_legend(tracer.queuelevel.element_name_list,'Queuelevel plot legend',savefig,'queuelevel_legend',format)
        end
    end

    
    if ((1 == isfield(tracer,'cpuusage')) && (1 == isfield(tracer,'framerate')))
        # Create legend name list 
        legend_list = tracer.framerate.element_name_list;
        legend_list{end+1} = tracer.cpuusage.cpu_name_list{1};

        # Find time value extrema
        timestamp_max = max(max(max(tracer.framerate.timestamp_mat)),max(tracer.cpuusage.timestamp_mat(:,1)));
        timestamp_min = min(min(min(tracer.framerate.timestamp_mat)),min(tracer.cpuusage.timestamp_mat(:,1)));

        figure('Name','Frame rate and CPU usage')
        [hAx,hLine1,hLine2] = plotyy(tracer.framerate.timestamp_mat',tracer.framerate.fps_mat',tracer.cpuusage.timestamp_mat(:,1),tracer.cpuusage.cpu_mat(:,1));
         if (0 == strcmp(legend_location,'extern'))
            legend(str2latex(legend_list),'Location',legend_location)
        end

        title('Frame rate and CPU usage','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel(hAx(1),'FPS','fontsize',FONTSIZE)
        ylabel(hAx(2),'CPU usage (%)','fontsize',FONTSIZE)
        xlim([timestamp_min,timestamp_max])

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
        # Create a new figure if the legend location is extern
        if (1 == strcmp(legend_location,'extern'))
            plot_legend(legend_list,'Frame rate and CPU usage legend',savefig,'cpuusage_framerate_legend',format)
        end
    end

    if ((1 == isfield(tracer,'cpuusage')) && (1 == isfield(tracer,'framerate')))
        # Create legend name list
        legend_list = tracer.framerate.element_name_list;
        legend_list{end+1} = tracer.cpuusage.cpu_name_list{1};
        legend_list{end+1} = tracer.cpuusage.cpu_name_list{2};
        legend_list{end+1} = tracer.cpuusage.cpu_name_list{3};
        legend_list{end+1} = tracer.cpuusage.cpu_name_list{4};

        timestamp_max = max(max(max(max(tracer.framerate.timestamp_mat)),max(tracer.cpuusage.timestamp_mat)));
        timestamp_min = min(min(min(min(tracer.framerate.timestamp_mat)),min(tracer.cpuusage.timestamp_mat)));

        figure('Name','Frame rate and CPU usage (cores)')
        [hAx,hLine1,hLine2] = plotyy(tracer.framerate.timestamp_mat',tracer.framerate.fps_mat',tracer.cpuusage.timestamp_mat,tracer.cpuusage.cpu_mat);
         if (0 == strcmp(legend_location,'extern'))
            legend(str2latex(legend_list),'Location',legend_location)
        end

        title('Frame rate and CPU usage (cores)','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel(hAx(1),'FPS','fontsize',FONTSIZE)
        ylabel(hAx(2),'CPU usage (%)','fontsize',FONTSIZE)
        xlim([timestamp_min,timestamp_max])

        if (TRUE == savefig)
            disp('Save cpuusage (cores) vs framerate figure...')
            switch format
                case 'pdf'
                    print tracer -dpdf -append
                case 'png'
                    print('cpuusage-cores_framerate','-dpng');
                otherwise
                    printf('octave: WARN: %s is not supported',format)
            end
        end
        # Create a new figure if the legend location is extern
        if (1 == strcmp(legend_location,'extern'))
            plot_legend(legend_list,'Frame rate and CPU usage legend',savefig,'cpuusage_framerate_legend',format)
        end
    end

    # Plot Buffer time
    if (1 == isfield(tracer,'buffer'))
        # Calculate the greatest time value
        timestamp_max = max(max(tracer.buffer.timestamp_mat));
        timestamp_min = min(min(tracer.buffer.timestamp_mat));

        figure('Name','Buffer time')
        plot(tracer.buffer.timestamp_mat',tracer.buffer.buffer_mat','linewidth',LINEWIDTH)
        title('Processing time','fontsize',FONTSIZE)
        xlabel('time (seconds)','fontsize',FONTSIZE)
        ylabel('time (nanoseconds)','fontsize',FONTSIZE)
        xlim([timestamp_min,timestamp_max])
        if (0 == strcmp(legend_location,'extern'))
            legend(str2latex(tracer.buffer.element_name_list),'Location',legend_location)
        end

        if (TRUE == savefig)
            disp('Save buffer figure...')
            switch format
                case 'pdf'
                    print tracer -dpdf -append
                case 'png'
                    print('buffer','-dpng');
                otherwise
                    printf('octave: WARN: %s is not supported',format)
            end
        end
        # Create a new figure if the legend location is extern
        if (1 == strcmp(legend_location,'extern'))
            plot_legend(tracer.buffer.element_name_list,'Buffer plot legend',savefig,'buffer_legend',format)
        end
    end

end
