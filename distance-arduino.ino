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

void setup()
{
  Serial.begin(9600);

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
  } 
  else {
    distanceL = durationL * 0.034 / 2;
    meterL = distanceL / 100;

    Serial.print("Distance Left: ");
    Serial.print(distanceL);
    Serial.print(" cm\t");
    Serial.print(meterL);
    Serial.println(" m");
  }

  //for Right
  digitalWrite(trigR, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigR, LOW);

  durationR = pulseIn(echoR, HIGH);

  if (durationR >= 38000) {
    Serial.println("Out of range");
  } 
  else {
    distanceR = durationR * 0.034 / 2;
    meterR = distanceR / 100;

    Serial.print("Distance Right: ");
    Serial.print(distanceR);
    Serial.print(" cm\t");
    Serial.print(meterR);
    Serial.println(" m");
  }

  //for Mid
  digitalWrite(trigM, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigM, LOW);

  durationM = pulseIn(echoM, HIGH);

  if (durationM >= 38000) {
    Serial.println("Out of range");
  } 
  else {
    distanceM = durationM * 0.034 / 2;
    meterM = distanceM / 100;

    Serial.print("Distance Mid: ");
    Serial.print(distanceM);
    Serial.print(" cm\t");
    Serial.print(meterM);
    Serial.println(" m");
  }

  delay(500);
}
 