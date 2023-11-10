% Clinic.m
% This file takes in the raw data from the Rasberry pi and plots the
% voltage output
clear;
clf;

rawdata = readmatrix('flow_run1.csv');
time = rawdata(:,1)/1000;
% Digital to Analog
res = 4.88;
data = rawdata(:,2:end)*4.88;


% Plot data 
figure(1);
plot(time, data(:,3));
title('Raw data output: Thermistor')
xlabel('time')
ylabel('mV')

figure(2);
plot(time, data(:,4));
title('Raw data output: Diffential Pressure')
xlabel('time')
ylabel('mV')

figure(3)
plot(time, data(:,5))
title('Raw data output: Absolute Pressure')
xlabel('time')
ylabel('mV')

figure(4)
plot(time, data(:,6))
title('Raw data output: SFM3000')
xlabel('time')
ylabel('mV')



