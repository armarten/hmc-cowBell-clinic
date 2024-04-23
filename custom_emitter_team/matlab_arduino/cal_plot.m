close all hidden;
name_ali = 'cal_sensors_alicat';
name_esp = 'cal_sensors_esp32';
file_loc_ali = append('arduino_output_save/',name_ali);
file_loc_esp = append('arduino_output_save/',name_esp);

% rdtbl = readtable(file_location);
%     dead_rows = get_header_length(file_location); % Number of rows before column headers


Tali = readtable(file_loc_ali); 
Tesp = readtable(file_loc_esp, MissingRule="omitrow"); 

Tesp = rmmissing(Tesp);
Tesp(1:6,:) = [];
Tesp(1828:1828+76,:) = [];



% tmp = split(Tesp.Var2," , ");
% Tesp.flow33 = tmp(:,2);
% Tesp.flow34 = tmp(:,3);
Tali = renamevars(Tali,["Var1","Var2","Var3","Var4","Var5","Var6"], ...
                ["pres","temp","vflow","mflow","stpt","comp"])
Tesp = renamevars(Tesp,["Var1","Var2","Var3","Var4","Var5","Var6","Var7","Var8"], ...
                ["time","time_ms","dlt1","flow33","dlt2","flow34","dlt3","flowtot"])

% Tesp.flow33 = str2num(Tesp.flow33);

%% Plotting
figure(65)
plot(Tali.mflow)
% plot(Tali.Var1,Tali.Var4)
hold on;
plot(Tesp.flow33)
plot(Tesp.flow34)
hold off;
legend("Alicat", "3300", "3400")
grid on;
grid minor;

figure(60)
plot((Tesp.flow33([947:1148,1172:2569])-Tali.mflow([947:1148,1172:2569]))/Tali.mflow([947:1148,1172:2569])*100);
% plot((Tesp.flow33(100:2500)-Tali.mflow(100:2500))/Tali.mflow(100:2500)*100);
grid on;
grid minor;
ylabel("% Error")
xlabel("Run Time")
title("3300 Error Compared to Alicat")

figure(75)
plot((Tesp.flow34(64:857)-Tali.mflow(64:857))/Tali.mflow(64:857)*100);
grid on;
grid minor;
ylabel("% Error")
xlabel("Run Time")
title("3400 Error Compared to Alicat")


figure(83)
scatter(Tali.mflow([947:1148,1172:2569]), (Tesp.flow33([947:1148,1172:2569])-Tali.mflow([947:1148,1172:2569]))./Tali.mflow([947:1148,1172:2569])*100);
hold on;
Fit = polyfit(Tali.mflow([947:1148,1172:2569]), (Tesp.flow33([947:1148,1172:2569])-Tali.mflow([947:1148,1172:2569]))./Tali.mflow([947:1148,1172:2569])*100,4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(polyval(Fit,Tali.mflow([947:1148,1172:2569])))
hold off;
grid on;
grid minor;
title("Error of SFM3300 vs True Flow Rate")
ylabel("% Error")
xlabel("Alicat Mass Flow Reading")

figure(84)
scatter(Tali.mflow(64:857), (Tesp.flow34(64:857)-Tali.mflow(64:857))./Tali.mflow(64:857)*100);
hold on;
Fit = polyfit(Tali.mflow(64:857), (Tesp.flow34(64:857)-Tali.mflow(64:857))./Tali.mflow(64:857)*100,4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(polyval(Fit,Tali.mflow(64:857)))
hold off;
grid on;
grid minor;
title("Error of SFM3400 vs True Flow Rate")
ylabel("% Error")
xlabel("Alicat Mass Flow Reading")


% 02:13:04.971	 