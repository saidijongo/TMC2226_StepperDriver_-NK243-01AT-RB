
#include <Arduino.h>

const int EN_PIN = 5;    // enable, active when LOW
const int DIR_PIN = 3;   // direction, (DIR_PIN, isClockwise ? HIGH : LOW)
const int STEP_PIN = 4;  // step, 1.8 degrees per step.
const int RELAY_PIN[] = {5, 6, 7, 8, 9, 10, 11};  // Update to your specific relay pins

const int TOP_SENSOR_PIN = 13;
const int BOTTOM_SENSOR_PIN = 12;

const float SENSOR_BACK_STEP = 100; // 3300 8 degrees 2850 14.4
const int STEP_DELAY = 1000;
const float STEP_ANGLE = 1.8; // 0.0072;
; //64; // Initial step delay value

float _step_start_position = 0;
float _step_end_position = 0;
float _step_current_position = 0;

float _step_zero_angle = 2250; // 8 degrees from the sensor 2250

int _step_delay = STEP_DELAY; // Sensor response delay

enum class MotorState { STOPPED, MOVING_CW, MOVING_CCW };
MotorState _motorState = MotorState::STOPPED;
bool _isHoming = false;

bool _stopCommandReceived = false;

void setup() {
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH);

  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  pinMode(TOP_SENSOR_PIN, INPUT_PULLUP);
  pinMode(BOTTOM_SENSOR_PIN, INPUT_PULLUP);

  for (int i = 0; i < sizeof(RELAY_PIN) / sizeof(RELAY_PIN[0]); i++) {
    pinMode(RELAY_PIN[i], OUTPUT);
    digitalWrite(RELAY_PIN[i], LOW); // LOW initially
  }

  Serial.begin(115200);
  Serial.flush();
  initializeMotor(); // Initialize the motor at startup
}

void motorStep(bool isClockwise, int steps) {
  digitalWrite(DIR_PIN, isClockwise ? HIGH : LOW);
  digitalWrite(EN_PIN, LOW);

  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(_step_delay);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(_step_delay);

    if (isClockwise)
      _step_current_position++; 
    else
      _step_current_position--; 
  }

  digitalWrite(EN_PIN, HIGH);
}

void stopMotor() {
  digitalWrite(DIR_PIN, LOW);
  digitalWrite(EN_PIN, HIGH);

  _motorState = MotorState::STOPPED;
}

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void moveMotor(int angle, int speedPercent) {
  if (_step_start_position == 0 && _step_end_position == 0) {
    Serial.print("ST,0,RETMOVE,ERR_NOINIT,0,0,ED\r\n"); // not initialized
    return;
  }

  // Calculate the target step position for the requested angle
  int step_target_position = static_cast<int>(angle / STEP_ANGLE) + _step_zero_angle;

  // Determine the rotation direction
  bool isClockwise = (_step_current_position < step_target_position);

  int speed = map(speedPercent, 0, 100, 800, 20);
  String dir = isClockwise ? "CW" : "CCW";
  Serial.print(String(dir) + " Angle " + String(angle) + " Speed " + String(speed) + "\r\n");

  _motorState = isClockwise ? MotorState::MOVING_CW : MotorState::MOVING_CCW;

  while (_step_current_position != step_target_position) {
    bool sleft = digitalRead(BOTTOM_SENSOR_PIN) == LOW;
    bool sright = digitalRead(TOP_SENSOR_PIN) == LOW;

    if (_stopCommandReceived) {
      stopMotor();
      Serial.print("ST,0,RETSTOP,OK," + String(_step_current_position * STEP_ANGLE) + "," + String(speed) + ",ED\r\n");
      _stopCommandReceived = false;
      return;
    }

    if (sleft || sright) {
      stopMotor();
      delay(1000);

      if (isClockwise) {
        motorStep(false, SENSOR_BACK_STEP); // Rotate 8 degrees CCW
      } else {
        motorStep(true, SENSOR_BACK_STEP); // Rotate 8 degrees CW
      }

      delay(1000);
      Serial.print("ST,0,RETMOVE,ERR_INTERRUPT," + String(step_target_position) + "," + String(speedPercent) + ",ED\r\n");
      return;
    }

    motorStep(isClockwise, 1);

    if (isClockwise == true && _step_current_position >= _step_end_position) {
      Serial.print("ST,0,RETMOVE,ERR_LESS," + String(step_target_position) + "," + String(speedPercent) + ",ED\r\n");
      return;
    }

    if (isClockwise == false && _step_current_position <= _step_start_position) {
      Serial.print("ST,0,RETMOVE,ERR_OVER," + String(step_target_position) + "," + String(speedPercent) + ",ED\r\n");
      return;
    }
  }

  Serial.print("ST,0,RETMOVE,OK," + String(step_target_position) + "," + String(speedPercent) + ",ED\r\n");
}

void initializeMotor() {
  int stepOverCheck = 500000;

  bool sleft = true;
  bool sright = true;

  int old_step_delay = _step_delay;

  do {
    motorStep(false, 1); 
    stepOverCheck--;

    if (stepOverCheck < 0) {
      Serial.print("ST,0,RETINIT,ERR-OV-CCW,ED\r\n");
      return;
    }

    sleft = digitalRead(BOTTOM_SENSOR_PIN) == LOW;
    sright = digitalRead(TOP_SENSOR_PIN) == LOW;

    if (_stopCommandReceived) {
      stopMotor();
      Serial.print("ST,0,RETSTOP,OK," + String(_step_current_position * STEP_ANGLE) + "," + String(_step_delay) + ",ED\r\n");
      _stopCommandReceived = false;
      return;
    }

    if (sright) {
      delay(_step_delay);
      stopMotor();
      break;
    } else if (sleft) { // LEFT
      Serial.print("ST,0,RETINIT,ERR_OP,ED\r\n"); // OPPOSITE SIDE INIT
      delay(_step_delay);
      stopMotor();

      do {
        motorStep(true, 1); // Rotate CW until right sensor is interrupted
        sleft = digitalRead(BOTTOM_SENSOR_PIN) == LOW;
        sright = digitalRead(TOP_SENSOR_PIN) == LOW;

        if (sright) {
          delay(1000);
          break;
        }
      } while (sright == false);
    }
  } while (sleft == false && sright == false);

  _step_start_position = 0;
  _step_current_position = 0;

  stepOverCheck = 500000;

  do {
    motorStep(false, 1); // Rotate CCW until right sensor is interrupted
    stepOverCheck--;

    if (stepOverCheck < 0) {
      Serial.print("ST,0,RETINIT,ERR-OV-CW,ED\r\n");
      _step_delay = old_step_delay;
      return;
    }

    sleft = digitalRead(BOTTOM_SENSOR_PIN) == LOW;
    sright = digitalRead(TOP_SENSOR_PIN) == LOW;
  } while (sright == false);

  motorStep(true, SENSOR_BACK_STEP); // Rotate 8 degrees CW
  delay(_step_delay);

  stepOverCheck = 500000;
  do {
    motorStep(true, 1); // Rotate CW until left sensor is interrupted
    stepOverCheck--;

    if (stepOverCheck < 0) {
      return;
    }

    sleft = digitalRead(BOTTOM_SENSOR_PIN) == LOW;
    sright = digitalRead(TOP_SENSOR_PIN) == LOW;

    if (sleft) {
      stopMotor();
      delay(_step_delay);
      motorStep(false, SENSOR_BACK_STEP); // Rotate 8 degrees CCW
      delay(_step_delay);
      stopMotor();
      break;
    }

    if (_stopCommandReceived) {
      stopMotor();
      Serial.print("ST,0,RETSTOP,OK," + String(_step_current_position) + "," + String(_step_delay) + ",ED\r\n");
      _stopCommandReceived = false;
      return;
    }
  } while (sleft == false);

  _step_end_position = _step_current_position;
  float angleTotal = ((_step_end_position - _step_start_position) * STEP_ANGLE * STEP_ANGLE) / 2;

  Serial.print("ST,0,RETINIT," + String(angleTotal) + ",ED\r\n");
  _step_delay = old_step_delay;
}

void status() {
  String status;
  int angle = static_cast<int>(_step_current_position * STEP_ANGLE);
  String direction;

  switch (_motorState) {
    case MotorState::STOPPED:
      status = "STOPPED";
      break;
    case MotorState::MOVING_CW:
      status = "MOVING";
      direction = "CW";
      break;
    case MotorState::MOVING_CCW:
      status = "MOVING";
      direction = "CCW";
      break;
  }

  if (_motorState == MotorState::STOPPED) {
    Serial.print("ST,0,RETSTATUS," + status + "," + String(angle) + ",ED\r\n");
  } else {
    int speed = map(_step_delay, 30, 800, 0, 100);
    Serial.print("ST,0,RETSTATUS," + status + "," + direction + "," + String(angle) + "," + String(speed) + ",ED\r\n");
  }
}

void ionizerLamp(bool state) {
  for (int i = 0; i < sizeof(RELAY_PIN) / sizeof(RELAY_PIN[0]); i++) {
    digitalWrite(RELAY_PIN[i], state ? HIGH : LOW);
  }
}

void processCommand(String commandStr) {
  commandStr.trim();

  String cmd = getValue(commandStr, ',', 2);

  if (cmd.equals("CONN")) {
    if (Serial.available() > 0) {
      Serial.print("ST,0,RETCONN,0,ED\r\n"); // Connected
    } else {
      Serial.print("ST,0,RETCONN,0,ED\r\n"); // Connected
    }
    return;
  }

  if (cmd.equals("INIT")) {
    initializeMotor();
    return;
  }

  if (cmd.equals("STATUS")) {
    status();
    return;
  }

  if (cmd.equals("MOVE")) {
    int angle = getValue(commandStr, ',', 4).toInt();
    int speedPercent = getValue(commandStr, ',', 5).toInt();
    int stepValue = int(angle * 27.5);
    moveMotor(stepValue, speedPercent);
    return;
  }

  if (cmd.equals("LAMP")) {
    int lampState = getValue(commandStr, ',', 3).toInt();
    ionizerLamp(lampState == 1);
    Serial.print("ST,0,RETLAMP,OK," + String(lampState) + ",ED\r\n");
    return;
  }

  if (cmd.equals("STOP")) {
    _stopCommandReceived = true;
    stopMotor();
    return;
  }
}

void loop() {
  if (Serial.available() > 0) {
    String data = Serial.readString();
    processCommand(data);
  }
}
