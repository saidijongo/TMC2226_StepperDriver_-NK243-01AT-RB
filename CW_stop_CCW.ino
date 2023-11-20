#define EN_PIN    5 // enable
#define DIR_PIN   3 // direction
#define STEP_PIN  4 // step

void setup() {
  // set pin modes
  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH); // deactivate driver (LOW active)
}

void loop() {
  // Rotate CW for 3 seconds
  digitalWrite(DIR_PIN, LOW); // Set direction for clockwise rotation
  digitalWrite(EN_PIN, LOW);  // Activate driver

  for (int i = 0; i < 1500; i++) { // 1500 steps at 2 ms per step is 3 seconds
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000); 
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000); 
  }

  // Stop for 3 seconds
  digitalWrite(EN_PIN, HIGH); // Deactivate driver
  delay(3000);

  // Rotate CCW for 3 seconds
  digitalWrite(DIR_PIN, HIGH); // Set direction for counterclockwise rotation
  digitalWrite(EN_PIN, LOW);   // Activate driver

  for (int i = 0; i < 1500; i++) { // 1500 steps at 2 ms per step is 3 seconds
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000); 
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000); 
  }

  // Stop for 3 seconds
  digitalWrite(EN_PIN, HIGH); // Deactivate driver
  delay(3000);
}
