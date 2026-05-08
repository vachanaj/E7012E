#include <Servo.h>


// Make sure to jump pin 2 to 13
volatile int pulseCount = 0;
volatile unsigned long timeSincePulse = 0;

const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data

boolean newData = false;
volatile float frequency=1;
volatile int magCount=4;
//volatile float wheelDiameter= 0.118;
volatile float speed=0;
const float circumference=0.118*PI;
float throttle = 0;
boolean newPulse = false;
volatile float steerAngle = 100;
volatile float setSpeed = 0;

Servo steerServo;  // create servo object to control a servo
Servo motorServo;


float proportional, integral, derivative, setpoint, error = 0; // PID variables

// PID parameters (example values right now)
float Kp = 14;
float Ki = 3;
float Kd = 1;

unsigned long lastTime = 0; // used in calcPID
float previous_error = 0; // used in calcPID

float speedHistory[5] = {0, 0, 0, 0, 0};
int speedIndex = 0;

float addSpeedAndGetAverage(float newSpeed) {
    speedHistory[speedIndex] = newSpeed;
    speedIndex = (speedIndex + 1) % 5;

    float sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += speedHistory[i];
    }

    return sum / 5.0;
}

void setup() {
  Serial.begin(9600);
  
  //pinMode(13, OUTPUT); // The speed to ESC
  motorServo.attach(13);
  
  steerServo.attach(12); // Steer angle 
  pinMode(2, INPUT);   // The Interrupt pin (connected to 13)

  // Attach interrupt to Pin 2
  // RISING means trigger when the voltage goes from LOW to HIGH
  attachInterrupt(digitalPinToInterrupt(2), countPulse, FALLING);
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
    response = response + "\n";
    
    // Send the response back to the Rock 4
    Serial.println(response);
  }
}

// This function runs every time Pin 2 sees a RISING edge
void countPulse() {
  pulseCount++;
  float pulseTime = millis()-timeSincePulse;
  pulseTime = pulseTime/1000;
  timeSincePulse = millis();
  calcSpeed(pulseTime);
  newPulse=true;
}

void calcSpeed(float pulseTime) {
  speed = circumference / (pulseTime*magCount);
}

void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
    
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
                Serial.println("hej");
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}


// use this function in the loop of arduino program
void calcPID(float setpoint, float speed) {
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0; // seconds

  if (dt <= 0) return;

  // replace with sensor reading
  float measured_speed = speed;

  float error = setpoint - measured_speed;

  integral += error * dt;

  // prevent integral windup
  integral = constrain(integral, -100, 100);
  Serial.print(integral);
  Serial.println(" integral");
  derivative = (error - previous_error) / dt;

  float output = Kp * error + Ki * integral + Kd * derivative;
  Serial.print(output);
  Serial.println(" wanted throttle");
  output = constrain(output, 55, 255);

  // apply output for next timestep
  throttle = output;

  previous_error = error;
  lastTime = currentTime;
}
