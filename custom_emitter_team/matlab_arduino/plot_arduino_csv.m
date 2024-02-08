clear;
close all hidden;

file_name = '07-Feb-2024-13-24-22_ppp_sweep'; % Input file name from arduino_output_save folder
name_note = "ppp_sweep";
figlist = [];

T = table_from_csv(file_name);
plots(T)
save_plots(figlist, append(file_name, "_", name_note))

%%
function plots(T)
    % Plots from table
    figure(30);
        plot(T.time_ms,T.avg_flow); % table headers change on Arduino file (sfm3300_read vs Pcontorlfrfr)
%         hold on;
% %         yline(mean(T.desired_flowrate));
% %         plot(T.time_ms,T.avgd_flowrate);
%         hold off;
        fig_title = "SLM through SFM3300 sweep across PPP valve full range";
%         fig_title = append("System Response with Proportional Control at 5 slm, K_p = ",num2str(mean(T.kp)));
        title(fig_title);
        xlabel("Time (ms)");
        ylabel("Mass Flow Rate (slm)");
%         legend();
%     figure(40);
%         plot(T.time_ms(5:end), T.avgd_flowrate(5:end));
end

function T = table_from_csv(file_name)
    % Returns a table with the contents and column titles of file_name.csv
        % in the folder called 'arduino_output_save'
    file_location = append('arduino_output_save/',file_name,'.csv');
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