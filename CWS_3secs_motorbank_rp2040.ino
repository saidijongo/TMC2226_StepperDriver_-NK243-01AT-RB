#define EN_PIN    5 // enable
#define DIR_PIN   3 // direction
#define STEP_PIN  4 // step

void setup() {
  // set pin modes
  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH); // deactivate driver (LOW active)
  Serial.begin(9600); 
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    executeCommand(command);
  }
}

void executeCommand(char command) {
  switch (command) {
    case 'C':
      rotateClockwise();
      break;
      
    case 'W':
      rotateCounterClockwise();
      break;
      
    case 'S':
      stopMotor();
      break;
      
    default:
      Serial.println("Invalid command");
  }
}

void rotateClockwise() {
  digitalWrite(DIR_PIN, LOW); //  clockwise rotation
  digitalWrite(EN_PIN, LOW);  // Activate driver

  for (int i = 0; i < 1500; i++) { // 1500 steps at 2 ms per step is 3 seconds
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000); 
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000); 
  }

  // Stop motor
  stopMotor();
}

void rotateCounterClockwise() {
  digitalWrite(DIR_PIN, HIGH); // counterclockwise rotation
  digitalWrite(EN_PIN, LOW);   // Activate driver

  for (int i = 0; i < 1500; i++) { // 1500 steps at 2 ms per step is 3 seconds
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000); 
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000); 
  }

  // Stop motor
  stopMotor();
}

void stopMotor() {
  digitalWrite(EN_PIN, HIGH); // Deactivate driver
  digitalWrite(STEP_PIN, HIGH);
  delay(3000); // Stop for 3 seconds
}
