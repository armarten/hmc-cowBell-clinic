function txt_to_csv(inputFileName)
    
    fullFileName = append('arduino_output_save/', inputFileName, ".txt")
    data = readtable(fullFileName)  



%     filePath = fullfile(folderPath, inputFileName);
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
%     outputFileName = fullfile(folderPath, [name '.csv']);
%     csvwrite(outputFileName, data);
end
