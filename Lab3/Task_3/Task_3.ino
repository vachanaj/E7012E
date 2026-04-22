#include <Servo.h>

Servo steerServo;  // create servo object to control a servo
Servo motorServo;

void setup() {
  // Using the Programming Port (Serial)
  Serial.begin(115200); 
  while (!Serial); // Wait for the port to connect

  //pinMode(13, OUTPUT); // The speed to ESC
  motorServo.attach(13);
  
  steerServo.attach(12); // Steer angle 

  motorServo.writeMicroseconds(1500); // neutral
  delay(3000); // allow ESC to arm
}

void loop() {
  // Check if data is available from the Rock 4
  if (Serial.available() > 0) {
    // Read the incoming message until a newline character
    String incoming = Serial.readStringUntil('\n');

    // Create a response
    String response = "Acknowledgment: I received [" + incoming + "]";
    if (incoming[0] == 'S') {
      float throttle = String(incoming).substring(1).toFloat();
      int pwm = map(throttle, 0, 255, 1500, 2000);
      response = response + ", Updated speed";
      motorServo.writeMicroseconds(pwm);
    } else if (incoming[0] == 'A'){
      float steerAngle = String(incoming).substring(1).toFloat();
      steerServo.write(steerAngle);
      response = response + ", Updated angle";
    }
    
    
    // Send the response back to the Rock 4
    Serial.println(response);
  }
}
