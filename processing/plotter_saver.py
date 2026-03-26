# -*- coding: utf-8 -*-
"""
Created on Thu Mar 26 01:26:47 2026

@author: Bobke
"""

import serial
import time
from datetime import datetime
import csv
import matplotlib.pyplot as plt
from collections import deque

p_o_vals = deque(maxlen=1000)
p_i_vals = deque(maxlen=1000)
timestamps = deque(maxlen=1000)


plt.ion()  # interactive mode
fig, ax = plt.subplots()
line1, = ax.plot([], [], label='P_N2O')
line2, = ax.plot([], [], label='P_IPA')
ax.set_xlabel("Time")
ax.set_ylabel("Pressure")
ax.legend()

t_fill_exp = 900 #s

def generate_output_filename():
    now = datetime.now()
    # Create a datetime string suitable for a filename, e.g., "output_20230405_123456.csv"
    filename = now.strftime("output_%Y%m%d_%H%M%S.csv")
    return filename

def readserial(comport, baudrate, timestamp=False):

    ser = serial.Serial(comport, baudrate, timeout=None)         # 1/timeout is the frequency at which the port is read
    time.sleep(2)  # give the arduino time to start.
    ser.flushInput()
    
    
    output_csv = generate_output_filename()  # Generate filename with current datetime
    with open(output_csv, 'w', newline='') as csvfile:
        csv_writer = csv.writer(csvfile)
        # Write header once
        # Write header
        csv_writer.writerow(['Timestamp', 't [ms]', 'Mode', 'Launch_Time [s]', 'Fill_Time [s]', 'P_o [Bar]', 'P_i [Bar]'])
        
        mode = None
        launch_time = None
        fill_time = None
        try:
            while True:
                data = ser.readline().decode().strip()
            
                if not data:
                    continue
            
                # --- Handle metadata lines ---
                if data.startswith("Mode:"):
                    try:
                        mode = data.split(":")[1].strip()
                    except ValueError:
                        mode = None
                    
                    if launch_time == "STANDBY" or not launch_time:
                        print("Mode:", mode)
            
                elif data.startswith("Launch Time:"):
                    try:
                        launch_time = int(data.split(":")[1].strip().split()[0])
                    except ValueError:
                        launch_time = None
                    
                    fill_time = "STANDBY"
                    print("Launch time:", launch_time)
            
                elif data.startswith("Fill Time:"):
                    try:
                        fill_time = int(data.split(":")[1].strip().split()[0])
                    except ValueError:
                        fill_time = None
                    launch_time = "STANDBY"
                    print("Fill time:",fill_time, " / ", t_fill_exp, " s")
            
                # --- Handle main data line ---
                elif "P_o" in data:
                    now = datetime.now()
                    timestamp = now.strftime('%H:%M:%S') + '.{:04d}'.format(int(now.microsecond / 1000))
                    
                    try:
                        values = {}
                        for item in data.split(','):
                            for item in data.split(','):
                                if ':' in item:
                                    key, val = item.split(':', 1)
                                    values[key.strip()] = val.strip()
                        
                        
                        p_o = float(values.get('P_o', 0))
                        p_i = float(values.get('P_i', 0))
                        t = float(values.get('t', 0))
                        csv_writer.writerow([
                            timestamp,
                            t,
                            mode,
                            launch_time,
                            fill_time,
                            p_o,
                            p_i
                        ])
                                    
                        csvfile.flush()  # safer logging
    
                        
                        p_o_vals.append(p_o)
                        p_i_vals.append(p_i)
                        timestamps.append(t/1000)
                        
                        # --- THEN update plot ---
                        line1.set_data(timestamps, p_o_vals)
                        line2.set_data(timestamps, p_i_vals)
                    except:
                        plt.pause(0.01)
                    """
                    if len(timestamps) % 50 == 0:
                        ax.relim()
                        ax.autoscale_view()
                    plt.draw()
                    plt.pause(0.01)
                    """
        except KeyboardInterrupt:
            ser.close()
            print("Serial reading stopped by user. (Ctrl-c in terminal)")
            

if __name__ == '__main__':

    readserial('COM5', 9600, timestamp=True)