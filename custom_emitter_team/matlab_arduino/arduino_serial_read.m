% Replace 'COMx' with your Arduino's serial port

function arduino_serial_read()
    s = serialport('/dev/tty.usbserial-0001', 9600);
    csv_file = fopen('arduino_output_save/error_empty.csv','w'); % Initialize variable
    
    try
        % Generage a timestamped file name for the csv file
        name_note = 'cout_vs_serial_esp32';
        timedatestamp_messy = string(datetime);
        timedatestamp = regexprep(timedatestamp_messy, '[ :]', '-');
        file_name = append('arduino_output_save/',timedatestamp,"_",name_note);
        file_location = append(file_name,'.csv');
        file_location % print
        % Open a CSV file for writing
        csv_file = fopen(file_location, 'w');
    
        while true
            % Read data from Arduino
            data = readline(s);
    
            % Print data to console
    %         disp(data);
    
            % Write data to CSV file
            fprintf(csv_file, '%s\n', data);
        end
    catch exception
        % Close the serial port and CSV file on error or keyboard interrupt
        fclose(s);  % Close the file
        clear s;    % Clear the serialport object to close the connection
        fclose(csv_file);
        rethrow(exception);
    end

