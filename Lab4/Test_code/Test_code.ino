volatile int blinkCount = 0;
volatile unsigned long timeSinceBlink = 0;
volatile byte brightness = 255;
volatile int blinkLength = 500;
volatile bool printFrequency = false;   // ✅ Flag instead of Serial in ISR
volatile float lastFrequency = 0;       // ✅ Store result for main loop to print

const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);  
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), countBlink, RISING);
}

void loop() {
  analogWrite(13, brightness);   // ✅ PWM-capable pin
  delay(blinkLength);
  analogWrite(13, 0);
  delay(blinkLength);

  Serial.print("Total Blinks: ");
  Serial.println(blinkCount);

  // ✅ Print frequency from main loop, not ISR
  if (printFrequency) {
    Serial.print("Frequency: ");
    Serial.println(lastFrequency);
    printFrequency = false;
  }

void recvWithEndMarker();
  if (newData == true) {
    if (receivedChars[0] == 'B') {
      brightness = 2.55 * String(receivedChars).substring(1).toInt();
    } else if (receivedChars[0] == 'F') {
      blinkLength = 500 / String(receivedChars).substring(1).toFloat();
    }
    newData = false;
  }
}

void countBlink() {
  blinkCount++;
  unsigned long now = millis();                      // ✅ Read once
  float blinkTime = (now - timeSinceBlink) / 1000.0;
  if (blinkTime > 0) {
    lastFrequency = 1.0 / blinkTime;                // ✅ Store for main loop
    printFrequency = true;                           // ✅ Set flag, don't print
  }
  timeSinceBlink = now;
}
