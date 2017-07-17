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
