

function [serie_name_list,timestamp_mat,value_mat] = load_serie_timestamp_value(file_name)

    # Configuration
    RESULT = 0;
    FONTSIZE = 14;
    LINEWIDTH = 1;

    # Constants
    TRUE = 1;
    FALSE = 0;

    # Open tracer data
    fileID = fopen(file_name);

    count = 1;
    # Compute How many series need to be created

    [timestamp count] = fscanf(fileID,'[%s]');
    if (0 == count)
        serie_name_list{1} = "";
        timestamp_mat = 0;
        value_mat = 0;
        return
    end
    [serie_val count] = fscanf(fileID,'%s\"');
    [time count] = fscanf(fileID,'%ld');

    serie_name_list = {serie_val};
    serie_freq_list = 1;
    serie_found = FALSE;

    while (count == 1)
        [timestamp count] = fscanf(fileID,'[%s]');
        if (count == 0)
            break
        end
        [serie_val count] = fscanf(fileID,'%s\"');
        [time count] = fscanf(fileID,'%ld');

        serie_name_list_len = length(serie_name_list);
        for list_idx = 1:serie_name_list_len
            if (1 == strcmp(char(serie_name_list{list_idx}),serie_val))
                serie_freq_list(list_idx) = serie_freq_list(list_idx) + 1;
                serie_found = TRUE;
            end
        end
        if (serie_found == FALSE)
            serie_name_list_len = length(serie_name_list) + 1;
            serie_name_list{serie_name_list_len} = serie_val;
            serie_freq_list(serie_name_list_len) = 1;
        end
        serie_found = FALSE;
    end

    if (RESULT)
        for list_idx = 1:length(serie_name_list)
            printf('Serie: %s events: %d \n',serie_name_list{list_idx},serie_freq_list(list_idx));
        end
    end

    # Creata matrix to store the data
    timestamp_mat = nan(length(serie_name_list),max(serie_freq_list));
    value_mat = nan(size(timestamp_mat));

    # Move to the beginning of a file
    frewind(fileID)
    data_mat_idx_list = ones(1,length(serie_name_list));
    serie_name_list_len = length(serie_name_list);
    count = 1;
    while (count == 1)
        [timestamp count] = fscanf(fileID,'[%s]');
        if (count == 0)
            break
        end
        [serie_val count] = fscanf(fileID,'%s\"');
        [time count] = fscanf(fileID,'%ld');
        # Match the event with a pad name
        for list_idx = 1:serie_name_list_len
            if (1 == strcmp(char(serie_name_list{list_idx}),serie_val))
                [timestamp_array] = sscanf(timestamp,'%d:%d:%f]');
                timestamp_val = timestamp_array(3) + (timestamp_array(2) * 60) + (timestamp_array(1) * 3600);
                timestamp_mat(list_idx,data_mat_idx_list(list_idx)) = timestamp_val;
                value_mat(list_idx,data_mat_idx_list(list_idx)) = time;
                data_mat_idx_list(list_idx) = data_mat_idx_list(list_idx) + 1;
            end
        end
    end

    fclose(fileID);

end
