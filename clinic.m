% Clinic.m
% This file takes in the raw data from the Rasberry pi and plots the
% voltage output
clear;
clf;

file_name = '2023_11_09__19_15_26__100_hz__10000_n__low_range';
file_location = append('Results/',file_name,'.csv');
rawdata = readmatrix(file_location);
rdtbl = readtable(file_location);

time = rawdata(:,1)/1000;
% Digital to Analog
res = 4.88;
data = rawdata(:,2:end)*4.88;

% Plot data 
figure(1);
plot(time, data(:,3));
title('Raw data output: Thermistor')
xlabel('time (ms)')
ylabel('mV')

figure(2);
plot(rdtbl, "time_ms", "diff_p");
title('Raw data output: Diffential Pressure')
xlabel('time (ms)')
ylabel('mV')

figure(3)
plot(rdtbl, "time_ms", "abs_p");
title('Raw data output: Absolute Pressure')
xlabel('time (ms)')
ylabel('mV')

figure(4)
plot(rdtbl, "time_ms", "sfm3300");
title('Raw data output: SFM3000')
xlabel('time (ms)')
ylabel('slm')


sfm3300_nonzero_check = abs(rdtbl.sfm3300) > 0.0333;
sfm_nonzero = rdtbl.sfm3300(sfm3300_nonzero_check);
diffp_nonzero = rdtbl.diff_p(sfm3300_nonzero_check);


figure(21)
plot(sfm_nonzero)
hold on;
plot(diffp_nonzero)
hold off;

sfm_div_diff = rdtbl.sfm3300 ./ rdtbl.diff_p;
pres_factor = mean(sfm_div_diff);
custom_scaled_diff = rdtbl.diff_p .* sfm_div_diff;
scaled_diff = rdtbl.diff_p * pres_factor;

sfm_div_diff_nonzero = sfm_nonzero ./ diffp_nonzero;
pres_factor_nonzero = mean(sfm_div_diff_nonzero);
scaled_diff_nonzero = rdtbl.diff_p * pres_factor_nonzero;



figure(10)
plot(sfm_div_diff)


figure(20)
plot(rdtbl.time_ms, scaled_diff)
hold on;
plot(rdtbl.time_ms, scaled_diff_nonzero)
plot(rdtbl, "time_ms", "sfm3300")
legend("scaled diff p", "scaled diff nonzero", "sfm3300")
ylabel("slm")
hold off;

% Figlist should be a list of figure numbers. These figures will be saved
% as .mat and .png files to the folder plot_figs. Leave figlist = [] to not
% save any figures.
figlist = [];

save_plots(figlist, file_name)

function save_plots(figlist, file_name)
    if isempty(figlist)
    else
        for fig = figlist
            fig_file_name = append('plot_figs/',file_name,"_fig","_",int2str(fig));
            saveas(fig, fig_file_name, 'png')
            saveas(fig, fig_file_name, 'fig')
        end  
    end
end


