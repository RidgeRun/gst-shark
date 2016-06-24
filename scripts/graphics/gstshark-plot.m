

GSTSHARK_SAVEFIG = 0;
TRUE = 1;

arg_list = argv ();
figs_num = 0;

for i = 1:nargin
  
    tracer_name = char(arg_list{i});

    switch tracer_name
        case 'cpuusage'
            disp('Processing cpusage...')
            plot_cpuusage
            figs_num = figs_num + 1;
        case 'framerate'
            disp('Processing framerate...')
            plot_framerate
            figs_num = figs_num + 1;
        case 'proctime'
            disp('Processing proctime...')
            plot_proctime
            figs_num = figs_num + 1;
        case 'interlatency'
            disp('Processing interlatency...')
            plot_interlatency
            figs_num = figs_num + 1;
        case 'scheduling'
            disp('Processing scheduling...')
            plot_scheduling
            figs_num = figs_num + 1;
        case '--savefig'
            GSTSHARK_SAVEFIG = TRUE;
        otherwise
            if (0 !=length(tracer_name))
                printf('octave: WARN: %s tracer does not exit',tracer_name)
            end
    end
end

plot_tracer(tracer,GSTSHARK_SAVEFIG,'pdf')

printf ("\n")

 
