clear;
close all hidden;

file_names = ["parallel_100_alicat250"];
% comb_data = table(["pres";"tempc";"vflow";"mflow";"comp"]);

% Define column headers
columnHeaders = ["pres"; "tempc"; "vflow"; "mflow"; "comp"];

% Create an empty cell array with the same number of columns
emptyData = cell(1, numel(columnHeaders));

% Create the empty table
comb_data = table(emptyData{:}, 'VariableNames', columnHeaders);

for j = (1:length(file_names))
%     file_name = 'inline_parallel_250SLPM_2'; % Input file name from arduino_output_save folder
    file_name = file_names(j)
    name_note = "";
    figlist = [];
    table_data_mfc = table_from_csv(append("arduino_output_save/",file_name));
    
    
    table_data_mfc_dropped = table_data_mfc(:,[1:4,6]);
    table_data = renamevars(table_data_mfc_dropped,["Var6"],["Var5"]);
    table_data = renamevars(table_data,["Var1", "Var2", "Var3", "Var4", "Var5"],["pres", "tempc", "vflow", "mflow", "comp"]);
    comb_data = [comb_data ; table_data]

end

% table_data_mfc = table_from_csv(append("arduino_output_save/",file_name));
% table_data_mfc2 = table_from_csv(append("arduino_output_save/",file_name));


% table_data_mfc_dropped = table_data_mfc(:,[1:4,6]);
% table_data = renamevars(table_data_mfc_dropped,["Var6"],["Var5"]);
% table_data = renamevars(table_data,["Var1", "Var2", "Var3", "Var4", "Var5"],["pres", "tempc", "vflow", "mflow", "comp"]);

% problem_rows = [1:3440,29312:40732,51455:69263];
% table_data(problem_rows,:) = [];
samp_period = 50E-3; % s
% samp_rate = 1 / samp_period;
times = samp_period .* (1:height(comb_data));
figure(70)
plot(times, comb_data.mflow, "LineWidth",3)
hold on;
yline(100,"Color","red")
hold off;
grid on;
grid minor;
ylabel("Mass Flow Rate (SLPM)")
xlabel("Time (s)")
% 
% rows_50 = [1587:8063];
% rows_100  = [8636:15178];
% rows_150   = [15996:22673];
% rows_100    = [26713:27935, 28091:28986];


% table_50 = table_data(rows_50,:);
% table_100 = table_data(rows_100,:);
% table_150 = table_data(rows_150,:);
% table_27_5 = table_data(rows_27_5,:);
% table_100 = table_data(rows_100,:);

% tab_struct = struct('table',{table_50, table_100, table_150});
% setpoints = [50 100 150];


% plot all subtables
% for (i = (1:length(setpoints)))
% tab = tab_struct(i).table
% figure(i)
% plot(tab.mflow)
% end
%%%%
% 
% runningErrs = [];
% instErrs = [];
% minErrs = [];
% maxErrs = [];
% 
% for i = (1:length(setpoints))
%     tab = tab_struct(i).table;
%     setpoint = setpoints(i)
%     avgFlow = mean(tab.mflow);
%     errorOffset = avgFlow - setpoints(i)
%     errorRatio = errorOffset / setpoints(i)
%     errorPerc = errorRatio * 100
%     runningErr = runningError(tab, setpoint)
%     [minErr, maxErr] = minmaxErr(tab, setpoint)
% 
%     runningErrs = [runningErrs runningErr]
%     instErrs = [instErrs errorRatio]
%     minErrs = [minErrs minErr]
%     maxErrs = [maxErrs maxErr]
% end
%%%%%

% mean(table_0_0275.mflow)
% mean(table_0_275.mflow)
% mean(table_27_5.mflow)




%    dim = size(table_data.pres);
%    table_data.setpoint = ones(dim)*35;
%    table_data.error = (table_data.total - table_data.setpoint)./table_data.setpoint;
    
%%%5
% plots(table_data, setpoints, runningErrs, instErrs, minErrs, maxErrs)
% % plots(table_data);
% save_plots(figlist, append(file_name, "_", name_note));

%%
function instErr = instErr(tab, setpoint)
    avgFlow = mean(tab.mflow);
    errorOffset = avgFlow - setpoint;
    errorRatio = errorOffset / setpoint;
    errorPerc = errorRatio * 100;
    instErr = errorRatio;
end

function [minErr, maxErr] = minmaxErr(tab, setpoint)
    minFlow = min(tab.mflow)
    maxFlow = max(tab.mflow)
    minErr = (minFlow - setpoint) / setpoint;
    maxErr = (maxFlow - setpoint) / setpoint;
end

function runningErr = runningError(tab, setpoint)
    samp_period = 50E-3/60; % s
%     samp_rate = 1 / samp_period;
%     times = samp_period .* (1:height(tab));
    actualEmissions = sum(tab.mflow * samp_period);
    predictedEmissions = (sum(ones(height(tab.mflow),1)) - 1) * setpoint * samp_period;
    runningErr = ( actualEmissions - predictedEmissions ) / predictedEmissions;
end

function plots(T, setpoints, runningErrs, instErrs, minErrs, maxErrs)
% function plots(T)
    % Plots from table
    
    samp_period = 50E-3; % ms
    samp_rate = 1 / samp_period;
    times = samp_period .* (1:height(T));
    figure(30);
%     plot(times, T.mflow, 'LineWidth', 3);
      plot(T.mflow);
      grid on;
      grid minor;
%     hold on;
%     plot(times, T.vflow);
%     hold off;
    legend("mass flow")
    ylabel("Mass Flow Rate (SLPM)")
    xlabel("row")
    

    figure(50)
    sz = 10;
    lw = 2;
    cbGreen = [76 158 112]./255;
    cbBlue = [56 63 186]./255;
%     scatter(setpoints, instErrs * 100, 60, 'b','o','filled');
    scatter(setpoints, runningErrs .* 100, sz, 'o','MarkerEdgeColor',cbGreen,'MarkerFaceColor',cbGreen);
    hold on;
    errorbar(setpoints, instErrs .* 100, minErrs.* 100, maxErrs.* 100,'o','MarkerEdgeColor',cbBlue,'MarkerFaceColor',cbBlue, 'MarkerSize', sz, 'LineWidth', lw);

    set(gca, 'XScale','log')
%     semilogx(setpoints, instErrs .* 100,'o','MarkerEdgeColor',cbBlue,'MarkerFaceColor',cbBlue, 'MarkerSize', sz, 'LineWidth', lw);

%     scatter(setpoints, runningErrs * 100, 60, 'r','o','filled');
    yline([10, -10],"--")
    ylim([-12 1.3*max(maxErrs)*100])
    hold off;
    legend("Running Error", "Average Instantaneous Error", "Max Error Constraint")
    xlabel("Mass Flow Rate (SLPM)")
    ylabel("% Error")
    grid on;

 
%     figure(34)
%     time_steady = T.time_ms(10:end,:);
%     error_steady = T.error(10:end,:);
%     plot(time_steady, error_steady*100)
%     xlabel("Time (ms)")
%     ylabel("% Error")
% 
%     mean(abs(error_steady*100))
% 
%     figure(44)
%     plot(T.time_ms, T.error*100)
%     xlabel("Time (ms)")
%     ylabel("% Error")

end


function fancy_plots(T, setpoints, runningErrs, instErrs)
    % Plots from table
    
    samp_period = 50E-3; % ms
    samp_rate = 1 / samp_period;
    times = samp_period .* (1:height(T));
    figure(30);
    fig1_comps.fig = gcf;
    fig1_comps.t1 = tiledlayout(fig1_comps.fig, 1, 2);
    fig1_comps.n(1) = nexttile;

    fig1_comps.p1 = plot(times, T.mflow);

    legend(fig1_comps.p1, "mass flow")
    fig1_comps.tile1.plotYLabel = ylabel("Mass Flow Rate (SLPM)");
    fig1_comps.tile1.plotXLabel = xlabel("row");
    

%     figure(50)
%     sz = 20;
%     scatter(setpoints, instErrs * 100, 60, 'b','o','filled');
    fig1_comps.n(2) = nexttile;
    fig1_comps.p2 = semilogx(setpoints, runningErrs .* 100,'o','MarkerEdgeColor',[1 0 0], 'MarkerSize', sz);

    hold on;
    fig1_comps.p2 = semilogx(setpoints, instErrs .* 100,'o','MarkerEdgeColor',[0 0.447 0.741], 'MarkerSize', sz);

%     scatter(setpoints, runningErrs * 100, 60, 'r','o','filled');
    fig1_comps.p2 = yline([10, -10],"--");
    ylim(gca, [-12 12])
    hold off;
    legend(fig1_comps.p1, "Running Error", "Average Instantaneous Error", "Max Error Constraint")
    fig1_comps.tile2.plotXLabel = xlabel("Mass Flow Rate (SLPM)")
    fig1_comps.tile2.plotYLabel = ylabel("% Error")
    grid on;
    
    
    STANDARDIZE_FIGURE(fig1_comps);


    % SET PLOT PROPERTIES
    % The following command will set the properties for p1.
    % Choices for Colors can be found in in PLOT_STANDARDS
    set(fig1_comps.p1, 'LineStyle', 'none', 'Marker', 'o', 'MarkerSize', 6, 'MarkerEdgeColor', PS.Blue4, 'MarkerFaceColor', PS.Blue1);
    set(fig1_comps.p2, 'LineStyle', '--', 'LineWidth', 2, 'Color', PS.Red2);
%     set(fig1_comps.p3, 'LineStyle', 'none', 'Marker', 'o', 'MarkerSize', 6, 'MarkerEdgeColor', PS.Blue4, 'MarkerFaceColor', PS.Blue1);
%     set(fig1_comps.p4, 'LineStyle', '--', 'LineWidth', 2, 'Color', PS.Red2);


 
%     figure(34)
%     time_steady = T.time_ms(10:end,:);
%     error_steady = T.error(10:end,:);
%     plot(time_steady, error_steady*100)
%     xlabel("Time (ms)")
%     ylabel("% Error")
% 
%     mean(abs(error_steady*100))
% 
%     figure(44)
%     plot(T.time_ms, T.error*100)
%     xlabel("Time (ms)")
%     ylabel("% Error")

end


function T = table_from_csv(file_name)
    % Returns a table with the contents and column titles of file_name.csv
        % in the folder called 'arduino_output_save'
%     file_location = append('arduino_output_save/',file_name,'.csv');
     file_location = append(file_name,'');

    % rdtbl = readtable(file_location);
%     dead_rows = get_header_length(file_location); % Number of rows before column headers
    T = readtable(file_location); 
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


