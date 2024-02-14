

function T = table_from_csv(file_name)
    % Returns a table with the contents and column titles of file_name.csv
        % in the folder called 'arduino_output_save'
    file_location = append('arduino_output_save/',file_name,'.csv');
    % rdtbl = readtable(file_location);
    dead_rows = get_header_length(file_location); % Number of rows before column headers
    T = readtable(file_location,'NumHeaderLines',dead_rows);  % skips the first n rows of data
return
end