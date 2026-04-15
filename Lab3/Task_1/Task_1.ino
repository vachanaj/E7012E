float proportional, integral, derivative, setpoint, error = 0; // PID variables

// PID parameters (example values right now)
float Kp = 1.5;
float Ki = 0.5;
float Kd = 0.1;

unsigned long lastTime = 0; // used in calcPID
float previous_error = 0; // used in calcPID

float motorVoltage = 0;

// use this function in the loop of arduino program
void calcPID(float setpoint, float speed) {
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0; // seconds

  if (dt <= 0) return;

  // replace with sensor reading
  measured_speed = getSpeed();

  error = setpoint - measured_speed;

  integral += error * dt;

  // prevent integral windup
  integral = constrain(integral, -100, 100);

  derivative = (error - previous_error) / dt;

  float output = Kp * error + Ki * integral + Kd * derivative;

  output = constrain(output, 0, 255);

  // apply output for next timestep
  motorVoltage = output;

  previous_error = error;
  lastTime = currentTime;
}