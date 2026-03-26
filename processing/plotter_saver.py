import serial
import time
from datetime import datetime
import csv
import matplotlib.pyplot as plt
from collections import deque
import numpy as np

p_o_vals = deque(maxlen=1000)
p_i_vals = deque(maxlen=1000)
timestamps = deque(maxlen=1000)

PLOT_INTERVAL = 0.1  # seconds 

plt.ion()
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 6), sharex=True)

# --- Raw data plot ---
line1, = ax1.plot([], [], label='P_N2O', color='blue')
line2, = ax1.plot([], [], label='P_IPA', color='red')
ax1.set_ylabel("Pressure [bar]")
ax1.set_xlim(0, 150)
ax1.legend()
ax1.grid(True)

# --- Moving average plot ---
ma_window = 20
line1_ma, = ax2.plot([], [], label='P_N2O MA', color='blue')
line2_ma, = ax2.plot([], [], label='P_IPA MA', color='red')
ax2.set_xlabel("Time [s]")
ax2.set_ylabel("Pressure MA [bar]")
ax2.set_xlim(0, 150)
ax2.legend()
ax2.grid(True)

plt.show(block=False)

t_fill_exp = 900  # s


def generate_output_filename():
    now = datetime.now()
    return now.strftime("output_%Y%m%d_%H%M%S.csv")


def readserial(comport, baudrate, timestamp=False):

    ser = serial.Serial(comport, baudrate, timeout=None)
    time.sleep(2)
    ser.flushInput()

    last_plot_time = time.time()

    output_csv = generate_output_filename()
    with open(output_csv, 'w', newline='') as csvfile:
        csv_writer = csv.writer(csvfile)

        csv_writer.writerow([
            'Timestamp', 't [ms]', 'Mode',
            'Launch_Time [s]', 'Fill_Time [s]',
            'P_o [Bar]', 'P_i [Bar]'
        ])

        mode = None
        launch_time = None
        fill_time = None

        try:
            while True:
                data = ser.readline().decode(errors='ignore').strip()

                if not data:
                    continue

                # --- Metadata ---
                if data.startswith("Mode:"):
                    try:
                        mode = data.split(":")[1].strip()
                    except ValueError:
                        mode = None

                    if launch_time == "STANDBY" or not launch_time:
                        print("Mode:", mode)
                    elif mode == "ABORT":
                        print("Mode:", mode)

                elif data.startswith("ABORT IN:"):
                    try:
                        launch_time = int(data.split(":")[1].strip().split()[0])
                    except ValueError:
                        launch_time = None

                    fill_time = "STANDBY"
                    print("ABORT IN:", launch_time)

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
                    print("Fill time:", fill_time, "/", t_fill_exp, "s")

                # --- Main data ---
                elif "_o" in data:
                    now = datetime.now()
                    timestamp = now.strftime('%H:%M:%S') + '.{:04d}'.format(int(now.microsecond / 1000))

                    try:
                        values = {}
                        for item in data.split(','):
                            if ':' in item:
                                key, val = item.split(':', 1)
                                values[key.strip()] = val.strip()

                        p_o = float(values.get('P_o', 0))
                        p_i = float(values.get('P_i', 0))
                        t = float(values.get('t', 0))

                        # --- CSV ---
                        csv_writer.writerow([
                            timestamp,
                            t,
                            mode,
                            launch_time,
                            fill_time,
                            p_o,
                            p_i
                        ])
                        csvfile.flush()

                        # --- DATA APPEND (slightly relaxed condition) ---
                        if (not timestamps) or (t/1000 - timestamps[-1] >= 0.1):

                            timestamps.append(t/1000)
                            p_o_vals.append(p_o)
                            p_i_vals.append(p_i)

                            # --- THROTTLED PLOTTING ---
                            if time.time() - last_plot_time > PLOT_INTERVAL:
                                last_plot_time = time.time()

                                t_list = list(timestamps)
                                p_o_list = list(p_o_vals)
                                p_i_list = list(p_i_vals)

                                # Raw
                                line1.set_data(t_list, p_o_list)
                                line2.set_data(t_list, p_i_list)

                                # Moving average
                                if len(p_o_vals) >= ma_window:
                                    kernel = np.ones(ma_window) / ma_window
                                    p_o_ma = np.convolve(p_o_list, kernel, mode='valid')
                                    p_i_ma = np.convolve(p_i_list, kernel, mode='valid')
                                    t_ma = t_list[ma_window-1:]

                                    line1_ma.set_data(t_ma, p_o_ma)
                                    line2_ma.set_data(t_ma, p_i_ma)

                                if len(timestamps) > 1:
                                    window_size = 150  # seconds
                                    current_time = timestamps[-1]

                                    ax1.set_xlim(current_time - window_size, current_time)
                                    ax2.set_xlim(current_time - window_size, current_time)

                                # Autoscale
                                ax1.relim()
                                ax1.autoscale_view(scalex=False, scaley=True)
                                ax2.relim()
                                ax2.autoscale_view(scalex=False, scaley=True)

                                fig.canvas.draw_idle()
                                fig.canvas.flush_events()
                                plt.pause(0.001)

                    except Exception as e:
                        print("Error:", e)

        except KeyboardInterrupt:
            ser.close()
            print("Serial reading stopped by user.")


if __name__ == '__main__':
    readserial('/dev/ttyACM0', 9600, timestamp=True)