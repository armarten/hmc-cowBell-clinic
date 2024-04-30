%% Get data to files
close all hidden;
clear;

samp_per_s  = 50E-3;
samp_per_ms = 50;

all_file_names = {'sweep_air_big_alicat20', 'sweep_air_big_esp20', 'sweep_air_big_alicat250', 'sweep_air_big_esp250', 'sweep_air_small_alicat20', 'sweep_air_small_esp', 'sweep_ch4_big_alicat20', 'sweep_ch4_big_esp20', 'sweep_ch4_big_alicat250', 'sweep_ch4_big_esp250', 'sweep_ch4_small_alicat20', 'sweep_ch4_small_esp20'};

cropTali = {145:3673, 117:2843, 2011:5756, 156:4786, 253:5585, 282:5671};
cropTesp = {195:3723, 318:3044, 2370:6115, 182:4812, 243:5575, 337:5726};

% Tali_s_

for file_index = (0:(length(all_file_names)/2)-1)

    ali_filename = all_file_names{2*file_index+1};
    esp_filename = all_file_names{2*file_index+2};
    
    file_loc_ali = append('arduino_output_save/new_sweep/',ali_filename );
    file_loc_esp = append('arduino_output_save/new_sweep/',esp_filename);
    
    
    if contains( ali_filename , "ch4" ) 
        comp = "CH_4";
    else
        comp = "Air";
    end


    Tali = readtable(file_loc_ali); 
    Tesp = readtable(file_loc_esp); 

    Tali = Tali(cropTali{file_index+1},:);
    Tesp = Tesp(cropTesp{file_index+1},:);



    Tali = renamevars(Tali,["Var1","Var2","Var3","Var4","Var5","Var6"], ...
                ["pres","temp","vflow","mflow","stpt","comp"]);
    Tesp = renamevars(Tesp,["Var1","Var2","Var3"], ...
                ["time_ms", "sensor", "mflow"]);

    if Tesp.sensor{1} == 's'
        sensor = "SFM3400 (Small)";
    elseif Tesp.sensor{1} == 'b'
            sensor = "SFM3300 (Large)";
    end
    
    

    if (max(Tali.mflow) < 100)
        rowsToDelete = Tali.vflow  >25.4; %VOV
        Tali(rowsToDelete,:) = [];
        Tesp(rowsToDelete,:) = [];
    end

    
    
    plots(ali_filename, Tali, Tesp, file_index+1, sensor, comp);
end



% rdtbl = readtable(file_location);
%     dead_rows = get_header_length(file_location); % Number of rows before column headers





% Tesp = renamevars(Tesp,["Var1", "Var2", "Var3", "Var4"], ...
%                 ["time_ms", "flow33","flow34", "total"]);

%% 
function plots(filename, Tali, Tesp, fignum, sensor, comp)
    figure(10+fignum)
    plot(Tesp.mflow);
    hold on;
    plot(Tali.mflow);
    hold off;
    title([sensor comp])
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
    title("Error of SFM3300 vs True Flow Rate", FontSize=34)
    ylabel("% Error")
    xlabel("Alicat Mass Flow Reading (SLPM)")
    
    
    figure(90+fignum)
    scatter(x33, y33 ./ 100 .* Tali.mflow);
    hold on;
    Fit33 = polyfit(x33, y33 ./ 100 .* Tali.mflow, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
    plot(x33, polyval(Fit33,x33),"LineWidth",3)
    % yline([10 -10],"--");
    % xlim([0 30])
    hold off;
    grid on;
    grid minor;
    title("Error of SFM3300 vs True Flow Rate")
    ylabel("Offset Error (SLPM)")
    xlabel("Alicat Mass Flow Reading (SLPM)")


end

