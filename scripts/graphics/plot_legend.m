function plot_legend(label_list,fig_title,fig_save,image_name,format)
    figure('Name',fig_title)
    zero_vect = zeros(length(label_list),2);
    plot(zero_vect',zero_vect')
    legend(str2latex(label_list),'Location','north');
    axis off
    title(fig_title,'fontsize',16)
    if (0 != fig_save)
        disp('Save schedule legend...')
        switch format
            case 'pdf'
                print tracer -dpdf -append
            case 'png'
                print(image_name,'-dpng');
            otherwise
                printf('octave: WARN: %s is not supported',format)
        end
    end
end
