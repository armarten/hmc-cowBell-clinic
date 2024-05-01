%% Get data to files
close all hidden;
clear;

samp_per_s  = 50E-3;
samp_per_ms = 50;
concatFlag = 0;

all_file_names = {'sweep_air_big_alicat20', 'sweep_air_big_esp20', 'sweep_air_big_alicat250', 'sweep_air_big_esp250', 'sweep_air_small_alicat20', 'sweep_air_small_esp', 'sweep_ch4_big_alicat20', 'sweep_ch4_big_esp20', 'sweep_ch4_big_alicat250', 'sweep_ch4_big_esp250', 'sweep_ch4_small_alicat20', 'sweep_ch4_small_esp20'};


cropTali = {145:3673, 117:2843, 2011:5756, 156:4786, 253:5585, 282:5671};
cropTesp = {195:3723, 318:3044, 2370:6115, 182:4812, 243:5575, 337:5726};


for file_index = (0:(length(all_file_names)/2)-1)

    ali_filename = all_file_names{2*file_index+1};
    esp_filename = all_file_names{2*file_index+2};
    
    file_loc_ali = append('arduino_output_save/new_sweep/',ali_filename );
    file_loc_esp = append('arduino_output_save/new_sweep/',esp_filename);
    
    
    
    if contains( ali_filename , "ch4" ) 
        comp = "CH4";
    else
        comp = "Air";
    end
    

    Tali = readtable(file_loc_ali); 
    Tesp = readtable(file_loc_esp); 

    

    Tali = Tali(cropTali{file_index+1},:);
    Tesp = Tesp(cropTesp{file_index+1},:);

    Tali = rmoutliers(Tali,'DataVariables','mflow')
    

    % Assuming you have time vectors or can create them based on sampling frequency
    time_ali = (1:height(Tali)) / 50; % Adjust 50 to actual sample rate if needed
    time_esp = (1:height(Tesp)) / 50; % Adjust 50 to actual sample rate if needed


    Tali = renamevars(Tali,["Var1","Var2","Var3","Var4","Var5","Var6"], ...
                ["pres","temp","vflow","mflow","stpt","comp"]);
    Tesp = renamevars(Tesp,["Var1","Var2","Var3"], ...
                ["time_ms", "sensor", "mflow"]);



    if width(Tali) > 6
        Tali = removevars(Tali,{'Var7', 'Var8'});
    end

    if Tesp.sensor{1} == 's'
        sensor = "SFM3400 (Small)";
    elseif Tesp.sensor{1} == 'b'
        sensor = "SFM3300 (Large)";
    end

    % Cross-correlation to find the time offset
    [c, lags] = xcorr(Tesp.mflow, Tali.mflow, 'coeff');
    [~, I] = max(abs(c));
    time_shift = lags(I)


        % Adjusting Tesp based on time shift
        
    if time_shift > 0
        Tesp = [Tesp(time_shift+1:end, :); array2table(nan(time_shift, width(Tesp)), 'VariableNames', Tesp.Properties.VariableNames)];
    elseif time_shift < 0
        Tesp = [array2table(nan(-time_shift, width(Tesp)), 'VariableNames', Tesp.Properties.VariableNames); Tesp(1:end+time_shift, :)];
    end
    % 
    % % Proceed with renaming, removing variables, and your other data processing...
    % Tali = renamevars(Tali, ["Var1", "Var2", "Var3", "Var4", "Var5", "Var6"], ...
    %                   ["pres", "temp", "vflow", "mflow", "stpt", "comp"]);
    % Tesp = renamevars(Tesp, ["Var1", "Var2", "Var3"], ...
    %                   ["time_ms", "sensor", "mflow"]);


    % Identify rows with mflow equal to 0 in either table
    rows_to_delete = (Tali.mflow == 0 | Tesp.mflow == 0);

    % Remove those rows from both tables
    Tali(rows_to_delete, :) = [];
    Tesp(rows_to_delete, :) = [];




    if width(Tali) > 6
        Tali = removevars(Tali,{'Var7', 'Var8'});
    end

    if Tesp.sensor{1} == 's'
        sensor = "SFM3400 (Small)";
    elseif Tesp.sensor{1} == 'b'
        sensor = "SFM3300 (Large)";
    end

    Tali.time_ms = (transpose((0:height(Tali)-1)) .* samp_per_ms) + min(Tesp.time_ms);
    Tali.sensor = repmat(Tesp.sensor{1},height(Tali),1);

    

    if (max(Tali.mflow) < 100)
        rowsToDelete = Tali.vflow  >25.4; %VOV
        Tali(rowsToDelete,:) = [];
        Tesp(rowsToDelete,:) = [];
    end

    
   

    if concatFlag == 0 && sensor == "SFM3300 (Large)" && comp == "Air"
        big_sensor_air_esp = Tesp;
        big_sensor_air_alicat = Tali;
        concatFlag = 1;

    elseif concatFlag == 1 && sensor == "SFM3300 (Large)" && comp == "Air"
        big_sensor_air_esp = [big_sensor_air_esp ; Tesp];
        big_sensor_air_alicat = [big_sensor_air_alicat ; Tali];
        concatFlag = 0;

    elseif concatFlag == 0 && sensor == "SFM3300 (Large)" && comp == "CH4"
        big_sensor_ch4_esp = Tesp;
        big_sensor_ch4_alicat = Tali;
        concatFlag = 1;

    elseif concatFlag == 1 && sensor == "SFM3300 (Large)" && comp == "CH4"
        big_sensor_ch4_esp = [big_sensor_ch4_esp ; Tesp];
        big_sensor_ch4_alicat = [big_sensor_ch4_alicat ; Tali];
        concatFlag = 0;

    elseif sensor == "SFM3400 (Small)" && comp == "Air"
        small_sensor_air_esp = Tesp;
        small_sensor_air_alicat = Tali;

    elseif sensor == "SFM3400 (Small)" && comp == "CH4"
        small_sensor_ch4_esp = Tesp;
        small_sensor_ch4_alicat = Tali;
        concatFlag = 0;

    end


    % plots(ali_filename, Tali, Tesp, file_index+1, sensor, comp);


end


plots('concat_air', big_sensor_air_alicat, big_sensor_air_esp, 7, "SFM3300 (Large)", "Air");
plots('concat_ch4', big_sensor_ch4_alicat, big_sensor_ch4_esp, 8, "SFM3300 (Large)", "CH4");

% I want 2 figures, one for big sensor and one for small sensor

figure(200) % Big Sensor
xba = big_sensor_air_alicat.mflow;
yba = (big_sensor_air_esp.mflow-big_sensor_air_alicat.mflow)./big_sensor_air_alicat.mflow*100;
xbc = big_sensor_ch4_alicat.mflow;
ybc = (big_sensor_ch4_esp.mflow-big_sensor_ch4_alicat.mflow)./big_sensor_ch4_alicat.mflow*100;

scatter(xba, yba);
hold on;
scatter(xbc, ybc);

Fitba = polyfit(xba, yba, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
Fitbc = polyfit(xbc, ybc, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 

plot(xba, polyval(Fitba,xba),"LineWidth",3)
plot(xbc, polyval(Fitbc,xbc),"LineWidth",3)

hold off;
grid on;
grid minor;

title("Air vs Methane Behavior for SFM3300 (Large Sensor)")
ylabel("% Error in Sensirion Sensor Reading")
xlabel("Alicat Mass Flow Reading (SLPM)")
legend("Air", "CH4", "Air Polyfit", "CH4 Polyfit");
%  Throw out outliers, 2-3 stdevs away



figure(201) % Small Sensor
xsa = small_sensor_air_alicat.mflow;
ysa = (small_sensor_air_esp.mflow-small_sensor_air_alicat.mflow)./small_sensor_air_alicat.mflow*100;
xsc = small_sensor_ch4_alicat.mflow;
ysc = (small_sensor_ch4_esp.mflow-small_sensor_ch4_alicat.mflow)./small_sensor_ch4_alicat.mflow*100;

scatter(xsa, ysa);
hold on;
scatter(xsc, ysc);

Fitsa = polyfit(xsa, ysa, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
Fitsc = polyfit(xsc, ysc, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 

plot(xsa, polyval(Fitsa,xsa),"LineWidth",3)
plot(xsc, polyval(Fitsc,xsc),"LineWidth",3)

hold off;
grid on;
grid minor;

title("Air vs Methane Behavior for SFM3400 (Small Sensor)")
ylabel("% Error in Sensirion Sensor Reading")
xlabel("Alicat Mass Flow Reading (SLPM)")
legend("Air", "CH4", "Air Polyfit", "CH4 Polyfit");

% Tesp = renamevars(Tesp,["Var1", "Var2", "Var3", "Var4"], ...
%                 ["time_ms", "flow33","flow34", "total"]);

%% 
function plots(filename, Tali, Tesp, fignum, sensor, comp)
    figure(10+fignum)
    plot(Tesp.mflow);
    hold on;
    plot(Tali.mflow);
    hold off;
    title([filename sensor comp "Flow vs Time"])
    ylabel("Mass Flow Rate (SLPM)");
    xlabel("Samples");
    legend("ESP", "Ali")

    x33 = Tali.mflow;
    y33 = (Tesp.mflow-Tali.mflow)./Tali.mflow*100;

    figure(80+fignum)
    scatter(x33, y33);
    hold on;
    Fit33 = polyfit(x33, y33, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
    plot(x33, polyval(Fit33,x33),"LineWidth",3)
    % yline([10 -10],"--");
    % ylim([-25 25]);
    % xlim([0 30])
    ylim([-15 20]);

    hold off;
    grid on;
    grid minor;
    title([filename sensor comp "Error of SFM3300 vs True Flow Rate"])
    ylabel("% Error")
    xlabel("Alicat Mass Flow Reading (SLPM)")
    
    % 
    % figure(90+fignum)
    % scatter(x33, y33 ./ 100 .* Tali.mflow);
    % hold on;
    % Fit33 = polyfit(x33, y33 ./ 100 .* Tali.mflow, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
    % plot(x33, polyval(Fit33,x33),"LineWidth",3)
    % % yline([10 -10],"--");
    % % xlim([0 30])
    % hold off;
    % grid on;
    % grid minor;
    % title("Error of SFM3300 vs True Flow Rate")
    % ylabel("Offset Error (SLPM)")
    % xlabel("Alicat Mass Flow Reading (SLPM)")


end

