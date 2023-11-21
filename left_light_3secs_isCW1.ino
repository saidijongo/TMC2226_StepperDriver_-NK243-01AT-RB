#define EN_PIN    5 // enable
#define DIR_PIN   3 // direction
#define STEP_PIN  4 // step

const int steps = 1500;
const int _step_delay = 1000;
bool isClockwise = true;

void stepMotor(int pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(_step_delay);
  digitalWrite(pin, LOW);
  delayMicroseconds(_step_delay);
}

void setup() {
  // set pin modes
  pinMode(EN_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH); // deactivate driver (LOW active)
}

void loop() {
  // Rotate for 3 seconds
  digitalWrite(DIR_PIN, isClockwise ? LOW : HIGH); // Set direction
  digitalWrite(EN_PIN, LOW);  // Activate driver

  for (int i = 0; i < steps; i++) {
    stepMotor(STEP_PIN);
  }

  // Stop for 3 seconds
  digitalWrite(EN_PIN, HIGH); // Deactivate driver
  digitalWrite(STEP_PIN, HIGH);

  delay(3000);

  // Change direction
  isClockwise = !isClockwise;

  // 3 seconds in the opposite dir
  digitalWrite(DIR_PIN, isClockwise ? LOW : HIGH); // Set direction
  digitalWrite(EN_PIN, LOW);   // Activate driver

  for (int i = 0; i < steps; i++) {
    stepMotor(STEP_PIN);
  }

  // Stop for 3 seconds
  digitalWrite(EN_PIN, HIGH); // Deactivate driver
  digitalWrite(STEP_PIN, LOW);
  delay(3000);
}
