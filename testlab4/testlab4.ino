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

//new
int trigL = 11; //Left
int echoL = 3;
int durationL;
float distanceL;
float meterL;

int trigR = 10; //Right
int echoR = 4;
int durationR;
float distanceR;
float meterR;

int trigM = 9; //Mid
int echoM = 5;
int durationM;
float distanceM;
float meterM;

float distancemidpoint;
float MidpointHistory[5] = {0, 0, 0, 0, 0};
int MidpointIndex = 0;

volatile float avgMidpointAngle = 100;

float Aproportional, Aintegral, Aderivative, Asetpoint, Aerror = 0; // PID variables

// PID parameters (example values right now)
float AKp = 1;
float AKi = 3;
float AKd = 1;

unsigned long AlastTime = 0; // used in calcPID
float Aprevious_error = 0; // used in calcPID

float addMidpointAndGetAverage(float newMidpoint) {
    MidpointHistory[MidpointIndex] = newMidpoint;
    MidpointIndex = (MidpointIndex + 1) % 5;

    float sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += MidpointHistory[i];
    }

    return sum / 5.0;
}

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

    Serial.begin(9600);
  steerServo.attach(12); // Steer angle 
  motorServo.writeMicroseconds(1500); // neutral

  pinMode(trigL, OUTPUT);
  digitalWrite(trigL, LOW);
  delayMicroseconds(2);
  pinMode(echoL, INPUT);

  pinMode(trigR, OUTPUT);
  digitalWrite(trigR, LOW);
  delayMicroseconds(2);
  pinMode(echoR, INPUT);

  pinMode(trigM, OUTPUT);
  digitalWrite(trigM, LOW);
  delayMicroseconds(2);
  pinMode(echoM, INPUT);

  delay(3000); // allow ESC to arm
}

void loop() {

  //for Left
  digitalWrite(trigL, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigL, LOW);

  durationL = pulseIn(echoL, HIGH);

  if (durationL >= 38000) {
    Serial.println("Out of range");
    durationL = 38000;
    distanceL = durationL * 0.034 / 2;
  } 
  else {
    distanceL = durationL * 0.034 / 2;
  }

  //for Right
  digitalWrite(trigR, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigR, LOW);

  durationR = pulseIn(echoR, HIGH);

  if (durationR >= 38000) {
    Serial.println("Out of range Right");
    durationR = 38000;
    distanceR = durationR * 0.034 / 2;
  } 
  else {
    distanceR = durationR * 0.034 / 2;
  }

  //Calculating midpoint
  distancemidpoint = (distanceR-distanceL)/2;

  float avgMidpoint = addMidpointAndGetAverage(distancemidpoint);

  float setMidpoint = 0;

  calcPIDAngle(setMidpoint, avgMidpoint);

  Serial.println(avgMidpointAngle);

  avgMidpointAngle = (avgMidpointAngle - 27.5)/2; //scaling cuz servo stupido

 
  steerServo.write(avgMidpointAngle);

  // Print the current count to the Serial Monitor
  if(newPulse){
    Serial.print("Total activations: ");
    Serial.println(pulseCount);
    Serial.print("Speed: ");
    Serial.print(speed);
    Serial.println(" m/s");
    Serial.print(throttle);
    Serial.println(" throttle");
    newPulse=false;
    float avgSpeed = addSpeedAndGetAverage(speed);
    if(setSpeed !=0 ){//&& millis()- lastTime> 0.05){
      calcPID(setSpeed, avgSpeed);
    }
  }
  
  // Set speed
  //analogWrite(13, throttle);
  int pwm = map(throttle, 0, 255, 1500, 2000);
  motorServo.writeMicroseconds(pwm);
  //steerServo.write(steerAngle);
  recvWithEndMarker();
  if (newData == true) {
    if (receivedChars[0] == 'S') {
       setSpeed = String(receivedChars).substring(1).toFloat();
       Serial.print(setSpeed);
       Serial.println(" set speed");
       if(setSpeed != 0) {
        throttle=55;
       } else {
        throttle=0;
       }
       lastTime = millis();
       timeSincePulse = millis();
      } else if (receivedChars[0] == 'A'){
       steerAngle = String(receivedChars).substring(1).toFloat();
       //analogWrite(12, steerAngle);
       Serial.print(steerAngle);
       Serial.println(" angle");
      }
    newData=false;
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
    delay(100);
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

// use this function in the loop of arduino program
void calcPIDAngle(float setmidpoint, float midpoint) {
  //Serial.print(midpoint);
  unsigned long currentTime = millis();
  float dt = (currentTime - AlastTime) / 1000.0; // seconds

  if (dt <= 0) return;

  // replace with sensor reading
  float measured_midpoint = midpoint;

  float error = setmidpoint - measured_midpoint;

  integral += error * dt;

  // prevent integral windup
  integral = constrain(integral, -100, 100);
 // Serial.print(integral);
 // Serial.println(" integral");
  derivative = (error - Aprevious_error) / dt;

  //float output = Kp * error + Ki * integral + Kd * derivative;
  float output = Kp * error;


  avgMidpointAngle = (output + 646)*255/1292 +127.5; //scales from max input to 0-255

   //Serial.println(avgMidpointAngle);

  Serial.println(" wanted 0-255 angle:");
  // Serial.print(avgMidpointAngle);
  avgMidpointAngle = constrain(avgMidpointAngle, 0, 255);
  

  // apply output for next timestep
 // throttle = output;

  Aprevious_error = error;
  AlastTime = currentTime;

}