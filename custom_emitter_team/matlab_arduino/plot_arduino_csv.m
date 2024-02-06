function plot_arduino_csv()

file_name = '05-Feb-2024-15-06-15_test_zeroes'; % Input file name from arduino_output_save folder


file_location = append('arduino_output_save/',file_name,'.csv');
% rdtbl = readtable(file_location);
dead_rows = 4; % Number of rows before column headers
T = readtable(file_location,'NumHeaderLines',dead_rows);  % skips the first n rows of data

end

