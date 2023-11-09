// TMC2225 Stepper Driver
//White = 2B = +B, Black = 2A=-B,green = 1A = A+, yellow = 1B = -A

#define EN_PIN    5 //enable (CFG6)
#define DIR_PIN   3 //direction
#define STEP_PIN  4 //step

unsigned long currentTime;
unsigned long previousTime = 0;
unsigned long clockwiseDuration = 10000; // 10 seconds
unsigned long counterClockwiseDuration = 10000; // 10 seconds
bool isClockwise = true;

void setup()
{
  // Set pin modes
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH); // Deactivate driver (LOW active)
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(DIR_PIN, LOW); // LOW or HIGH
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);

  digitalWrite(EN_PIN, LOW); // Activate driver
}

void loop()
{
  currentTime = millis();

  if (isClockwise) {
    // Rotate clockwise
    digitalWrite(DIR_PIN, LOW);
  } else {
    // Rotate counter-clockwise
    digitalWrite(DIR_PIN, HIGH);
  }

  // Make steps
  digitalWrite(STEP_PIN, HIGH);
  delay(2);
  digitalWrite(STEP_PIN, LOW);
  delay(2);

  // Check if it's time to change direction
  if (isClockwise && currentTime - previousTime >= clockwiseDuration) {
    isClockwise = false;
    previousTime = currentTime;
  } else if (!isClockwise && currentTime - previousTime >= counterClockwiseDuration) {
    isClockwise = true;
    previousTime = currentTime;
  }
}
