

GSTSHARK_SAVEFIG = 0;
GSTSHARK_SAVEFIG_FORMAT = 'pdf'
TRUE = 1;

arg_list = argv ();

for i = 1:nargin
  
    option = char(arg_list{i});

    switch option
        case 'cpuusage'
            disp('Processing cpusage...')
            plot_cpuusage
        case 'framerate'
            disp('Processing framerate...')
            plot_framerate
        case 'proctime'
            disp('Processing proctime...')
            plot_proctime
        case 'interlatency'
            disp('Processing interlatency...')
            plot_interlatency
        case 'scheduling'
            disp('Processing scheduling...')
            plot_scheduling
        case '--savefig'
            GSTSHARK_SAVEFIG = TRUE;
        case 'png'
            GSTSHARK_SAVEFIG_FORMAT = 'png';
        case 'pdf'
            GSTSHARK_SAVEFIG_FORMAT = 'pdf';
        otherwise
            if (0 !=length(option))
                printf('octave: WARN: %s tracer does not exit',option)
            end
    end
end

plot_tracer(tracer,GSTSHARK_SAVEFIG,GSTSHARK_SAVEFIG_FORMAT)

printf ("\n")

 
