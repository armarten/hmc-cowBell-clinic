% function txt_to_csv(inputFileName)
%     
% %     fullFileName = append('arduino_output_save/', inputFileName)
% %     data = readtable(fullFileName)  
%      folderPath = 'arduino_output_save/';
%      outputFileName = inputFileName;
% 
%      filePath = fullfile(folderPath, inputFileName);
%     
% 
%     % Check if the file exists
% %     if exist(filePath, 'file') ~= 2
% %         error('File not found in specified folder.');
% %     end
% 
%     % Read the contents of the text file
%     fileID = fopen(filePath, 'r');
%     fileContents = textscan(fileID, '%s', 'Delimiter', '\n', 'Whitespace', '');
%     fclose(fileID);
% 
%     % Extract the content from the cell array
%     fileContents = fileContents{1};
% 
%     % Remove the first and last line
%     fileContents = fileContents(2:end-1);
% 
%     % Convert the remaining lines to a matrix
%     data = cellfun(@(x) str2double(strsplit(x)), fileContents, 'UniformOutput', false);
%     data = vertcat(data{:});
% 
%     % Write the matrix to a CSV file
%     [~, name, ~] = fileparts(inputFileName);
%     outputFileName = fullfile(folderPath, [folderPath name '.csv']);
%     csvwrite([folderPath name], data);
% end
% 
% function txt_to_csv(filename)
%     % Define the folder path
%     folderPath = 'arduino_output_save';
%     
%     % Construct full path to the input text file
%     fullPath = fullfile(folderPath, filename);
%     
%     % Define the output CSV file name
%     [~, name, ~] = fileparts(filename);
%     outputCsvFile = fullfile(folderPath, [name '.csv']);
%     
%     % Open the text file for reading
%     fid = fopen(fullPath, 'rt');
%     if fid == -1
%         error('File %s could not be opened.', fullPath);
%     end
%     
%     % Prepare to read the data line by line
%     data = [];
%     while ~feof(fid)
%         line = fgetl(fid);
%         % Parse the line into numeric data and a string
%         nums = sscanf(line, '%f %f %f %f', [1, 4]);
%         str = strtrim(regexp(line, '[^\d\.\+\-]*$', 'match', 'once'));
%         
%         % Append the numeric data and convert the string to a numeric code if needed
%         data = [data; nums]; % For simplicity, we ignore the string part here
%         
%         % Alternatively, to include the string as a separate column:
%         % data = [data; nums, {str}]; % Uncomment to store as cell array including the string
%     end


% function txt_to_csv(filename)
%     % Extract the folder path from the input filename
%     [folderPath, ~, ~] = fileparts(filename);
%     
%     % Construct full path to the input text file
%     fullPath = fullfile(folderPath, filename);
%     
%     % Define the output CSV file name
%     [~, name, ~] = fileparts(filename);
%     outputCsvFile = fullfile(folderPath, [name '.csv']);
%     
%     % Open the text file for reading
%     fid = fopen(fullPath, 'rt');
%     if fid == -1
%         error('File %s could not be opened.', fullPath);
%     end
%     
%     % Prepare to read the data line by line
%     data = [];
%     while ~feof(fid)
%         line = fgetl(fid);
%         % Parse the line into numeric data and a string
%         nums = sscanf(line, '%f %f %f %f', [1, 4]);
%         str = strtrim(regexp(line, '[^\d\.\+\-]*$', 'match', 'once'));
%         
%         % Append the numeric data and convert the string to a numeric code if needed
%         data = [data; nums]; % For simplicity, we ignore the string part here
%         
%         % Alternatively, to include the string as a separate column:
%         % data = [data; nums, {str}]; % Uncomment to store as cell array including the string
%     end
%     
%     % Close the text file
%     fclose(fid);
%     
%     % Write data to CSV file
%     csvwrite(outputCsvFile, data);
% end

data = importdata('arduino_output_save/parallel_all_run_1_mfm');
csvwrite('arduino_output_save/parallel_all_run_1_mfm.csv', data.data);



% data = importdata('filename.txt');
% csvwrite('filename.csv',data.data);

