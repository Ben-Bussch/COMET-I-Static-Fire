# -*- coding: utf-8 -*-
"""
Created on Thu Mar 26 01:26:47 2026

@author: Bobke
"""

import serial
import time
from datetime import datetime
import csv

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
        csv_writer.writerow(['Timestamp', 'Data'])  # Write the header row
        csvfile.flush()
        try:
            while True:
                data = ser.readline().decode().strip() #'utf-8', errors='ignore'
                if data and timestamp:
                    now = datetime.now()
                    formatted_timestamp = now.strftime('%H:%M:%S') + '.{:04d}'.format(int(now.microsecond / 1000))
                    csv_writer.writerow([formatted_timestamp, data])
                    #print(f'{formatted_timestamp} > {data}')   # Optional: monitor in terminal.
        
        except KeyboardInterrupt:
            print("Serial reading stopped by user. (Ctrl-c in terminal)")


if __name__ == '__main__':

    readserial('COM9', 9600, timestamp=True)