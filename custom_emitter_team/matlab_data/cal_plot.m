%% Get data to files
close all hidden;
clear;
name_ali = 'running_avg_test_alicat250';
name_esp = 'running_avg_test_esp_3300';
file_loc_ali = append('arduino_output_save/',name_ali);
file_loc_esp = append('arduino_output_save/',name_esp);

samp_per = 120E-3;

% rdtbl = readtable(file_location);
%     dead_rows = get_header_length(file_location); % Number of rows before column headers


Tali = readtable(file_loc_ali); 
Tesp = readtable(file_loc_esp, MissingRule="omitrow"); 


Tali = renamevars(Tali,["Var1","Var2","Var3","Var4","Var5","Var6"], ...
                ["pres","temp","vflow","mflow","stpt","comp"]);
Tesp = renamevars(Tesp,["Var1","Var2","Var3","Var4"], ...
                ["time_ms","flow33","flow34","flowtot"]);


%% Test
% values = [];

figure(90)
n = 11;
flows = Tesp.flow33

plot(Tesp.time_ms,Tesp.flow33)
hold on;

cutoff = floor(length(flows)/n);
flows1 = flows(1:cutoff*n);
times1 = times(1:cutoff*n);
flows2 = reshape(flows1, [], n);
times2 = reshape(times1, [], n);
flows3 = mean(flows2, 1);
times3 = mean(times2, 1);
titles = [titles, n]
plot(times3, flows3);
hold off;



%% Cull bad rows, isolate data

% Tesp = rmmissing(Tesp);
% Tesp(1:8,:) = [];
% Tesp([1:1583+8 7886:height(Tesp)],:) = [];
% Tesp.time_ms = Tesp.time_ms - min(Tesp.time_ms);

Tesp([1:1600 3008:height(Tesp)], :) = [];
Tali([1:379 968:height(Tali)], :) = [];

Tali.time_ms = (transpose((1:height(Tali))) .* 120) + min(Tesp.time_ms);


% Tali.time = transpose(linspace(min(Tesp.time_ms),max(Tesp.time_ms),height(Tali)));

% tmp = split(Tesp.Var2," , ");
% Tesp.flow33 = tmp(:,2);
% Tesp.flow34 = tmp(:,3);
% Tali([1:376 3290:height(Tali)],:) = [];
% Tali.time_ms = (173900/1450) .* transpose((1:height(Tali)));

% Tali.time_ms = Tesp;


% Tesp.flow33 = str2num(Tesp.flow33);




%%
flows = transpose(Tesp.flow33);
times = transpose(Tesp.time_ms);
figure(27)
yline(mean(Tali.mflow))
hold on;
titles = [string(mean(Tali.mflow))];
for n = (1:5:21)
    cutoff = floor(length(flows)/n);
    flows1 = flows(1:cutoff*n);
    times1 = times(1:cutoff*n);
    flows2 = reshape(flows1, [], n);
    times2 = reshape(times1, [], n);
    flows3 = mean(flows2, 1);
    times3 = mean(times2, 1);
    titles = [titles, n]
    plot(times3, flows3);
end
hold off;
legend(string(titles))



%% Uncomment below to get error plots

% Talimatch33 = Tali.mflow(947:2568);
% Talimatch34 = Tali.mflow(64:857);
% 
% Tespmatch33 = Tesp.flow33(947:2568);
% Tespmatch34 = Tesp.flow34(64:857);
% 
% keepme33 = Talimatch33~=0 & Talimatch33>50 & Talimatch33<250 & Tespmatch33~=0;
% keepme34 = Talimatch34~=0 & Tespmatch34~=0 & Talimatch34>5;
% 
% Tali33mflowCut = Talimatch33(keepme33);
% Tali34mflowCut = Talimatch34(keepme34);
% 
% 
% Tesp33no0 = Tespmatch33(keepme33);
% Tesp34no0 = Tespmatch34(keepme34);
% 
% x33 = Tali33mflowCut;
% y33 = (Tesp33no0-Tali33mflowCut)./Tali33mflowCut*100;
% 
% x34 = Tali34mflowCut;
% y34 = (Tesp34no0-Tali34mflowCut)./Tali34mflowCut*100;
% 
% 
% x33 = Tali.mflow
% y33 = Tesp.flow33
% 
% %% Plotting
% figure(65)
% plot(Tali.mflow)
% % plot(Tali.Var1,Tali.Var4)
% hold on;
% plot(Tesp.flow33)
% plot(Tesp.flow34)
% hold off;
% legend("Alicat", "3300", "3400")
% grid on;
% grid minor;
% 
% figure(66)
% plot(Talimatch33)
% % plot(Tali.Var1,Tali.Var4)
% hold on;
% plot(Tespmatch33)
% hold off;
% legend("Alicat", "3300")
% grid on;
% grid minor;
% title("SFM3300 in Series with Alicat 250 SLPM")
% 
% 
% figure(67)
% times = gettimes(samp_per,Tespmatch34,"min");
% plot(times, Talimatch34)
% % plot(Tali.Var1,Tali.Var4)
% hold on;
% plot(times,Tespmatch34)
% hold off;
% legend("Alicat", "3400")
% grid on;
% grid minor;
% xlabel("Time (s)")
% title("SFM3400 in Series with Alicat 250 SLPM")
% 
% figure(60)
% plot(y33);
% % plot((Tesp.flow33(100:2500)-Tali.mflow(100:2500))/Tali.mflow(100:2500)*100);
% grid on;
% grid minor;
% ylabel("% Error")
% xlabel("Run Time")
% title("3300 Error Compared to Alicat")
% 
% figure(75)
% plot(y34);
% grid on;
% grid minor;
% ylabel("% Error")
% xlabel("Run Time")
% title("3400 Error Compared to Alicat")
% 
% 
% figure(83)
% scatter(x33, y33);
% hold on;
% Fit33 = polyfit(x33, y33, 4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
% plot(x33, polyval(Fit33,x33))
% yline([10 -10],"--");
% ylim([-25 25]);
% xlim([0 250])
% hold off;
% grid on;
% grid minor;
% title("Error of SFM3300 vs True Flow Rate")
% ylabel("% Error")
% xlabel("Alicat Mass Flow Reading (SLPM)")
% 
% figure(84)
% scatter(x34, y34);
% hold on;
% Fit = polyfit(x34, y34,4); % x = x data, y = y data, 1 = order of the polynomial i.e a straight line 
% plot(x34, polyval(Fit,x34))
% yline([10 -10],"--");
% ylim([-25 25]);
% xlim([0 40])
% hold off;
% grid on;
% grid minor;
% title("Error of SFM3400 vs True Flow Rate")
% ylabel("% Error")
% xlabel("Alicat Mass Flow Reading (SLPM)")

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
% 
% function times = gettimes(samp_period, tab, secmin)
%     times = samp_period .* (1:height(tab));
%     if (secmin == "min")
%         times = times ./ 60;
%     end
% end
