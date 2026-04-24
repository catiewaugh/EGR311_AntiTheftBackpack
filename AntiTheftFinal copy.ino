#include <Wire.h>

// PIN SETUP
const int buttonPin = 4;

const int yellowLED = 6;
const int greenLED  = 8;
const int redLED    = 7;

const int speakerPin = 9;

// MPU
const int MPU_ADDR = 0x68;

// STATE
int lastButtonState;

bool systemArmed = false;
bool monitoringStarted = false;
bool alarmActive = false;

unsigned long armStartTime = 0;

// BASELINE
float baseAx = 0, baseAy = 0, baseAz = 0;
float baseGx = 0, baseGy = 0, baseGz = 0;

// THRESHOLDS
const float accelThreshold = 0.80;
const float gyroThreshold  = 120.0;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  pinMode(speakerPin, OUTPUT);

  Serial.begin(9600);
  Wire.begin();

  // Wake MPU
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  delay(500);

  // Calibrate baseline (keep still during this)
  calibrateMPU();

  setGreen();
  noTone(speakerPin);

  lastButtonState = digitalRead(buttonPin);
}

void loop() {
  int currentButtonState = digitalRead(buttonPin);

  // Detect switch change
  if (currentButtonState != lastButtonState) {
    delay(50);
    currentButtonState = digitalRead(buttonPin);

    if (currentButtonState != lastButtonState) {
      lastButtonState = currentButtonState;

      // SWITCH ON
      if (currentButtonState == LOW) {
        systemArmed = true;
        monitoringStarted = false;
        alarmActive = false;

        armStartTime = millis();

        setYellow();
        noTone(speakerPin);
      }

      // SWITCH OFF
      else {
        systemArmed = false;
        monitoringStarted = false;
        alarmActive = false;

        noTone(speakerPin);
        setGreen();
      }
    }
  }

  // Delay before monitoring
  if (systemArmed && !monitoringStarted) {
    if (millis() - armStartTime >= 2500) {
      monitoringStarted = true;
      setYellow();
    }
  }

  // Motion detection
  if (systemArmed && monitoringStarted && !alarmActive) {
    float ax, ay, az, gx, gy, gz;
    readMPU(ax, ay, az, gx, gy, gz);

    float accelChange = sqrt(
      sq(ax - baseAx) +
      sq(ay - baseAy) +
      sq(az - baseAz)
    );

    float gyroMag = sqrt(
      sq(gx - baseGx) +
      sq(gy - baseGy) +
      sq(gz - baseGz)
    );

    Serial.print("Accel: ");
    Serial.print(accelChange);
    Serial.print(" | Gyro: ");
    Serial.println(gyroMag);

    if (accelChange > accelThreshold || gyroMag > gyroThreshold) {
      alarmActive = true;
      setRed();
      tone(speakerPin, 1500);
    }
  }
}

// LEDS
void setGreen() {
  digitalWrite(greenLED, HIGH);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
}

void setYellow() {
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, HIGH);
  digitalWrite(redLED, LOW);
}

void setRed() {
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, HIGH);
}

// MPU READ
void readMPU(float &ax, float &ay, float &az,
             float &gx, float &gy, float &gz) {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  int16_t rawAx = Wire.read()<<8 | Wire.read();
  int16_t rawAy = Wire.read()<<8 | Wire.read();
  int16_t rawAz = Wire.read()<<8 | Wire.read();

  Wire.read(); Wire.read(); // skip temp

  int16_t rawGx = Wire.read()<<8 | Wire.read();
  int16_t rawGy = Wire.read()<<8 | Wire.read();
  int16_t rawGz = Wire.read()<<8 | Wire.read();

  ax = rawAx / 16384.0;
  ay = rawAy / 16384.0;
  az = rawAz / 16384.0;

  gx = rawGx / 131.0;
  gy = rawGy / 131.0;
  gz = rawGz / 131.0;
}

// CALIBRATION
void calibrateMPU() {
  float sumAx = 0, sumAy = 0, sumAz = 0;
  float sumGx = 0, sumGy = 0, sumGz = 0;

  for (int i = 0; i < 100; i++) {
    float ax, ay, az, gx, gy, gz;
    readMPU(ax, ay, az, gx, gy, gz);

    sumAx += ax;
    sumAy += ay;
    sumAz += az;

    sumGx += gx;
    sumGy += gy;
    sumGz += gz;

    delay(10);
  }

  baseAx = sumAx / 100.0;
  baseAy = sumAy / 100.0;
  baseAz = sumAz / 100.0;

  baseGx = sumGx / 100.0;
  baseGy = sumGy / 100.0;
  baseGz = sumGz / 100.0;
}
