%-------------------------------------------------------------------------%
%--------------------------- BEATRIX Mic Test ----------------------------%
%-------------------------------------------------------------------------%
clear
clc

%---------------------------%
%-- Object Initialization --%
%---------------------------%
usb_port = '/dev/cu.usbmodem11401';

if(~exist('arduinoObj','var'))
    arduinoObj = arduino(usb_port,"Uno","Libraries","I2C");
end

%---------------------------%
%--------- Mic Setup -------%
%---------------------------%
mic1 = 'A0';
mic2 = 'A1';

windowSize = 100;

%---------------------------%
%----------- Plot ----------%
%---------------------------%
figure

subplot(2,1,1)
h1 = plot(zeros(windowSize,1));
title("Mic 1 (A0)")
ylabel("Voltage (V)")
ylim([0 2.5])

subplot(2,1,2)
h2 = plot(zeros(windowSize,1));
title("Mic 2 (A1)")
ylabel("Voltage (V)")
xlabel("Sample")
ylim([0 2.5])

%---------------------------%
%------ Real-Time Loop -----%
%---------------------------%
while true

    data1 = zeros(windowSize,1);
    data2 = zeros(windowSize,1);

    for i = 1:windowSize
        data1(i) = readVoltage(arduinoObj,mic1);
        data2(i) = readVoltage(arduinoObj,mic2);
    end

    set(h1,'YData',data1)
    set(h2,'YData',data2)

    drawnow

end