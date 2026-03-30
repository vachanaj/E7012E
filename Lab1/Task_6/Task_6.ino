// Make sure to jump pin 2 to 13
volatile int blinkCount = 0;
volatile unsigned long timeSinceBlink = 0;
volatile byte brightness = 255;

const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data

boolean newData = false;
volatile int blinkLength = 500;
volatile float frequency=1;
void setup() {
  Serial.begin(9600);
  
  pinMode(13, OUTPUT); // The LED pin
  pinMode(2, INPUT);   // The Interrupt pin (connected to 13)

  // Attach interrupt to Pin 2
  // RISING means trigger when the voltage goes from LOW to HIGH
  attachInterrupt(digitalPinToInterrupt(2), countBlink, FALLING);
}

void loop() {
  // Blink
  analogWrite(13, brightness);
  delay(blinkLength);
  analogWrite(13, 0);
  delay(blinkLength);

  // Print the current count to the Serial Monitor
  Serial.print("Total Blinks: ");
  Serial.println(blinkCount);
  Serial.print("Frequency: ");
  Serial.println(frequency);
  recvWithEndMarker();
  if (newData == true) {
    if (receivedChars[0] == 'B') {
      brightness = 2.55 * String(receivedChars).substring(1).toInt();
      Serial.println(brightness);
      } else if (receivedChars[0] == 'F'){
      //Serial.println(String(receivedChars).substring(1));
      blinkLength= 500/String(receivedChars).substring(1).toFloat();
      }
    newData=false;
  }
}

// This function runs every time Pin 2 sees a RISING edge
void countBlink() {
  blinkCount++;
  float blinkTime = millis()-timeSinceBlink;
  blinkTime = blinkTime/1000;
  frequency = 1/blinkTime;
  timeSinceBlink= millis();
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
