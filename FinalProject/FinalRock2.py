import cv2
import numpy as np
import serial
import time
import threading
import sys

# Initialize serial connection
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
ser.flush()

# Open the default camera
cam = cv2.VideoCapture(0, cv2.CAP_V4L2)
frame_width = int(cam.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cam.get(cv2.CAP_PROP_FRAME_HEIGHT))

# Focal length and distance functions
def FocalLength(measured_distance, real_width, width_in_rf_image):
    return (width_in_rf_image * measured_distance) / real_width

focal_length = FocalLength(70, 15.5, 110)

def Distance_finder(Focal_Length, real_face_width, face_width_in_frame):
    return (real_face_width * Focal_Length) / face_width_in_frame

def labelImage(output):
    gray = cv2.cvtColor(output, cv2.COLOR_BGR2GRAY)
    gray = cv2.medianBlur(gray, 5)
    circles = cv2.HoughCircles(gray, cv2.HOUGH_GRADIENT, dp=1, minDist=100, 
                               param1=100, param2=50, minRadius=20, maxRadius=200)

    if circles is not None:
        circles = np.uint16(np.around(circles))
        hsv = cv2.cvtColor(output, cv2.COLOR_BGR2HSV)
        for i in range(min(5, len(circles[0]))):
            x, y, r = circles[0][i]
            mask = np.zeros(gray.shape, dtype=np.uint8)
            cv2.circle(mask, (x, y), r, 255, -1)
            pixels = hsv[mask == 255]
            h = pixels[:, 0][pixels[:, 1] > 40]
            if len(h) == 0: continue
            
            mean_h = np.mean(h)
            if (mean_h < 30) or (mean_h > 170):
                label = "Red"
            elif 35 < mean_h < 100:
                label = "Green"
            else: continue

            distance = Distance_finder(focal_length, 15.5, r*2)
            return (output, label, distance)
    return (output, "none", "null")

# --- NEW: Terminal Input Thread ---
def manual_input_thread():
    while True:
        user_cmd = input("\nEnter Command (e.g., S0.5, SKP1.2, AKPslow5): ").strip()
        if user_cmd:
            # Format and send to serial
            ser_msg = f"{user_cmd}\n".encode('utf-8')
            ser.write(ser_msg)
            print(f">>> Sent to Serial: {user_cmd}")

# Start the background thread
input_thread = threading.Thread(target=manual_input_thread, daemon=True)
input_thread.start()

cur_speed = 0
started = False

try:
    while True:
        ret, frame = cam.read()
        if not ret: break

        (frame, color, dist) = labelImage(frame)

        # Autonomous logic
        if color != "none":
            if color == "Green" and not started:
                started = True
                print(f"\n[Auto] Green detected: {dist}cm - Starting")
                ser.write(b'S0.01\n')
            elif color == "Red" and started:
                started = False
                print(f"\n[Auto] Red detected: {dist}cm - Stopping")
                ser.write(b'S0\n')

        # Read serial feedback
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').rstrip()
            print(f"\n[Serial Feed]: {line}")

except KeyboardInterrupt:
    print("\nShutting down...")
    ser.write(b'S0\n')
    cam.release()
    ser.close()