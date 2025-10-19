%% Replay GPS NMEA data over serial port
% This script reads GPS NMEA data from a log file (as recorded by the
% SerialPortGPSApp example app) and writes it to a serialport with the same
% pacing as it was recorded at. See usage instructions in the "Create App
% to Display and Log Data from Serial Port GPS Device" example.
%
% Press any key to stop transmitting data while the script is running.
%

% Copyright 2024 The MathWorks, Inc.

%% Specify serial port, data file, and write parameters

% Specify serial port name and baud rate
serialPortName = "COM4";
% The baud rate should match the value used in the SerialPortGPSApp example
baudRate = 9600;

% Specify GPS NMEA log file name
fileName = "GPS_data_raw.txt";

% Specify data transmission time step (s)
dt = 20e-3;

%% Read and process data

% Read GPS NMEA log file as timetable
data = readtimetable(fileName);

% Create binned time array for transmission
binnedTimeArray = binTime(data.Time,dt);

%% Connect to serial port

% Delete existing serial port connection (if any)
delete(serialportfind(Tag="gpsReplayPort"))

s = serialport(serialPortName,baudRate,Tag="gpsReplayPort");
configureTerminator(s,"CR/LF");

%% Replay data

% Delete existing timer (if any)
delete(timerfind(Tag="gpsReplayTimer"))

% Create and setup timer
t = timer(Period=dt,...
    BusyMode="drop",...
    ExecutionMode="fixedRate",...
    TasksToExecute=numel(binnedTimeArray),...
    TimerFcn=@(src,event) gpsReplayTimerFcn(src,event,s,data,binnedTimeArray),...
    Tag="gpsReplayTimer");

disp("Press any key to stop transmitting." + newline + newline)
start(t)
pause

%% Clean up

% Clear timer
stop(t)
delete(t)
clear t

% Disconnect serialport
clear s

%% Local functions

function gpsReplayTimerFcn(src,~,s,data,binnedTimeArray)
%gpsReplayTimerFcn Timer callback function, which writes data to serialport
%  src is the associated timer object
%  s is a serialport object
%  data is the data timetable containing the timestamps and text lines to write to serialport
%  binnedTimeArray is a cell array with data indices associated with each time step

    taskIndex = src.TasksExecuted;
    lineNumbers = binnedTimeArray{taskIndex};

    if ~isempty(lineNumbers)
        for ii = 1:numel(lineNumbers)
            dataOut = data{lineNumbers(ii),1}{1};
            disp(dataOut)
            writeline(s,dataOut)
        end
    end
end

function binnedTimeArray = binTime(t,dt)
%binTime Bin timestamps into a time step array
%  t is the time array (datetime, duration, or double values)
%  dt is the time step value (s)
%  binnedTimeArray output is a cell array with data indices associated with each time step

    switch class(t)
        case "datetime"
            t = seconds(t-t(1));
        case "duration"
            t = seconds(t);
        case "double"
            % Use t as is
        otherwise
            error("Time array should be a datetime, duration, or double array.")
    end

    % Timestamps, in number of time steps
    ts = round(t/dt);

    % Time step array with associated data indices
    binnedTimeArray = cell(ts(end)+1,1);

    for ii = 1:numel(ts)
        binnedTimeArray{ts(ii)+1} = [binnedTimeArray{ts(ii)+1} ii];
    end

end