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
float avgMidpoint, avgDistance, oldMidpoint, newMidpoint;
//float timeTurnStarted, timeTurnStopped = 0;
float feedforwardTimeOut = 0;
float estimate = 0;
float previousEstimate = 0;

// --- Speed PID Variables ---
float Kp = 8, Ki = 1, Kd = 0.2;
float integral = 0, previous_error = 0;
unsigned long lastTime = 0;

// --- Steering PID Variables (The "A" prefix) ---
float AKp = 0.2, AKi = 0, AKd = 0.02; // Adjusted AKp for visible steering 0.2, 0.02
float Aintegral = 0, Aprevious_error = 0, Aerror = 0, Aderivative = 0;
unsigned long AlastTime = 0;
float avgMidpointAngle = 90; // Center is usually 90

//gain scheduling
float AKp_slow = 12;
float AKp_fast = 8;

// --- Moving Average Filters ---
float speedHistory[5] = {0};
int speedIndex = 0;
float MidpointHistory[5] = {0};
int MidpointIndex = 0;
float DistanceHistory[10] = {0};
int DistanceIndex = 0;

// --- Serial Communication ---
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

float alpha = 0.5f;

float steeringBase = 0.0;
float steeringFeedforward = 0.0;
float steeringFinal = 0.0;

unsigned long timeTurnStarted = 0;
unsigned long timeTurnStopped = 0;



// --- Helper: Moving Average for Forward Distance ---
float addDistanceAndGetAverage(float newDistance) {
    DistanceHistory[DistanceIndex] = newDistance;
    DistanceIndex = (DistanceIndex + 1) % 5;
    float sum = 0;
    for (int i = 0; i < 5; i++) sum += DistanceHistory[i];
    return sum / 5.0;
}

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
    Serial.begin(115200);
    
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
    delay(25); 
    distanceR = readDistance(trigR, echoR);
    delay(25);
    distanceM = readDistance(trigM, echoM);

    // 2. STEERING PID
    distancemidpoint = (distanceR - distanceL) / 2.0;

    //float avgMidpoint = addMidpointAndGetAverage(distancemidpoint);
    avgMidpoint = distancemidpoint;

    avgDistance = addDistanceAndGetAverage(distanceM);



    alphaFilter();

    calcPIDAngle(0, avgMidpoint); // Target is 0 (dead center)

    //avgMidpointAngle = 95;

    //isTurn();


    steerServo.write(avgMidpointAngle);

      // Print the current count to the Serial Monitor
  if(newPulse){
    String response = "Speed: "+ String(speed)+"\n";
    //Serial.print("Total activations: ");
    //Serial.println(pulseCount);
    //Serial.print("Speed: ");
    //Serial.print(speed);
    //Serial.println(" m/s");
    //Serial.print(throttle);
    //Serial.println(" throttle");
    Serial.println(response);
    newPulse=false;
  }

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

    // Check if data is available from the Rock 4
  if (Serial.available() > 0) {
    // Read the incoming message until a newline character
    String incoming = Serial.readStringUntil('\n');
    incoming.trim(); // Remove any stray spaces or carriage returns

    String response = "Acknowledgment: [" + incoming + "]";

    // --- SPEED COMMANDS ---
    if (incoming.startsWith("SKP")) {
        float kp_val = incoming.substring(3).toFloat();
        Kp = kp_val; // Assign to your speed PID P variable
        response += " - Speed KP updated";
    } 
    else if (incoming.startsWith("SKI")) {
        float ki_val = incoming.substring(3).toFloat();
        Ki = ki_val; // Assign to your speed PID I variable
        response += " - Speed KI updated";
    } 
    else if (incoming.startsWith("SKD")) {
        float kd_val = incoming.substring(3).toFloat();
        Kd = kd_val; // Assign to your speed PID D variable
        response += " - Speed KD updated";
    } 
    else if (incoming.startsWith("S")) {
        setSpeed = incoming.substring(1).toFloat();
        response += " - Target speed updated";
        if (setSpeed == 0) {
            throttle = 0;
            motorServo.writeMicroseconds(1500);
        }
    } 
    else if (incoming.startsWith("F")) {
        alpha = incoming.substring(1).toFloat();
        response += " - Alpha updated";
    } 
    
    // --- STEER COMMANDS ---
    // Note: Since we removed gain scheduling, all AKP commands update one variable
    else if (incoming.startsWith("AKP")) {
        //float steer_kp_val;
        if (incoming.startsWith("AKPslow")) {
            AKp_slow = incoming.substring(7).toFloat();
        } else if (incoming.startsWith("AKPfast")) {
            AKp_fast = incoming.substring(7).toFloat();
        } else {
            AKp = incoming.substring(3).toFloat();
        }
        //steer_kp = steer_kp_val; // Unified variable for steering KP
        response += " - Steer KP updated (No scheduling)";
    } 
    else if (incoming.startsWith("AKI")) {
        float ki_val = incoming.substring(3).toFloat();
        AKi = ki_val; // Assign to your speed PID I variable
        response += " - Steer KI updated";
    } 
    else if (incoming.startsWith("AKD")) {
        float kd_val = incoming.substring(3).toFloat();
        AKd = kd_val; // Assign to your speed PID D variable
        response += " - Steer KD updated";
    }

    // Send the response back to the Rock 4
    Serial.println(response);
    Serial.println("current PID parameters:");
    Serial.print("SKP: ");
    Serial.println(Kp);
    Serial.print("SKI: ");
    Serial.println(Ki);
    Serial.print("SKD: ");
    Serial.println(Kd);
    Serial.print("AKP: ");
    Serial.println(AKp);
    Serial.print("AKI: ");
    Serial.println(AKi);
    Serial.print("AKD: ");
    Serial.println(AKd);
  }

    // 4. COMMANDS & TELEMETRY
    recvWithEndMarker();
    if (newData) processCommands();

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 200) {
        Serial.print("L:"); Serial.print(distanceL);
        Serial.print(" R:"); Serial.print(distanceR);
        Serial.print(" Spd:"); Serial.print(speed);
        Serial.print(" Ang:"); Serial.print(avgMidpointAngle);
        Serial.print(" Mid:"); Serial.println(avgDistance);
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
    integral = constrain(integral, -20, 20); 
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
    avgMidpointAngle = map(output, -50, 50, 65, 125);
    avgMidpointAngle = constrain(avgMidpointAngle, 65, 125);

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


void alphaFilter()
{

    // Estimate velocity from previous estimates
    float Vy = estimate - previousEstimate;

    // Save old estimate
    previousEstimate = estimate;

    // Predict next position
    float Ypred = estimate + Vy;

    // Correct using measurement
    estimate = Ypred + alpha * (avgMidpoint - Ypred);

    avgMidpoint = estimate;
}

void isTurn() {

    const float rampForTurn = 1000.0;     // ms
    const float feedforwardAngle = 20.0;  // degrees

    bool inTurn = (avgDistance < 20);

    // reset feedforward every loop
    steeringFeedforward = 0.0;

    // =========================
    // ENTER TURN
    // =========================
    if (inTurn) {

        // start timer once
        if (timeTurnStarted == 0) {
            timeTurnStarted = millis();
        }

        // cancel exit timer
        timeTurnStopped = 0;

        // ramp 0 -> 1
        float rampIn = constrain(
            (millis() - timeTurnStarted) / rampForTurn,
            0.0,
            1.0
        );

        steeringFeedforward = feedforwardAngle * rampIn;
    }

    // =========================
    // EXIT TURN
    // =========================
    else {

        // start exit timer once
        if (timeTurnStarted != 0 && timeTurnStopped == 0) {
            timeTurnStopped = millis();
        }

        // ramp out
        if (timeTurnStopped != 0) {

            float rampOut = 1.0 - constrain(
                (millis() - timeTurnStopped) / rampForTurn,
                0.0,
                1.0
            );

            steeringFeedforward = feedforwardAngle * rampOut;

            // fully exited turn
            if (rampOut <= 0.0) {
                timeTurnStarted = 0;
                timeTurnStopped = 0;
            }
        }
    }

    // =========================
    // BASE STEERING
    // =========================

    // Example:
    // midpoint controller already computed elsewhere
    steeringBase = avgMidpointAngle;

    // Apply turn direction
    if (avgMidpoint > 0) {
        steeringFinal = steeringBase + steeringFeedforward;
    }
    else {
        steeringFinal = steeringBase - steeringFeedforward;
    }

    // Send to servo
    avgMidpointAngle = steeringFinal;

    Serial.print("Base: ");
    Serial.print(steeringBase);

    Serial.print(" FF: ");
    Serial.print(steeringFeedforward);

    Serial.print(" Final: ");
    Serial.println(steeringFinal);
}