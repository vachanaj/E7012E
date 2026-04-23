#include <Servo.h>

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

Servo steerServo;  // create servo object to control a servo
Servo motorServo;

float proportional, integral, derivative, setpoint, error = 0; // PID variables

// PID parameters (example values right now)
float Kp = 1;
float Ki = 3;
float Kd = 1;

unsigned long lastTime = 0; // used in calcPID
float previous_error = 0; // used in calcPID

float addMidpointAndGetAverage(float newMidpoint) {
    MidpointHistory[MidpointIndex] = newMidpoint;
    MidpointIndex = (MidpointIndex + 1) % 5;

    float sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += MidpointHistory[i];
    }

    return sum / 5.0;
}

void setup()
{
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
  delay(6000);
  Serial.println("Distance:");
}

void loop()
{

  //for Left
  digitalWrite(trigL, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigL, LOW);

  durationL = pulseIn(echoL, HIGH);

  if (durationL >= 38000) {
    Serial.println("Out of range");
    durationL = 38000;
    distanceL = durationL * 0.034 / 2;
    meterL = distanceL / 100;
  } 
  else {
    distanceL = durationL * 0.034 / 2;
    meterL = distanceL / 100;

    //Serial.print("Distance Left: ");
    //Serial.print(distanceL);
    //Serial.print(" cm\t");
    //Serial.print(meterL);
    //Serial.println(" m");
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
    meterR = distanceR / 100;
  } 
  else {
    distanceR = durationR * 0.034 / 2;
    meterR = distanceR / 100;

    //Serial.print("Distance Right: ");
    //Serial.print(distanceR);
    //Serial.print(" cm\t");
    //Serial.print(meterR);
    //Serial.println(" m");
  }

  //Calculating midpoint
distancemidpoint = (distanceR-distanceL)/2;

float avgMidpoint = addMidpointAndGetAverage(distancemidpoint);

Serial.print("Distance Midpoint: ");
Serial.print(avgMidpoint);
Serial.println(" cm\t");

float setMidpoint = 0;

calcPIDAngle(setMidpoint, avgMidpoint);

  //for Mid
  digitalWrite(trigM, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigM, LOW);

  durationM = pulseIn(echoM, HIGH);

  if (durationM >= 38000) {
    Serial.println("Out of range Mid");
    durationM = 38000;
    distanceM = durationM * 0.034 / 2;
    meterM = distanceM / 100;
  } 
  else {
    distanceM = durationM * 0.034 / 2;
    meterM = distanceM / 100;

    //Serial.print("Distance Mid: ");
    //Serial.print(distanceM);
    //Serial.println(" cm\t");
  }

  delay(100);
}

// use this function in the loop of arduino program
void calcPIDAngle(float setmidpoint, float midpoint) {
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0; // seconds

  if (dt <= 0) return;

  // replace with sensor reading
  float measured_midpoint = midpoint;

  float error = setmidpoint - measured_midpoint;

  integral += error * dt;

  // prevent integral windup
  integral = constrain(integral, -100, 100);
 // Serial.print(integral);
 // Serial.println(" integral");
  derivative = (error - previous_error) / dt;

  //float output = Kp * error + Ki * integral + Kd * derivative;
  float output = Kp * error;


  Serial.print(output);
  Serial.println(" wanted distance");
  output = constrain(output, 55, 255);

  // apply output for next timestep
 // throttle = output;

  previous_error = error;
  lastTime = currentTime;
}
 