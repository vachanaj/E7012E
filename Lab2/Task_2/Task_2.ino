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
float motorVoltage = 0;
boolean newPulse = false;
void setup() {
  Serial.begin(9600);
  
  pinMode(13, OUTPUT); // The speed to ESC
  pinMode(12, OUTPUT); // Steer angle  
  pinMode(2, INPUT);   // The Interrupt pin (connected to 13)

  // Attach interrupt to Pin 2
  // RISING means trigger when the voltage goes from LOW to HIGH
  attachInterrupt(digitalPinToInterrupt(2), countPulse, FALLING);
}

void loop() {
  // Print the current count to the Serial Monitor
  if(newPulse){
  Serial.print("Total activations: ");
  Serial.println(pulseCount);
  Serial.print("Speed: ");
  Serial.print(speed);
  Serial.println(" m/s");
  newPulse=false;
  }
  // Set speed
  analogWrite(13, motorVoltage);

  recvWithEndMarker();
  if (newData == true) {
    if (receivedChars[0] == 'S') {
       float motorVoltage = String(receivedChars).substring(1).toFloat();
       Serial.print(motorVoltage);
       Serial.print(" motor voltage");
      } else if (receivedChars[0] == 'A'){
       float steerAngle = String(receivedChars).substring(1).toFloat();
       analogWrite(12, steerAngle);
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
}
