// Make sure to jump pin 2 to 13
volatile int blinkCount = 0;
volatile unsigned long timeSinceBlink = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(13, OUTPUT); // The LED pin
  pinMode(2, INPUT);   // The Interrupt pin (connected to 13)

  // Attach interrupt to Pin 2
  // RISING means trigger when the voltage goes from LOW to HIGH
  attachInterrupt(digitalPinToInterrupt(2), countBlink, RISING);
}

void loop() {
  // Blink
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);

  // Print the current count to the Serial Monitor
  Serial.print("Total Blinks: ");
  Serial.println(blinkCount);
}

// This function runs every time Pin 2 sees a RISING edge
void countBlink() {
  blinkCount++;
  unsigned long blinkTime = millis()-timeSinceBlink;
  Serial.print("Blink time:");
  Serial.println(blinkTime); 
  timeSinceBlink= millis();
}
