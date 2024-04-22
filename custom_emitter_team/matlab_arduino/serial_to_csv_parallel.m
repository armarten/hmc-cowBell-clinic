% https://www.mathworks.com/matlabcentral/answers/339524-how-do-i-read-from-two-serial-ports-in-parallel-on-the-same-machine
% Replace 'COMx' with your Arduino's serial port

function serial_to_csv_parallel()
    parpool("Processes",2)
    spmd
    if spmdIndex == 1
        portID = '/dev/tty.usbserial-0001'; % Replace with correct port
    else
        portID = '/dev/tty.usbserial-001'; % Replace with correct port
    end

    s = serialport(portID, 9600); % MAKE SURE BAUD RATES ARE CORRECT
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
            data1 = data{1}
            data2 = data{2}
%             csvstr = append(data1, data2)
    
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
    end
    parpool close

% 
% spmd(2)
%     if labindex == 1
%         portID = 1;
%     else
%         portID = 6;
%     end
%     disp(labindex);
%     s = serial(['COM' num2str(portID)]);
%     fopen(s);
%     fprintf(s, ['Hello from port number ' num2str(portID)]);
%     out = fscanf(s, '%c');
%     fclose(s);
%     delete(s);
% end
% 
% out{1}
% out{2}
% matlabpool close
