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
    gray = cv2.medianBlur(gray, 5)

    circles = cv2.HoughCircles(
        gray,
        cv2.HOUGH_GRADIENT,
        dp=1,
        minDist=100,
        param1=100,
        param2=60,
        minRadius=50,
        maxRadius=200
    )

    if circles is not None:
        circles = np.uint16(np.around(circles))

        hsv = cv2.cvtColor(output, cv2.COLOR_BGR2HSV)

        for i in range(len(circles[0])):
            x, y, r = circles[0][i]

            # Create mask for the circle
            mask = np.zeros(gray.shape, dtype=np.uint8)
            cv2.circle(mask, (x, y), r, 255, -1)

            # Extract HSV pixels inside circle
            pixels = hsv[mask == 255]

            # Split channels
            h = pixels[:, 0]
            s = pixels[:, 1]
            v = pixels[:, 2]

            # Only consider sufficiently saturated pixels (ignore gray/white)
            valid = s > 50
            h = h[valid]

            if len(h) == 0:
                label = "Unknown"
                color = (255, 255, 255)
            else:
                # Red wraps around HSV (two ranges!)
                red_pixels = np.sum((h < 10) | (h > 170))
                green_pixels = np.sum((h > 35) & (h < 85))

                if red_pixels > 0.6 * len(h):
                    label = "Red"
                    color = (0, 0, 255)
                elif green_pixels > 0.6 * len(h):
                    label = "Green"
                    color = (0, 255, 0)
                else:
                    label = "Other"
                    color = (255, 0, 0)

            # Draw result
            cv2.circle(output, (x, y), r, color, 2)
            cv2.circle(output, (x, y), 2, (255, 255, 255), 3)

            cv2.putText(output, label, (x - 40, y - r - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)

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
