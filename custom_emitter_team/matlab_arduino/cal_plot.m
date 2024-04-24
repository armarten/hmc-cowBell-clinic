close all hidden;
clear;
name_ali = 'cal_sensors_alicat';
name_esp = 'cal_sensors_esp32';
file_loc_ali = append('arduino_output_save/',name_ali);
file_loc_esp = append('arduino_output_save/',name_esp);

samp_per = 120E-3;

% rdtbl = readtable(file_location);
%     dead_rows = get_header_length(file_location); % Number of rows before column headers


Tali = readtable(file_loc_ali); 
Tesp = readtable(file_loc_esp, MissingRule="omitrow"); 

Tesp = rmmissing(Tesp);
Tesp(1:8,:) = [];
Tesp(1828:1828+74,:) = [];



% tmp = split(Tesp.Var2," , ");
% Tesp.flow33 = tmp(:,2);
% Tesp.flow34 = tmp(:,3);
Tali = renamevars(Tali,["Var1","Var2","Var3","Var4","Var5","Var6"], ...
                ["pres","temp","vflow","mflow","stpt","comp"]);
Tesp = renamevars(Tesp,["Var1","Var2","Var3","Var4","Var5","Var6","Var7","Var8"], ...
                ["time","time_ms","dlt1","flow33","dlt2","flow34","dlt3","flowtot"]);

Tali(2569:2754,:) = [];


% Tesp.flow33 = str2num(Tesp.flow33);

%%



Talimatch33 = Tali.mflow(947:2568);
Talimatch34 = Tali.mflow(64:857);

Tespmatch33 = Tesp.flow33(947:2568);
Tespmatch34 = Tesp.flow34(64:857);

keepme33 = Talimatch33~=0 & Talimatch33>50 & Talimatch33<250 & Tespmatch33~=0;
keepme34 = Talimatch34~=0 & Tespmatch34~=0 & Talimatch34>5;

Tali33mflowCut = Talimatch33(keepme33);
Tali34mflowCut = Talimatch34(keepme34);


Tesp33no0 = Tespmatch33(keepme33);
Tesp34no0 = Tespmatch34(keepme34);

x33 = Tali33mflowCut;
y33 = (Tesp33no0-Tali33mflowCut)./Tali33mflowCut*100;

x34 = Tali34mflowCut;
y34 = (Tesp34no0-Tali34mflowCut)./Tali34mflowCut*100;


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

figure(66)
plot(Talimatch33)
% plot(Tali.Var1,Tali.Var4)
hold on;
plot(Tespmatch33)
hold off;
legend("Alicat", "3300")
grid on;
grid minor;
title("SFM3300 in Series with Alicat 250 SLPM")


figure(67)
times = gettimes(samp_per,Tespmatch34,"min");
plot(times, Talimatch34)
% plot(Tali.Var1,Tali.Var4)
hold on;
plot(times,Tespmatch34)
hold off;
legend("Alicat", "3400")
grid on;
grid minor;
xlabel("Time (s)")
title("SFM3400 in Series with Alicat 250 SLPM")

figure(60)
plot(y33);
% plot((Tesp.flow33(100:2500)-Tali.mflow(100:2500))/Tali.mflow(100:2500)*100);
grid on;
grid minor;
ylabel("% Error")
xlabel("Run Time")
title("3300 Error Compared to Alicat")

figure(75)
plot(y34);
grid on;
grid minor;
ylabel("% Error")
xlabel("Run Time")
title("3400 Error Compared to Alicat")


figure(83)
scatter(x33, y33);
hold on;
Fit33 = polyfit(x33, y33, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(x33, polyval(Fit33,x33))
yline([10 -10],"--");
ylim([-25 25]);
xlim([0 250])
hold off;
grid on;
grid minor;
title("Error of SFM3300 vs True Flow Rate")
ylabel("% Error")
xlabel("Alicat Mass Flow Reading (SLPM)")

figure(84)
scatter(x34, y34);
hold on;
Fit = polyfit(x34, y34,4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
plot(x34, polyval(Fit,x34))
yline([10 -10],"--");
ylim([-25 25]);
xlim([0 40])
hold off;
grid on;
grid minor;
title("Error of SFM3400 vs True Flow Rate")
ylabel("% Error")
xlabel("Alicat Mass Flow Reading (SLPM)")

figlist = [];
save_plots(figlist, "sensirion_cal")

% 02:13:04.971	 


function save_plots(figlist, file_name)
    % Takes the figures listed in figlist and saves them as .png and .fig
    % to the folder "arduino_plot_figs"
    if isempty(figlist)
    else
        for fig = figlist
            fig_file_name = append('arduino_plot_figs/',file_name,"_fig","_",int2str(fig));
            saveas(fig, fig_file_name, 'png')
            saveas(fig, fig_file_name, 'fig')
        end  
    end
end

function times = gettimes(samp_period, tab, secmin)
    times = samp_period .* (1:height(tab));
    if (secmin == "min")
        times = times ./ 60;
    end
end
