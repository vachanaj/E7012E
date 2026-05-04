import serial
import time

# Initialize serial connection
# Ensure the port matches your 'ls /dev/ttyACM*' output
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
ser.flush()

try:
    while True:
        # Send data to Arduino
        input1 = input()
        encoded = bytes(str(input1)+"\n", "utf-8")
    
        ser.write(encoded)
        time.sleep(10)
        # Read response back
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            print(f"Received: {line}")
            
except KeyboardInterrupt:
    ser.close()