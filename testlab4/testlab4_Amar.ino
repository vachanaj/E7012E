#include <Servo.h>

// --- Speed Measurement (Hall Effect / Encoder) ---
volatile int pulseCount = 0;
volatile unsigned long timeSincePulse = 0;
volatile float speed = 0;
const float circumference = 0.118 * PI; // Wheel diameter 0.118m
const int magCount = 4;                 // Magnets per revolution
boolean newPulse = false;

// --- Actuators ---
Servo steerServo; 
Servo motorServo;
float throttle = 0;
volatile float setSpeed = 0;

// --- Ultrasonic Sensors ---
const int trigL = 11, echoL = 3;
const int trigR = 10, echoR = 4;
const int trigM = 9,  echoM = 5;

float distanceL, distanceR, distanceM;
float distancemidpoint;

// --- Speed PID Variables ---
float Kp = 1, Ki = 0.5, Kd = 0.1;
float integral = 0, previous_error = 0;
unsigned long lastTime = 0;

// --- Steering PID Variables (The "A" prefix) ---
float AKp = 4, AKi = 2, AKd = 1; // Adjusted AKp for visible steering
float Aintegral = 0, Aprevious_error = 0, Aerror = 0, Aderivative = 0;
unsigned long AlastTime = 0;
float avgMidpointAngle = 90; // Center is usually 90

// --- Moving Average Filters ---
float speedHistory[5] = {0};
int speedIndex = 0;
float MidpointHistory[5] = {0};
int MidpointIndex = 0;

// --- Serial Communication ---
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

// --- Helper: Moving Average for Speed ---
float addSpeedAndGetAverage(float newSpeed) {
    speedHistory[speedIndex] = newSpeed;
    speedIndex = (speedIndex + 1) % 5;
    float sum = 0;
    for (int i = 0; i < 5; i++) sum += speedHistory[i];
    return sum / 5.0;
}

// --- Helper: Moving Average for Midpoint ---
float addMidpointAndGetAverage(float newMidpoint) {
    MidpointHistory[MidpointIndex] = newMidpoint;
    MidpointIndex = (MidpointIndex + 1) % 5;
    float sum = 0;
    for (int i = 0; i < 5; i++) sum += MidpointHistory[i];
    return sum / 5.0;
}

void setup() {
    Serial.begin(9600);
    
    motorServo.attach(13);
    steerServo.attach(12);
    
    pinMode(2, INPUT);
    attachInterrupt(digitalPinToInterrupt(2), countPulse, FALLING);
    
    motorServo.writeMicroseconds(1500); // Neutral signal for ESC arming

    pinMode(trigL, OUTPUT); pinMode(echoL, INPUT);
    pinMode(trigR, OUTPUT); pinMode(echoR, INPUT);
    pinMode(trigM, OUTPUT); pinMode(echoM, INPUT);

    delay(3000); // Wait for ESC to arm
    lastTime = millis();
    AlastTime = millis();
}

void loop() {
    // 1. SENSOR DATA
    distanceL = readDistance(trigL, echoL);
    delayMicroseconds(10); 
    distanceR = readDistance(trigR, echoR);
    delayMicroseconds(10);
    distanceM = readDistance(trigM, echoM);

    // 2. STEERING PID
    distancemidpoint = (distanceR - distanceL) / 2.0;
    float avgMidpoint = addMidpointAndGetAverage(distancemidpoint);
    calcPIDAngle(0, avgMidpoint); // Target is 0 (dead center)
    steerServo.write(avgMidpointAngle);

    // 3. SPEED PID
    // Safety: check if car has stopped moving
    if (millis() - timeSincePulse > 500) speed = 0;

    float avgSpeed = addSpeedAndGetAverage(speed);
    if (setSpeed > 0) {
        calcPID(setSpeed, avgSpeed);
    } else {
        throttle = 0;
        integral = 0; 
    }

    // Apply Throttle
    int pwm = map(throttle, 0, 255, 1500, 2000);
    motorServo.writeMicroseconds(pwm);

    // 4. COMMANDS & TELEMETRY
    recvWithEndMarker();
    if (newData) processCommands();

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 200) {
        Serial.print("L:"); Serial.print(distanceL);
        Serial.print(" R:"); Serial.print(distanceR);
        Serial.print(" Spd:"); Serial.print(speed);
        Serial.print(" Ang:"); Serial.println(avgMidpointAngle);
        lastPrint = millis();
    }
}

// --- Distance Calculation ---
float readDistance(int trig, int echo) {
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    // 25ms timeout = ~4 meters max
    unsigned long duration = pulseIn(echo, HIGH, 25000); 
    if (duration == 0) return 400.0;
    return duration * 0.034 / 2.0;
}

// --- Speed PID ---
void calcPID(float target, float current) {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0;
    if (dt <= 0) return;

    float error = target - current;
    integral += error * dt;
    integral = constrain(integral, -100, 100); 
    float deriv = (error - previous_error) / dt;

    float output = (Kp * error) + (Ki * integral) + (Kd * deriv);
    throttle = constrain(output, 0, 255);

    previous_error = error;
    lastTime = now;
}

// --- Steering PID ---
void calcPIDAngle(float target, float current) {
    unsigned long now = millis();
    float dt = (now - AlastTime) / 1000.0;
    if (dt <= 0) return;

    Aerror = target - current;
    Aintegral += Aerror * dt;
    Aintegral = constrain(Aintegral, -50, 50);
    Aderivative = (Aerror - Aprevious_error) / dt;

    float output = (AKp * Aerror) + (AKi * Aintegral) + (AKd * Aderivative);

    // Map the PID output to servo degrees (65 to 115)
    // Adjust -50/50 range based on how wide your hallway is
    avgMidpointAngle = map(output, -50, 50, 65, 115);
    avgMidpointAngle = constrain(avgMidpointAngle, 65, 115);

    Aprevious_error = Aerror;
    AlastTime = now;
}

// --- Serial Processing ---
void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n', rc;
    while (Serial.available() > 0 && !newData) {
        rc = Serial.read();
        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) ndx = numChars - 1;
        } else {
            receivedChars[ndx] = '\0';
            ndx = 0;
            newData = true;
        }
    }
}

void processCommands() {
    if (receivedChars[0] == 'S') {
        setSpeed = String(receivedChars).substring(1).toFloat();
        Serial.print("SetSpeed to: "); Serial.println(setSpeed);
    }
    newData = false;
}

// --- Interrupts ---
void countPulse() {
    pulseCount++;
    unsigned long now = millis();
    float pulseTime = (now - timeSincePulse) / 1000.0;
    if (pulseTime > 0) {
        speed = circumference / (pulseTime * magCount);
    }
    timeSincePulse = now;
}
