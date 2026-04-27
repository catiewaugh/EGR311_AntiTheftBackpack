#include <Wire.h>

const int MPU            = 0x68;
const int PIN_SWITCH     = 4;
const int PIN_LED_YELLOW = 6;
const int PIN_LED_RED    = 7;
const int PIN_LED_GREEN  = 8;
const int PIN_SPEAKER    = 9;
const float THRESHOLD    = 0.5; // raise if triggers too easily

// MPU variables
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float AccErrorX, AccErrorY;
float GyroErrorX, GyroErrorY, GyroErrorZ;
float accAngleX, accAngleY;
float gyroAngleX = 0, gyroAngleY = 0, yaw = 0;
float roll, pitch;
unsigned long currentTime, previousTime;
float elapsedTime;
int c = 0;

// Baseline captured at arm time
float baselineRoll, baselinePitch, baselineYaw;

// State machine
enum State { REST, ARMED, ALARM };
State currentState = REST;

bool readyForNext      = true;
unsigned long armedAt  = 0;
unsigned long lastToneTime = 0;
bool toneHigh = false;

// ── MPU read ──────────────────────────────────────────────────────
void readMPU() {
  previousTime = currentTime;
  currentTime  = millis();
  elapsedTime  = (currentTime - previousTime) / 1000.0;

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;

  accAngleX = (atan(AccY / sqrt(AccX*AccX + AccZ*AccZ)) * 180 / PI) - AccErrorX;
  accAngleY = (atan(-AccX / sqrt(AccY*AccY + AccZ*AccZ)) * 180 / PI) - AccErrorY;

  Wire.beginTransmission(MPU);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0 - GyroErrorX;
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0 - GyroErrorY;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0 - GyroErrorZ;

  gyroAngleX += GyroX * elapsedTime;
  gyroAngleY += GyroY * elapsedTime;
  yaw        += GyroZ * elapsedTime;

  roll  = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;
}

// ── Calibration ───────────────────────────────────────────────────
void calculate_IMU_error() {
  for (c = 0; c < 200; c++) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccErrorX += atan(AccY / sqrt(AccX*AccX + AccZ*AccZ)) * 180 / PI;
    AccErrorY += atan(-AccX / sqrt(AccY*AccY + AccZ*AccZ)) * 180 / PI;
    delay(5);
  }
  AccErrorX /= 200.0;
  AccErrorY /= 200.0;

  for (c = 0; c < 200; c++) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroErrorX += GyroX;
    GyroErrorY += GyroY;
    GyroErrorZ += GyroZ;
    delay(5);
  }
  GyroErrorX /= 200.0;
  GyroErrorY /= 200.0;
  GyroErrorZ /= 200.0;
}

void captureBaseline() {
  // Read a few times to settle then snapshot
  for (int i = 0; i < 10; i++) { readMPU(); delay(10); }
  baselineRoll  = roll;
  baselinePitch = pitch;
  baselineYaw   = yaw;
}

bool motionDetected() {
  readMPU();
  float dr = abs(roll  - baselineRoll);
  float dp = abs(pitch - baselinePitch);
  float dy = abs(yaw   - baselineYaw);
  Serial.print("dR:"); Serial.print(dr);
  Serial.print(" dP:"); Serial.print(dp);
  Serial.print(" dY:"); Serial.println(dy);
  return (dr > THRESHOLD || dp > THRESHOLD || dy > THRESHOLD);
}

// ── LEDs / Speaker ────────────────────────────────────────────────
void setLEDs(bool g, bool y, bool r) {
  digitalWrite(PIN_LED_GREEN,  g ? HIGH : LOW);
  digitalWrite(PIN_LED_YELLOW, y ? HIGH : LOW);
  digitalWrite(PIN_LED_RED,    r ? HIGH : LOW);
}

void stopAlarm() {
  noTone(PIN_SPEAKER);
  digitalWrite(PIN_SPEAKER, LOW);
}

// ── State transitions ─────────────────────────────────────────────
void enterREST() {
  setLEDs(true, false, false);
  stopAlarm();
  currentState = REST;
  Serial.println("STATE: GREEN");
}

void enterARMED() {
  setLEDs(false, true, false);
  stopAlarm();
  currentState = ARMED;
  armedAt = millis();
  captureBaseline();
  Serial.println("STATE: YELLOW — 2s grace...");
}

void enterALARM() {
  setLEDs(false, false, true);
  currentState = ALARM;
  lastToneTime = millis();
  Serial.println("STATE: RED ALARM");
}

// ── Setup ─────────────────────────────────────────────────────────
void setup() {
  pinMode(PIN_LED_GREEN,  OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED,    OUTPUT);
  pinMode(PIN_SPEAKER,    OUTPUT);
  pinMode(PIN_SWITCH,     INPUT_PULLUP);

  Serial.begin(19200);
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  delay(100);

  currentTime = millis();

  // Boot flash
  setLEDs(true, true, true);
  delay(1000);

  // Calibrate — keep still
  calculate_IMU_error();
  Serial.println("Calibration done.");

  enterREST();
}

// ── Loop ──────────────────────────────────────────────────────────
void loop() {
  bool switchON = (digitalRead(PIN_SWITCH) == LOW);

  if (switchON && readyForNext) {
    readyForNext = false;
    delay(300);
    if (currentState == REST)       enterARMED();
    else if (currentState == ALARM) enterREST();
  }
  if (!switchON) readyForNext = true;

  // Motion check after grace period
  if (currentState == ARMED && millis() - armedAt > 2000) {
    if (motionDetected()) enterALARM();
  }

  // Alarm tone
  if (currentState == ALARM) {
    if (millis() - lastToneTime > 350) {
      tone(PIN_SPEAKER, toneHigh ? 1300 : 750);
      toneHigh = !toneHigh;
      lastToneTime = millis();
    }
  }

  delay(30);
}
