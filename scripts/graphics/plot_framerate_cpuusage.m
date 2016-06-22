


# Create legend name list 
legend_list = tracer.framerate.element_name_list;
legend_list{end+1} = tracer.cpuusage.cpu_name_list{1};

figure('Name','Frame rate and CPU usage')
[hAx,hLine1,hLine2] = plotyy(tracer.framerate.timestamp_mat',tracer.framerate.fps_mat',tracer.cpuusage.timestamp_mat(1,:),tracer.cpuusage.cpu(1,:));
legend(legend_list)

title('Frame rate and CPU usage','fontsize',FONTSIZE)
xlabel('time (seconds)','fontsize',FONTSIZE)
ylabel(hAx(1),'FPS','fontsize',FONTSIZE)
ylabel(hAx(2),'CPU usage (%)','fontsize',FONTSIZE)

