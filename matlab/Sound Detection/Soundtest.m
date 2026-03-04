%-------------------------------------------------------------------------%
%--------------------------- BEATRIX Mic Test ----------------------------%
%-------------------------- Author: Samer Ahmed --------------------------%
%-------------------------------------------------------------------------%
clear
clc
%---------------------------%
%-- Object Initialization --%
%---------------------------%
% Check if the Arduino object already exists
if(~exist('arduinoObj','var'))
% If exists, skip to save time. If not, create it from scratch
arduinoObj = arduino("/dev/cu.usbmodem1301","Uno","Libraries","I2C"); % Arduino object
end
%---------------------------%
%--------- Mic Setup -------%
%---------------------------%
% Microphone setup
mic2 = 'A0'; % 'A1' if you need to test the other microphone
%---------------------------%
%--- Real-Time Operation ---%
%---------------------------%
while true
% Read voltage samples 
samples = zeros(50, 1); %--> Recorded samples initialized to zeros
for i = 1:50
samples(i) = abs(readVoltage(arduinoObj,mic2)-1.25);
end
% Display the mean of the samples' window
disp(mean(samples));
end