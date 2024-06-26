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


    plots(ali_filename, Tali, Tesp, file_index+1, sensor, comp);


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

scatter(xba, yba)
hold on;

scatter(xbc, ybc)

% polys = (4:7);
% for n = polys
%     % Fitba = polyfit(xba, yba, 6); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
%     Fitbc = polyfit(xbc, ybc, n); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
% 
%     % plot(xba, polyval(Fitba,xba),"LineWidth",3)
%     plot(xbc, polyval(Fitbc,xbc),"LineWidth",3)
% 
% end
Fitba = polyfit(xba, yba, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(xba, polyval(Fitba,xba),"LineWidth",3)

Fitbc = polyfit(xbc, ybc, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(xbc, polyval(Fitbc,xbc),"LineWidth",3)


plot(xlim,xlim,'-b','LineWidth',2)
hold off;
ylabel("Alicat Reading (SLPM)")
xlabel("SFM3300 Reading (SLPM)")
title("Sensirion SFM3300 (Large) Reading vs Alicat Reading")
% legend("Air", "CH_4", "airfit","ch4fit","y=x")
legend(["air" "ch4" "airpoly" "ch4poly" "y=x"])



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


scatter(xba, yba);
hold on;
scatter(xbc, ybc);
% 
% polys = (4:9);
% for n = polys
%     Fitba = polyfit(xba, yba, 9); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
%     Fitbc = polyfit(xbc, ybc, n); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
% 
%     plot(xbc, polyval(Fitbc,xbc),"LineWidth",3)
% end
%     plot(xba, polyval(Fitba,xba),"LineWidth",3)


Fitba = polyfit(xba, yba, 9); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
Fitbc = polyfit(xbc, ybc, 8); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 

plot(xba, polyval(Fitba,xba),"LineWidth",3)
plot(xbc, polyval(Fitbc,xbc),"LineWidth",3)


hold off;
grid on;
grid minor;

title("Air vs Methane Behavior for SFM3300 (Large Sensor)")
ylabel("% Error in Sensirion Sensor Reading Compared to Alicat Sensor Reading")
xlabel("Sensirion SFM3300 (Large) Mass Flow Reading (SLPM)")
legend("Air", "CH4", "Air Polyfit", "CH4 Polyfit");



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


scatter(xsa, ysa)
hold on;

scatter(xsc, ysc)

% polys = (4:7);
% for n = polys
%     % Fitsa = polyfit(xsa, ysa, 6); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
%     Fitsc = polyfit(xsc, ysc, n); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
% 
%     % plot(xsa, polyval(Fitsa,xsa),"LineWidth",3)
%     plot(xsc, polyval(Fitsc,xsc),"LineWidth",3)
% 
% end
Fitba = polyfit(xsa, ysa, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(xsa, polyval(Fitba,xsa),"LineWidth",3)

Fitbc = polyfit(xsc, ysc, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(xsc, polyval(Fitbc,xsc),"LineWidth",3)


plot(xlim,xlim,'-b','LineWidth',2)
hold off;


ylabel("Alicat Reading (SLPM)")
xlabel("SFM3300 Reading (SLPM)")
title("Sensirion SFM3400 (Small) Reading vs Alicat Reading")
% legend("Air", "CH_4", "airfit","ch4fit","y=x")
legend(["air" "ch4" "airpoly" "ch4poly" "y=x"])


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

scatter(xsa, ysa);
hold on;
scatter(xsc, ysc);
% 
Fitsa = polyfit(xsa, ysa, 5); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
Fitsc = polyfit(xsc, ysc, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 

plot(xsa, polyval(Fitsa,xsa),"LineWidth",3)
plot(xsc, polyval(Fitsc,xsc),"LineWidth",3)




hold off;
grid on;
grid minor;

title("Air vs Methane Behavior for SFM3400 (Small Sensor)")
ylabel("% Error in Sensirion Sensor Reading Compared to Alicat Sensor Reading")
xlabel("Sensirion SFM3400 (Small) Mass Flow Reading (SLPM)")
legend("Air", "CH4", "Air Polyfit", "CH4 Polyfit");


% Plot esp reading vs alicat reading
% for ch4 and air
% Then do the diff of those

figure(210)
lower_max_big = min(max(xba), max(xbc));
poly_big_x = linspace(1, round(lower_max_big), 500);
poly_big_air = polyval(Fitba, poly_big_x);
poly_big_ch4 = polyval(Fitbc, poly_big_x);
diff_poly_big = poly_big_ch4 - poly_big_air;
div_poly_big = poly_big_ch4 ./ poly_big_air;

plot(poly_big_x, diff_poly_big)
hold on;
% plot(poly_big_x, div_poly_big)
hold off;

legend("CH4 -  Air", "CH4 ./ Air")

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




%% Old Code

% 
% polys = (3:6);
% for n = polys
%     % Fitba = polyfit(xsa, ysa, 5); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
%     Fitbc = polyfit(xsc, ysc, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
% 
%     plot(xsc, polyval(Fitbc,xsc),"LineWidth",3)
% end
% % plot(xsa, polyval(Fitba,xsa),"LineWidth",3)
% 
% legend(["ch4" polys])



% 
% legend(["air" "ch4" string(polys)  "air9"])
% Fitba = fit(xba,yba,'smoothingspline')
% Fitbc = fit(xbc,ybc,'smoothingspline')
% Fitba = sgolayfilt(xba,order,framelen)
% Fitbc = sgolayfilt(xbc,order,framelen)
% Fitba = movmean(yba,10)
% Fitbc = movmean(ybc,10)






% % Sample Data Preparation
% t = xbc; % Adjust according to your actual time vector
% orange_data = ybc; % Example data, replace with actual
% % blue_data = yba; % Example data, replace with actual
% 
% % Fitting a sinusoidal model to the orange data
% % ft = fittype('a*sin(b*x + c) + d', 'independent', 'x', 'dependent', 'y');
% % opts = fitoptions(ft);
% % opts.StartPoint = [10 0.07 0 0]; % Initial guess: Amplitude, Frequency, Phase, Offset
% % [fitresult_orange, gof_orange] = fit(t, orange_data, ft, opts);
% 
% % Fitting a sinusoidal model to the blue data
% % [fitresult_blue, gof_blue] = fit(t', blue_data', ft, opts);
% 
% % Plotting the results
% figure;
% plot(fitresult_orange, t', orange_data');
% hold on;
% % plot(fitresult_blue, t', blue_data');
% legend('Orange Data', 'Fit for Orange', 'Blue Data', 'Fit for Blue');
% title('Sinusoidal Fits to Data');
% xlabel('Time');
% ylabel('Data Value');
% grid on;
% hold off;


% plot(Fitba,"LineWidth",3)
% plot(Fitbc,"LineWidth",3)
