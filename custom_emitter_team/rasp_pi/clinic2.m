% Clinic2.m
% This file takes in a .csv of raw output from Main_spi_i2c_functions.py
% and processes and plots the results
close all hidden;
clear;

%% Set up variables
V_S = 5; % Supply voltage
slm_conversion_factor = 1;

%% Get data from file

% **** Paste in file name ****
file_name = '2023_11_28__16_25_39__100_hz__100_n__samp_rate_check';
% .csv file is in the Results folder
file_location = append('Results/',file_name,'.csv');
% Make a table of the .csv data
rdtbl = readtable(file_location);


%% Find actual sample rate

% Filter time to only values under 1000 ms (1 s)
sr_check = abs(rdtbl.time_ms) <= 1000;
sr_check_array = rdtbl.time_ms(sr_check);
% Check size of that array (samples / second)
sr_actual = height(sr_check_array);

% Check average time between samples
sr_diff = diff(rdtbl.time_ms);
mean_samp_per = mean(sr_diff);
max_samp_per = max(sr_diff);
min_samp_per = min(sr_diff);
figure(34)
plot(sr_diff)
title("Sampling Periods of OPFS Test")
xlabel("Sample")
ylabel("Sampling Period (ms)")

sr_mean = 1 / (min_samp_per/1000);

%% Implement Butterworth filter
nyquist = sr_mean / 2;
cutoff_freq = nyquist*0.1;
% Butterworth filter order
n = 6;

[b, a] = butter(n, cutoff_freq/nyquist);

% freqz(b, a, [], sr_mean)

figure(37)
plot(rdtbl.time_ms, rdtbl.diff_p, 'y')
hold on;

% filtfilt() does forward-backward filtering to remove lag https://dsp.stackexchange.com/questions/26299/zero-lag-butterworth-filtering
butter_pres = filtfilt(b,a,rdtbl.diff_p);

plot(rdtbl.time_ms, butter_pres, 'b')
legend("original", "filtered")
% pass;


%% Create offset for differential pressure values based on background_zeros file
% Paste in file name with the note "background_zeros"
file_name_zeros = '2023_11_09__19_07_20__100_hz__1000_n__background_zeros';
% .csv files in the Results folder
file_location_zeros = append('Results/',file_name_zeros,'.csv');
% Make a table of the .csv data
rdtbl_zeros = readtable(file_location_zeros);

% Convert zeroes value from V to mV and compensate to differential port
% reversal
rdtbl_zeros.diff_p = rdtbl_zeros.diff_p / -1000;
rdtbl_zeros.abs_p = rdtbl_zeros.abs_p / 1000;
% Convert differential pressure sensor offset from mV to kPa
rdtbl_zeros.diff_p = ((rdtbl_zeros.diff_p ./ V_S) - 0.04) / 0.09;
rdtbl_zeros.abs_p = ((rdtbl_zeros.abs_p / V_S) - 0.04) / 0.0012858;

% Caculate differential pressure sensor offset
diffp_offset = mean(rdtbl_zeros.diff_p);
% diffp_offset = 0;

%% Convert temp and pressure sensor mV to V, invert diff_p
rdtbl.diff_p = rdtbl.diff_p / 1000;
rdtbl.abs_p = rdtbl.abs_p / 1000;
rdtbl.temp_0 = rdtbl.temp_0 / 1000;
rdtbl.temp_1 = rdtbl.temp_1 / 1000;
rdtbl.temp_0_1 = rdtbl.temp_0_1 / 1000;

% Save diff_p V values for op amp calculation
diff_p_V = rdtbl.diff_p;

% Invert diff_p because sensor connections are probably switched
rdtbl.diff_p = rdtbl.diff_p * -1;
%% Convert raw pressure sensor voltages to kPa and then to Pa (equations and constants from data sheets)
    % Differential, MPX5010DP, https://www.nxp.com/docs/en/data-sheet/MPX5010.pdf
    % Absolute, MPX5700ASX, https://www.nxp.com/docs/en/data-sheet/MPX5700.pdf
% Equations from data sheets, linked above
rdtbl.diff_p = ((rdtbl.diff_p / V_S) - 0.04) / 0.09;
rdtbl.abs_p = ((rdtbl.abs_p / V_S) - 0.04) / 0.0012858;

% Zero differential pressure sensor values based on offset
rdtbl.diff_p = rdtbl.diff_p - diffp_offset;

% Convert from kPa to Pa
rdtbl.diff_p = rdtbl.diff_p * 1000;
rdtbl.abs_p = rdtbl.abs_p * 1000;

%% Deal with temperature values some other time...

%% Flow rate is proportional to sqrt(diff_p)
% Take sqrt of absolute value of diff_p, to avoid imaginary #s
rdtbl.opfs_flow_unscaled = sqrt(abs(rdtbl.diff_p));

%% Convert SFM3300 values from slm to m3/s
rdtbl.sfm3300 = rdtbl.sfm3300 / slm_conversion_factor;
%% Take zero values out of SFM3300 data and corresponding values out of OPFS data to get better coeff

sfm3300_nonzero_check = abs(rdtbl.sfm3300) > 0.0333/slm_conversion_factor;
sfm_nonzero = rdtbl.sfm3300(sfm3300_nonzero_check);
opfs_flow_nonzero = rdtbl.opfs_flow_unscaled(sfm3300_nonzero_check);

opfs_coeff = mean(sfm_nonzero ./ opfs_flow_nonzero);


rdtbl.opfs_flow_scaled = rdtbl.opfs_flow_unscaled * opfs_coeff;


%% Op Amp calculation, see https://www.desmos.com/calculator/8w1mxwllbe for further plotting

% figure(30)
% plot(rdtbl.opfs_flow_scaled, diff_p_V)
% hold on;
% xlabel("Q, slm")
% ylabel("V")
b = 0.04;
m = 0.09;
% V_out = ( (rdtbl.opfs_flow_scaled./opfs_coeff).^2 ./ V_S - b)./(m);
% V_out = -1* ( ( (rdtbl.opfs_flow_scaled./opfs_coeff).^2.*m*1000) + b) * V_S;
% V_out = -1*(((-1*(rdtbl.opfs_flow_scaled./opfs_coeff).^2 .* 1/1000) + diffp_offset)*m+b)*V_S;
% plot(rdtbl.opfs_flow_scaled, V_out)
% legend("actual", "calculated")
% hold off;

extrapolating_Q = 100;
V_out = -1*(((-1*(extrapolating_Q/opfs_coeff)^2 * 1/1000) + diffp_offset)*m+b)*V_S;


%% To compare SFM3300 value and OPFS value with out a scaling factor, plot them on 2 y axes


figure(10)
yyaxis("left")
plot(rdtbl, "time_ms", "sfm3300")
ylim( [ 0-0.05*max(rdtbl.sfm3300) 1.05*max(rdtbl.sfm3300) ])
ylabel("SFM3300 (slm)")
yyaxis("right")
plot(rdtbl, "time_ms", "opfs_flow_unscaled")
ylim([0-0.05*max(rdtbl.opfs_flow_unscaled) 1.05*max(rdtbl.opfs_flow_unscaled) ])
ylabel("OPFS (sqrt(Pa))")
legend("SFM3300", "OPFS", "Location", "northwest")
title("Volumetric Flow from SFM3300 and Orifice Plate Flow Sensor")


figure(20)
plot(rdtbl, "time_ms", "sfm3300")
hold on;
plot(rdtbl, "time_ms", "opfs_flow_scaled")
plot(rdtbl.time_ms, rdtbl.sfm3300 - rdtbl.opfs_flow_scaled)
ylabel("Volumetric Flow Rate (slm)")
xlabel("Time (ms)")
legend("SFM3300", "OPFS (scaled)","Difference", "Location","northwest")
title("Volumetric Flow from SFM3300 and calibrated Orifice Plate Flow Sensor")
hold off;




%% Save figures to plot_figs
% Figlist should be a list of figure numbers. These figures will be saved
% as .mat and .png files to the folder plot_figs. Leave figlist = [] to not
% save any figures.
figlist = [];

name_note = "samp_rate_test";
save_plots(figlist, append(file_name, name_note))

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




