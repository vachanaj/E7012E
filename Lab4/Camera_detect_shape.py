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

def labelImage(img):
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, (5, 5), 0)

    threshold = cv2.adaptiveThreshold(
        gray, 255,
        cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
        cv2.THRESH_BINARY_INV,
        15, 3
    )

    kernel = np.ones((3,3), np.uint8)
    clean = cv2.morphologyEx(threshold, cv2.MORPH_OPEN, kernel)

    contours, _ = cv2.findContours(clean, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    result = np.zeros_like(clean)
    for cnt in contours:
        if cv2.contourArea(cnt) > 300:
            cv2.drawContours(result, [cnt], -1, 255, -1)

    contours, _ = cv2.findContours(threshold, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    # Process each contour
    for i, contour in enumerate(contours):
        if i == 0 or cv2.contourArea(contour) < 10000:
            continue

        # Approximate contour shape
        approx = cv2.approxPolyDP(contour, 0.01 * cv2.arcLength(contour, True), True)

        # Draw contour
        cv2.drawContours(img, [contour], 0, (0, 0, 255), 5)

        # Find center
        M = cv2.moments(contour)
        x=0
        y=0
        if M['m00'] != 0:
            x = int(M['m10'] / M['m00'])
            y = int(M['m01'] / M['m00'])

        # Detect shape
        sides = len(approx)
        #if sides == 3:
        #    label = 'Triangle'
        #elif sides == 4:
        #    label = 'Quadrilateral'
        #elif sides == 5:
        #    label = 'Pentagon'
        #elif sides == 6:
        #    label = 'Hexagon'
        if sides > 4:
            label = 'Circle'
            cv2.putText(img, label, (x, y), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)


        # Label the shape
    return img

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
