clear;
close all hidden;

file_name = '21-Mar-2024-01-23-20_parallel_test'; % Input file name from arduino_output_save folder
name_note = "";
figlist = [];

table_data = table_from_csv(append("arduino_output_save/",file_name));

   dim = size(table_data.time_ms);
   table_data.setpoint = ones(dim)*35;
   table_data.error = (table_data.total - table_data.setpoint)./table_data.setpoint;
    

plots(table_data)
save_plots(figlist, append(file_name, "_", name_note));

%%
function plots(T)
    % Plots from table
    figure(30);
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
    plot(T.time_ms, T.flow_3300);
    hold on;
    plot(T.time_ms, T.total);
    plot(T.time_ms, T.setpoint)
    hold off;
%     yline(35)
    ylim([0 40])
    legend("Rough valve", "Total", "Set point", 'Location', 'SouthEast')
    ylabel("Flow Rate (SLPM)")
    xlabel("Time (ms)")

 
    figure(34)
    time_steady = T.time_ms(10:end,:);
    error_steady = T.error(10:end,:);
    plot(time_steady, error_steady*100)
    xlabel("Time (ms)")
    ylabel("% Error")

    mean(abs(error_steady*100))

    figure(44)
    plot(T.time_ms, T.error*100)
    xlabel("Time (ms)")
    ylabel("% Error")

end

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


