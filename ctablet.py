import serial
import threading
from collections import deque
import ctypes
import time

# Windows API
SetCursorPos = ctypes.windll.user32.SetCursorPos

# Get Screen Size
ctypes.windll.user32.SetProcessDPIAware()
SCREEN_W = ctypes.windll.user32.GetSystemMetrics(0)
SCREEN_H = ctypes.windll.user32.GetSystemMetrics(1)

# UART init
SERIAL_PORT = "COM4"
BAUD_RATE = 19200
BUFFER_READ_SIZE = 1024
PACKET_SIZE = 6

data_queue = deque()

# jitter
WINDOW_SIZE = 1
EMA_ALPHA = 0.25
MIN_MOVE = 1
MAX_DELTA = 20

x_window = deque(maxlen=WINDOW_SIZE)
y_window = deque(maxlen=WINDOW_SIZE)

prev_mx = None
prev_my = None

# UART
def uart_reader():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0)
        while True:
            data = ser.read(BUFFER_READ_SIZE)
            if data:
                data_queue.extend(data)
            time.sleep(0.00001)
    except serial.SerialException as e:
        print(f"Serial Error: {e}")

def move_mouse(x, y):
    SetCursorPos(x, y)

def main_loop():
    global prev_mx, prev_my
    while True:
        while len(data_queue) >= PACKET_SIZE:
            packet = [data_queue.popleft() for _ in range(PACKET_SIZE)]
            if packet[0] == 0xAA and packet[1] == 0x55:
                x = (packet[2] << 8) | packet[3]
                y = (packet[4] << 8) | packet[5]

                print(f"x:{x}, y:{y}")

                x = max(350, min(650, x))
                y = max(300, min(550, y))

                mx = int(SCREEN_W - (x - 350) / 300 * SCREEN_W)
                my = int((y - 300) / 250 * SCREEN_H)

                x_window.append(mx)
                y_window.append(my)
                avg_mx = int(sum(x_window) / len(x_window))
                avg_my = int(sum(y_window) / len(y_window))

                if prev_mx is None:
                    ema_mx, ema_my = avg_mx, avg_my
                else:
                    ema_mx = int(prev_mx * (1 - EMA_ALPHA) + avg_mx * EMA_ALPHA)
                    ema_my = int(prev_my * (1 - EMA_ALPHA) + avg_my * EMA_ALPHA)

                if prev_mx is not None:
                    if abs(ema_mx - prev_mx) < MIN_MOVE:
                        ema_mx = prev_mx
                    if abs(ema_my - prev_my) < MIN_MOVE:
                        ema_my = prev_my

                if prev_mx is not None:
                    dx = ema_mx - prev_mx
                    dy = ema_my - prev_my
                    if abs(dx) > MAX_DELTA:
                        ema_mx = prev_mx + (MAX_DELTA if dx > 0 else -MAX_DELTA)
                    if abs(dy) > MAX_DELTA:
                        ema_my = prev_my + (MAX_DELTA if dy > 0 else -MAX_DELTA)

                move_mouse(ema_mx, ema_my)
                prev_mx, prev_my = ema_mx, ema_my
            else:
                data_queue.popleft()
        else:
            time.sleep(0.0005)

if __name__ == "__main__":
    reader_thread = threading.Thread(target=uart_reader, daemon=True)
    reader_thread.start()
    main_loop()
