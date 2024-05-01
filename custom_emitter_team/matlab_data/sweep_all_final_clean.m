%% Get data to files
close all hidden;
clear;

sz = 10;
lw = 2;
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

    Tali = renamevars(Tali,["Var1","Var2","Var3","Var4","Var5","Var6"], ...
                ["pres","temp","vflow","mflow","stpt","comp"]);
    Tesp = renamevars(Tesp,["Var1","Var2","Var3"], ...
                ["time_ms", "sensor", "mflow"]);

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

    if (sensor == "SFM3300 (Large)")
        rowsToDelete1 = Tali.mflow < 2;
        Tali(rowsToDelete1,:) = [];
        Tesp(rowsToDelete1,:) = [];
    elseif (sensor == "SFM3400 (Small)")
        rowsToDelete1 = Tali.mflow < 0.3;
        Tali(rowsToDelete1,:) = [];
        Tesp(rowsToDelete1,:) = [];
    end


    if (max(Tali.mflow) < 100)
        rowsToDelete = Tali.vflow  > 25.4; %VOV
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

%% Sort everything in ascending order by Alicat flow rate


[big_sensor_air_alicat, sortIndex_ba]   = sortrows(big_sensor_air_alicat, 'mflow');
big_sensor_air_esp                      = big_sensor_air_esp(sortIndex_ba,:);

[big_sensor_ch4_alicat, sortIndex_bc]   = sortrows(big_sensor_ch4_alicat, 'mflow');
big_sensor_ch4_esp                      = big_sensor_ch4_esp(sortIndex_bc,:);

[small_sensor_air_alicat, sortIndex_sa] = sortrows(small_sensor_air_alicat, 'mflow');
small_sensor_air_esp                    = small_sensor_air_esp(sortIndex_sa,:);

[small_sensor_ch4_alicat, sortIndex_sc] = sortrows(small_sensor_ch4_alicat, 'mflow');
small_sensor_ch4_esp                    = small_sensor_ch4_esp(sortIndex_sc,:);




% plots('concat_air', big_sensor_air_alicat, big_sensor_air_esp, 7, "SFM3300 (Large)", "Air");
% plots('concat_ch4', big_sensor_ch4_alicat, big_sensor_ch4_esp, 8, "SFM3300 (Large)", "CH4");
%% Big sensor read vs actual

yba = big_sensor_air_alicat.mflow;
xba = big_sensor_air_esp.mflow;
ybc = big_sensor_ch4_alicat.mflow;
xbc = big_sensor_ch4_esp.mflow;

[~,TFrmba,~] = rmoutliers(yba, 'movmedian', 20);
yba(TFrmba,:) = [];
xba(TFrmba,:) = [];
[~,TFrmbc,~] = rmoutliers(ybc, 'movmedian', 20);
ybc(TFrmbc,:) = [];
xbc(TFrmbc,:) = [];


figure(20)

scatter(xba, yba, sz, "MarkerEdgeColor", "#0072BD")
hold on;

scatter(xbc, ybc, sz, "MarkerEdgeColor", "#D95319")

Fitba = polyfit(xba, yba, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(xba, polyval(Fitba,xba),'-','color',"#4DBEEE","LineWidth",lw)

Fitbc = polyfit(xbc, ybc, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(xbc, polyval(Fitbc,xbc),'-','color',"#EDB120", "LineWidth",lw)


plot([0 max(max(xba), max(xbc))],[0 max(max(xba), max(xbc))],'--k','LineWidth',1)
hold off;
ylabel("Reference Sensor Reading (SLPM)")
xlabel("Sensirion SFM3300 (Large Sensor) Mass Flow Reading (SLPM)")
title(["Sensirion SFM3300 (Large Sensor) Reading" "for Air and 2.5% Methane, Balance Air"], "FontSize",15)
subtitle("Referenced to Alicat M-Series 20 and 250 SLPM Mass Flow Meters")
legend("Air", "2.5% Methane, Bal Air", "Air Polyfit", "2.5% Methane, Bal Polyfit","y = x", "Location", "NorthWest");



% %% Big Sensor Compare CH4 and Air
% 
% lower_max_big = min(max(xba), max(xbc));
% poly_big_x = linspace(1, round(lower_max_big), 500);
% poly_big_air = polyval(Fitba, poly_big_x);
% poly_big_ch4 = polyval(Fitbc, poly_big_x);
% diff_poly_big = (poly_big_ch4 - poly_big_air);
% diff_norm_poly_big = diff_poly_big ./ poly_big_air;
% 
% % div_poly_big = poly_big_ch4 ./ poly_big_air;
% 
% figure(30)
% plot(poly_big_x, diff_poly_big)
% title("CH4 - Air Difference Big Sensor")
% figure(40)
% plot(poly_big_x, diff_norm_poly_big)
% title("(CH4-Air)/Air Big Sensor")

% legend("CH4 -  Air", "CH4 ./ Air")

%% Big sensor error vs actual

% I want 2 figures, one for big sensor and one for small sensor

figure(200) % Big Sensor
xba = big_sensor_air_esp.mflow;
yba = (big_sensor_air_esp.mflow-big_sensor_air_alicat.mflow)./big_sensor_air_alicat.mflow*100;
xbc = big_sensor_ch4_esp.mflow;
ybc = (big_sensor_ch4_esp.mflow-big_sensor_ch4_alicat.mflow)./big_sensor_ch4_alicat.mflow*100;

[~,TFrmba,~] = rmoutliers(yba, 'movmedian', 100);
yba(TFrmba,:) = [];
xba(TFrmba,:) = [];
[~,TFrmbc,~] = rmoutliers(ybc, 'movmedian', 100);
ybc(TFrmbc,:) = [];
xbc(TFrmbc,:) = [];


scatter(xba, yba, sz);
hold on;
scatter(xbc, ybc, sz);
% 
% polys = (4:9);
% for n = polys
%     Fitba = polyfit(xba, yba, 9); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
%     Fitbc = polyfit(xbc, ybc, n); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
% 
%     plot(xbc, polyval(Fitbc,xbc),"LineWidth",lw)
% end
%     plot(xba, polyval(Fitba,xba),"LineWidth",lw)


Fitba = polyfit(xba, yba, 9); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
Fitbc = polyfit(xbc, ybc, 8); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 

plot(xba, polyval(Fitba,xba),"LineWidth",lw)
plot(xbc, polyval(Fitbc,xbc),"LineWidth",lw)
yline(0,"--k","LineWidth",1)

hold off;
grid on;
grid minor;


title(["Error in Sensirion SFM3300 (Large Sensor) Reading" "for Air and 2.5% Methane, Balance Air"], "FontSize",15)
subtitle("Referenced to Alicat M-Series 20 and 250 SLPM Mass Flow Meters")
ylabel("% Error Between Sensirion and Reference Reading");
xlabel("Sensirion SFM3300 (Large Sensor) Mass Flow Reading (SLPM)")
legend("Air", "2.5% Methane, Bal Air", "Air Polyfit", "2.5% Methane, Bal Polyfit","Error = 0", "Location", "NorthWest");



%% Small sensor read vs actual
figure(21)

ysa = small_sensor_air_alicat.mflow;
xsa = small_sensor_air_esp.mflow;
ysc = small_sensor_ch4_alicat.mflow;
xsc = small_sensor_ch4_esp.mflow;

[~,TFrmba,~] = rmoutliers(ysa, 'movmedian', 20);
ysa(TFrmba,:) = [];
xsa(TFrmba,:) = [];
[~,TFrmbc,~] = rmoutliers(ysc, 'movmedian', 20);
ysc(TFrmbc,:) = [];
xsc(TFrmbc,:) = [];


scatter(xsa, ysa, sz, "MarkerEdgeColor", "#0072BD")
hold on;

scatter(xsc, ysc, sz, "MarkerEdgeColor", "#D95319")


Fitba = polyfit(xsa, ysa, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(xsa, polyval(Fitba,xsa),'-','color',"#4DBEEE","LineWidth",lw)

Fitbc = polyfit(xsc, ysc, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(xsc, polyval(Fitbc,xsc),'-','color',"#EDB120", "LineWidth",lw)



plot([0 max(max(xsa), max(xsc))],[0 max(max(xsa), max(xsc))],'--k','LineWidth',1)
hold off;


ylabel("Alicat Reading (SLPM)")
xlabel("SFM3300 Reading (SLPM)")
title("Sensirion SFM3400 (Small) Reading vs Alicat Reading")
% legend("Air", "CH_4", "airfit","ch4fit","y=x")
legend("Air", "2.5% Methane, Bal Air", "Air Polyfit", "2.5% Methane, Bal Polyfit","y = x", "Location", "NorthWest");


%% Small sensor error vs actual

figure(201) % Small Sensor
xsa = small_sensor_air_esp.mflow;
ysa = (small_sensor_air_esp.mflow-small_sensor_air_alicat.mflow)./small_sensor_air_alicat.mflow*100;
xsc = small_sensor_ch4_esp.mflow;
ysc = (small_sensor_ch4_esp.mflow-small_sensor_ch4_alicat.mflow)./small_sensor_ch4_alicat.mflow*100;

[~,TFrmsa,~] = rmoutliers(ysa, 'movmedian', 200);
ysa(TFrmsa,:) = [];
xsa(TFrmsa,:) = [];
[~,TFrmsc,~] = rmoutliers(ysc, 'movmedian', 200);
ysc(TFrmsc,:) = [];
xsc(TFrmsc,:) = [];

scatter(xsa, ysa, sz);
hold on;
scatter(xsc, ysc, sz);
% 
Fitsa = polyfit(xsa, ysa, 5); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
Fitsc = polyfit(xsc, ysc, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 

plot(xsa, polyval(Fitsa,xsa),"LineWidth",lw)
plot(xsc, polyval(Fitsc,xsc),"LineWidth",lw)
yline(0,"--k","LineWidth",1)




hold off;
grid on;
grid minor;

title(["Error in Sensirion SFM3400 (Small Sensor) Reading" "for Air and 2.5% Methane, Balance Air"], "FontSize",15)
subtitle("Referenced to Alicat M-Series 20 and 250 SLPM Mass Flow Meters")
ylabel("% Error Between Sensirion and Reference Reading");
xlabel("Sensirion SFM3400 (Small Sensor) Mass Flow Reading (SLPM)")
legend("Air", "2.5% Methane, Bal Air", "Air Polyfit", "2.5% Methane, Bal Polyfit", "Error = 0", "Location", "NorthWest");


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
    plot(x33, polyval(Fit33,x33),"LineWidth",lw)
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
    % plot(x33, polyval(Fit33,x33),"LineWidth",lw)
    % % yline([10 -10],"--");
    % % xlim([0 30])
    % hold off;
    % grid on;
    % grid minor;
    % title("Error of SFM3300 vs True Flow Rate")
    % ylabel("Offset Error (SLPM)")
    % xlabel("Alicat Mass Flow Reading (SLPM)")


end



