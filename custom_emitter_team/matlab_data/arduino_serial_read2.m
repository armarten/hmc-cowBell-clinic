% Replace 'COMx' with your Arduino's serial port

function arduino_serial_read()
    s1 = serialport('/dev/tty.usbserial-0001', 9600);
    s2 = serialport('/dev/tty.usbserial-FT55AYIQ', 19200);
    csv_file = fopen('arduino_output_save/error_empty.csv','w'); % Initialize variable
    
    try
        % Generage a timestamped file name for the csv file
        name_note = 'sensor_cal';
        timedatestamp_messy = string(datetime);
        timedatestamp = regexprep(timedatestamp_messy, '[ :]', '-');
        file_name = append('arduino_output_save/',timedatestamp,"_",name_note);
        file_location = append(file_name,'.csv');
        file_location % print
        % Open a CSV file for writing
        csv_file = fopen(file_location, 'w');
    
        while true
            % Read data from Arduino
            data1 = readline(s1);
            data2 = readline(s2)
            

            data_comb = [data1 data2]
    
            % Print data to console
            disp(data_comb);
    
            % Write data to CSV file
            fprintf(csv_file, '%s\n', data_comb);
            pause(0.001);
        end
    catch exception
        % Close the serial port and CSV file on error or keyboard interrupt
        fclose(s1);  % Close the file
        fclose(s2);  % Close the file

        clear s1;    % Clear the serialport object to close the connection
        clear s2;    % Clear the serialport object to close the connection

        fclose(csv_file);
        rethrow(exception);
    end

