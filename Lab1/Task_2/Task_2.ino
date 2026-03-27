void setup() {
  pinMode(13, OUTPUT); // Set digital pin 13 as output
}

void loop() {
  digitalWrite(13, HIGH); // Turn LED on
  delay(1000);            // Wait for 1 second
  digitalWrite(13, LOW);  // Turn LED off
  delay(1000);            // Wait for 1 second
  digitalWrite(13, HIGH); // Turn LED on
  delay(100);            // Wait for 0.1 second
  digitalWrite(13, LOW);  // Turn LED off
  delay(100);            // Wait for 0.1 second
  digitalWrite(13, HIGH); // Turn LED on
  delay(10);            // Wait for 0.01 second
  digitalWrite(13, LOW);  // Turn LED off
  delay(10);            // Wait for 0.01 second
}
