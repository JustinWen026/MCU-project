import pygame
import math
import sys
import serial
import threading
import time
from collections import deque

# UART setting
SERIAL_PORT = "COM4"
BAUD_RATE = 2400
PACKET_SIZE = 6

data_queue = deque()
latest_touch = {"x": None, "y": None}
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0)
waved = 0

def uart_reader():
    try:
        while True:
            data = ser.read(64)
            if data:
                data_queue.extend(data)
            time.sleep(0.00001)
    except Exception as e:
        print("UART Error:", e)

def uart_parser():
    xFiltered = 0
    yFiltered = 0
    xAVG = 0
    yAVG = 0
    count = 0
    while True:
        while len(data_queue) >= PACKET_SIZE:
            pkt = [data_queue.popleft() for _ in range(PACKET_SIZE)]
            if pkt[0] == 0xAA and pkt[1] == 0x55:
                x = (pkt[2] << 8) | pkt[3]
                y = (pkt[4] << 8) | pkt[5]

                x = max(80, min(920, x))
                y = max(100, min(950, y))

                xAVG += x
                yAVG += y

                count += 1
                if count == 10:
                    xAVG /= 10
                    yAVG /= 10

                    xFiltered = xAVG * 0.85 + xFiltered * 0.15
                    yFiltered = yAVG * 0.85 + yFiltered * 0.15

                    # print(f"x:{x}, y:{y}")

                    latest_touch["x"] = xFiltered
                    latest_touch["y"] = yFiltered
                    count = 0
                    xAVG = 0
                    yAVG = 0

            else:
                if len(data_queue) != 0:
                    data_queue.popleft()

def start_uart():
    threading.Thread(target=uart_reader, daemon=True).start()
    threading.Thread(target=uart_parser, daemon=True).start()

# pygame setting
WIDTH, HEIGHT = 600, 800
FPS = 60

LANE_WIDTH = 280
LANE_X = (WIDTH - LANE_WIDTH) // 2

BALL_RADIUS = 20
PIN_RADIUS = 10

pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Bowling Simulation (UART Touch)")
clock = pygame.time.Clock()
font = pygame.font.SysFont(None, 36)
button_font = pygame.font.SysFont(None, 28)

# resistive touchscreen range
TOUCH_X_MIN, TOUCH_X_MAX = 80, 920
TOUCH_Y_MIN, TOUCH_Y_MAX = 100, 950

def map_touch_to_lane(x, y):
    lane_x = LANE_X + (TOUCH_X_MAX - x - 60) / (TOUCH_X_MAX - TOUCH_X_MIN) * LANE_WIDTH
    lane_y = HEIGHT - (y - TOUCH_Y_MIN) / (TOUCH_Y_MAX - TOUCH_Y_MIN) * HEIGHT
    return lane_x, lane_y

def compute_velocity(points):
    vx = vy = 0
    for i in range(len(points) - 1):
        vx += points[i][0] - points[i + 1][0]
        vy += points[i + 1][1] - points[i][1]
    vx /= len(points) - 1
    vy /= len(points) - 1
    return vx, vy

# Bowling Pins
pin_layout = [
    (0, 0),
    (-20, -30),
    (20, -30),
    (-40, -60),
    (0, -60),
    (40, -60),
    (-60, -90),
    (-20, -90),
    (20, -90),
    (60, -90),
]

pin_origin_x = WIDTH // 2
pin_origin_y = 180

# Game status
pins = []
score = 0

ball_x = WIDTH // 2
ball_y = HEIGHT - 50
vel_x = vel_y = 0

touch_points = []
recording = False
ball_fired = False

# reset game
def reset_game():
    global pins, score, ball_x, ball_y, vel_x, vel_y, waved
    global touch_points, recording, ball_fired, data_queue

    data_queue.clear()
    latest_touch["x"] = None
    latest_touch["y"] = None

    ball_x = WIDTH // 2
    ball_y = HEIGHT - 50
    vel_x = vel_y = 0

    touch_points = []
    recording = False
    ball_fired = False

    # Sent L to pic18f4520 through UART
    if waved:
        ser.write(b"L")

    waved = 0

    pins = []
    for dx, dy in pin_layout:
        pins.append(
            {
                "x": pin_origin_x + dx,
                "y": pin_origin_y + dy,
                "vx": 0.0,
                "vy": 0.0,
                "alive": True,
                "collission": False,
            }
        )

    score = 0

reset_game()

# Retry button
retry_rect = pygame.Rect(20, 60, 100, 40)

# start UART
start_uart()

while True:
    clock.tick(FPS)

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            sys.exit()
        if event.type == pygame.MOUSEBUTTONDOWN:
            if retry_rect.collidepoint(event.pos):
                reset_game()

    ty = latest_touch["x"]
    tx = latest_touch["y"]

    print(f"tx:{tx}, ty:{ty}")

    if ty is not None and tx is not None and not ball_fired:

        # start recording
        if ty > 450 and not recording:
            recording = True
            touch_points = []
            print("▶ START RECORD")

        if recording:
            touch_points.append((tx, ty))

        # stop recording and serve the ball
        if recording and ty > 600 and len(touch_points) > 5:
            recording = False
            ball_fired = True
            print("■ END RECORD")

            ball_x, ball_y = map_touch_to_lane(*touch_points[0])
            vel_x, vel_y = compute_velocity(touch_points)

            vel_x *= 0.5
            vel_y *= 1

    # ball motion
    ball_x += vel_x
    ball_y -= vel_y
    vel_x *= 0.995
    vel_y *= 0.995

    if ball_x - BALL_RADIUS < LANE_X or ball_x + BALL_RADIUS > LANE_X + LANE_WIDTH:
        vel_x = 0

    # ball -> pin collision
    for pin in pins:
        if not pin["alive"]:
            continue

        dx = pin["x"] - ball_x
        dy = pin["y"] - ball_y
        dist = math.hypot(dx, dy)

        if dist < BALL_RADIUS + PIN_RADIUS and dist != 0:
            pin["collission"] = True
            nx = dx / dist
            ny = dy / dist
            force = math.hypot(vel_x, vel_y)

            pin["vx"] += nx * force
            pin["vy"] += ny * force

            vel_x *= 0.97
            vel_y *= 0.97

            # pin motion
    for pin in pins:
        if not pin["alive"]:
            continue

        pin["x"] += pin["vx"]
        pin["y"] += pin["vy"]

        pin["vx"] *= 0.5
        pin["vy"] *= 0.5

        if pin["collission"] and abs(pin["vx"]) + abs(pin["vy"]) < 0.1:
            pin["alive"] = False
            score += 1
            # send W to pic18f4520 through UART
            if not waved and score >= 6:
                ser.write(b"W")
                waved = 1

    # pin ↔ pin collision
    for i in range(len(pins)):
        for j in range(i + 1, len(pins)):
            p1, p2 = pins[i], pins[j]
            if not p1["alive"] or not p2["alive"]:
                continue

            dx = p2["x"] - p1["x"]
            dy = p2["y"] - p1["y"]
            dist = math.hypot(dx, dy)

            if dist < 2 * PIN_RADIUS and dist != 0:
                p2["collission"] = True
                nx = dx / dist
                ny = dy / dist
                impulse = 0.05 * (
                    math.hypot(p1["vx"], p1["vy"]) + math.hypot(p2["vx"], p2["vy"])
                )

                p1["vx"] -= nx * impulse
                p1["vy"] -= ny * impulse
                p2["vx"] += nx * impulse
                p2["vy"] += ny * impulse

    # draw on screen
    screen.fill((40, 120, 40))
    pygame.draw.rect(screen, (190, 150, 100), (LANE_X, 0, LANE_WIDTH, HEIGHT))

    pygame.draw.circle(screen, (30, 30, 30), (int(ball_x), int(ball_y)), BALL_RADIUS)

    for pin in pins:
        if pin["alive"]:
            pygame.draw.circle(
                screen, (240, 240, 240), (int(pin["x"]), int(pin["y"])), PIN_RADIUS
            )

    # show ball trail
    if len(touch_points) > 1:
        for i in range(len(touch_points) - 1):
            p1 = map_touch_to_lane(*touch_points[i])
            p2 = map_touch_to_lane(*touch_points[i + 1])
            pygame.draw.line(screen, (255, 0, 0), p1, p2, 2)

    screen.blit(font.render(f"Score: {score}", True, (255, 255, 255)), (20, 20))

    pygame.draw.rect(screen, (180, 50, 50), retry_rect)
    screen.blit(
        button_font.render("Retry", True, (255, 255, 255)),
        (retry_rect.x + 25, retry_rect.y + 10),
    )

    pygame.display.flip()
