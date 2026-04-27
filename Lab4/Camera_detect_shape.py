import cv2
import numpy as np

# Open the default camera
cam = cv2.VideoCapture(0, cv2.CAP_V4L2)
#cam.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))
# Get the default frame width and height
frame_width = int(cam.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cam.get(cv2.CAP_PROP_FRAME_HEIGHT))

# Define the codec and create VideoWriter object
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
out = cv2.VideoWriter('output.mp4', fourcc, 20, (frame_width, frame_height))

def labelImage(output):
    gray = cv2.cvtColor(output, cv2.COLOR_BGR2GRAY)
    # Reduce noise
    gray = cv2.medianBlur(gray, 5)

    # Detect circles
    circles = cv2.HoughCircles(
        gray,
        cv2.HOUGH_GRADIENT,
        dp=1,
        minDist=100,      
        param1=100,         
        param2=50,
        minRadius=30,       
        maxRadius=200
    )

    # Draw only the first detected circle
    if circles is not None:
        circles = np.uint16(np.around(circles))

        for i in range(len(circles[0])):
            x, y, r = circles[0][i]
            cv2.circle(output, (x, y), r, (0, 255, 0), 2)  # Circle outline
            cv2.circle(output, (x, y), 2, (0, 0, 255), 3)  # Center point
        print(len(circles[0]))
    return output

while True:
    ret, frame = cam.read()

    # Write the frame to the output file
    out.write(frame)
    frame = labelImage(frame)
    # Display the captured frame
    cv2.imshow('Camera', frame)

    # Press 'q' to exit the loop
    if cv2.waitKey(1) == ord('q'):
        break




# Release the capture and writer objects
cam.release()
out.release()
cv2.destroyAllWindows()
