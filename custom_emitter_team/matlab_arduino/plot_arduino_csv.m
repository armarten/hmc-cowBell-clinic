clear;
close all hidden;

file_name = '03-24_coolterm_parallel_alicat_2'; % Input file name from arduino_output_save folder
name_note = "";
figlist = [];

% for .csv
% T = table_from_csv(append("arduino_output_save/",file_name));

%% for CoolTerm .txt files

setpoint = transpose([0.05, 0.05, 0.034, 1, 0.34, 0.34, 0.05]);
sp_notes = transpose(["ss", "tr", "tr", "tr", 'tr', "ss-err", "ss-old"]);

samples = [];
sum_mfr = [];
tr_rise_start = [];
tr_rise_end = []; % time to get to 90% of set point


for i = 1:length(setpoint)

    fullFileName = append('arduino_output_save/', '03-24_coolterm_parallel_alicat_', num2str(i),".txt");
    T = readtable(fullFileName);


    
    if i == 1
        T_cut = T;
        T_ss = T_cut;
    elseif i == 2
        T_cut =  T(1:1040,:);
        T_ss = T_cut(857:end,:);
    elseif i == 3
        T_cut =  T(1:2585,:);
        T_ss = T_cut(1065:end,:);
    elseif i == 4
        T_cut = T;
        T_ss = T_cut(469:end,:);
    elseif i == 5
        T_cut = T;
        T_ss = T_cut(653:end,:);
    elseif i == 6
        T_cut = T;
        T_ss = T_cut;
    elseif i == 7
        T_cut = T;  
        T_ss = T_cut;
    end
    
    plots(T_cut,i, transpose(setpoint));

%   run average


    i;
    setpoint(i);
    samples = [samples ; height(T_ss)];
    sum_mfr = [sum_mfr ; sum(T_ss.mfr)];

    try
        rise_start = find(T_cut.mfr == 0.0002);
        rise_start = rise_start(1);
    catch
        rise_start = NaN;
    end
%     
%     try
%         rise_end = find(T_cut.mfr == round(0.9*setpoint(i),4));
%         rise_end = rise_end(1);
%     catch
%         tr_start_time
%         rise_end = NaN;
%     end

    [ d, rise_end ] = min( abs( T_cut.mfr - round(0.9*setpoint(i),4) ) );
    
    tr_rise_start = [tr_rise_start ; rise_start];
    tr_rise_end = [tr_rise_end ; rise_end];

  
end


transient_check = [];
for j = transpose(sp_notes)
    if contains(j, "tr")
        transient_check = [transient_check; 1];
    else
        transient_check = [transient_check; NaN];
    end
end


timestep = 0.05; % 500 ms
total_time_s = timestep .* samples
total_time_m = total_time_s ./ 60
expected_emissions_sl = total_time_m .* setpoint
actual_emissions_sl = sum_mfr .* (timestep ./ 60)
emissions_error_percent  = ((actual_emissions_sl - expected_emissions_sl) ./ expected_emissions_sl) .* 100
emissions_error_sl = actual_emissions_sl - expected_emissions_sl
tr_rise_end = transient_check.*tr_rise_end
rise_time_s = (tr_rise_end - tr_rise_start) .* timestep



run_name = string(setpoint)+"-"+sp_notes

T_results = table(setpoint, run_name, total_time_s, total_time_m, expected_emissions_sl, actual_emissions_sl, emissions_error_percent, emissions_error_sl, rise_time_s, transient_check)

figure(52)
scatter((1:height(T_results)),T_results.emissions_error_percent,'filled');
text((1:height(T_results)).*1.015,T_results.emissions_error_percent.*1.01,run_name);
xlabel("Data collection round (Figure Number)");
ylabel("Percent Error in Total Emissions");
yline(0)
title("Percent Error in Total Emissions for Various Set Points");


figlist = [];

% plots(T)
% save_plots(figlist, append(file_name, "_", name_note))
save_plots(figlist, append("03-24_coolterm_parallel_alicat", "_", "first_parallel_flow_test"))


results_table_file_name = "parallel_flow_test_results";
results_file_location = append("arduino_plot_figs/",results_table_file_name,".csv");
csv_file = fopen(results_file_location, 'w');
writetable(T_results,results_file_location,'Delimiter',',','QuoteStrings','all')



% scatter_error_compare

%%
function plots(T, i, setpoint)
    % Plots from table
    figure(i);
%         plot((T.time_ms-min(T.time_ms))/1000,T.avgd_flowrate); % table headers change on Arduino file (sfm3300_read vs Pcontorlfrfr)
%          hold on;
%          yline(mean(T.desired_flowrate));
% % %         plot(T.time_ms,T.avgd_flowrate);
%          hold off;
%         fig_title = "Proportional Control with SFM3400 and SS-4MG valve";
% %         fig_title = append("System Response with Proportional Control at 5 slm, K_p = ",num2str(mean(T.kp)));
%         title(fig_title);
%         xlabel("Time (s)");
%         ylim([0.9*min(T.avgd_flowrate) 1.1*mean(T.desired_flowrate)])
%         ylabel("Mass Flow Rate (slm)");
%         legend();
%     figure(40);
%         plot(T.time_ms(5:end), T.avgd_flowrate(5:end));
%      scatter3(T.min_flow_at_10_error_slm,T.max_flow_rate,T.cost,"filled");
%      xlabel("Min flow rate at 10% error (slm)");
%      ylabel("Max flow rate (slm)")
%      text(T.min_flow_at_10_error_slm,T.max_flow_rate,T.cost,T.model)  %<---- play with this      
%      zlabel("Cost, $USD");
% Parallel flow code output
%     plot(T.time_ms, T.avg_flow)

% CoolTerm
%     print(setpoint(i))
    plot((1:height(T.mfr))*0.05,T.mfr)
    hold on;
    yline(setpoint(i));
    yline([1.1*setpoint(i), 0.9*setpoint(i)],':')
    hold off;
    title("Parallel Flow Output Measured by Alicat 1 SLPM Mass Flow Meter")
    ylabel("Alicat Flow Reading (SLPM)")
    xlabel("Time (s)")
%     ylim([0.035 0.065])
    legend("MFC Output", "Set point", "+10% of set point", "-10% of set point", "Location","best")



end


% function error = long_run_error(target, measured)
%     error = 0*target*measured;
% end

function T = table_from_csv(file_name)
    % Returns a table with the contents and column titles of file_name.csv
        % in the folder called 'arduino_output_save'
%     file_location = append('arduino_output_save/',file_name,'.csv');
     file_location = append(file_name,'.csv');

    % rdtbl = readtable(file_location);
    dead_rows = get_header_length(file_location); % Number of rows before column headers
    T = readtable(file_location,'NumHeaderLines',dead_rows);  % skips the first n rows of data
return
end


function dead_rows = get_header_length(file_location)
    % Returns row number of the first instance of the string "data_begin" in csv  
    mat_data_test = readcell(file_location);  % skips the first n rows of data
    dead_rows = find(strcmp(mat_data_test, 'data_begin'));
end


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