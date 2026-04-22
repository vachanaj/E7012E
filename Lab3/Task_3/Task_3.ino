void setup() {
  // Using the Programming Port (Serial)
  Serial.begin(115200); 
  while (!Serial); // Wait for the port to connect
}

void loop() {
  // Check if data is available from the Rock 4
  if (Serial.available() > 0) {
    // Read the incoming message until a newline character
    String incoming = Serial.readStringUntil('\n');
    
    // Create a response
    String response = "Acknowledgment: I received [" + incoming + "]";
    
    // Send the response back to the Rock 4
    Serial.println(response);
  }
}
