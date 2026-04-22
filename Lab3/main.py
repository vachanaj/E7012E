import serial
import time

# Initialize serial connection
# Ensure the port matches your 'ls /dev/ttyACM*' output
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
ser.flush()

try:
    while True:
        # Send data to Arduino
        ser.write(b"Ping from Rock 4\n")
        
        # Read response back
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            print(f"Received: {line}")
            
        time.sleep(1)
except KeyboardInterrupt:
    ser.close()