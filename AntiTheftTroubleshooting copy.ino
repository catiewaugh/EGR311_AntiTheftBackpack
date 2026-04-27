/*
  Anti-Theft Backpack — TROUBLESHOOTING CODE v6 (No MPU)
  ========================================================
  Fixed:
    - LED pin assignments corrected (green=D7, yellow=D6, red=D8)
    - State advances IMMEDIATELY when switch latches ON (goes LOW)
    - Waits for switch to latch OFF (goes HIGH) before allowing next advance

  Wiring:
    Switch: one terminal → D4, other terminal → GND
    Green LED  → D7 (with 470Ω to GND)
    Yellow LED → D6 (with 470Ω to GND)
    Red LED    → D8 (with 470Ω to GND)
    Speaker    → D9
*/

const int PIN_SWITCH     = 4;
const int PIN_LED_GREEN  = 8;   // GREEN  on D7
const int PIN_LED_YELLOW = 6;   // YELLOW on D6
const int PIN_LED_RED    = 7;   // RED    on D8
const int PIN_SPEAKER    = 9;

enum State { REST, ARMED, ALARM };
State currentState = REST;

bool readyForNext = true;  // true = we can accept a new ON press

unsigned long lastToneTime = 0;
bool toneHigh = false;

void setLEDs(bool green, bool yellow, bool red) {
  digitalWrite(PIN_LED_GREEN,  green  ? HIGH : LOW);
  digitalWrite(PIN_LED_YELLOW, yellow ? HIGH : LOW);
  digitalWrite(PIN_LED_RED,    red    ? HIGH : LOW);
}

void stopAlarm() {
  noTone(PIN_SPEAKER);
  digitalWrite(PIN_SPEAKER, LOW);
}

void enterREST()  { setLEDs(true,false,false); stopAlarm(); currentState=REST;  Serial.println("STATE: GREEN (REST)"); }
void enterARMED() { setLEDs(false,true,false); stopAlarm(); currentState=ARMED; Serial.println("STATE: YELLOW (ARMED)"); }
void enterALARM() { setLEDs(false,false,true); currentState=ALARM; lastToneTime=millis(); Serial.println("STATE: RED (ALARM)"); }

void setup() {
  pinMode(PIN_LED_GREEN,  OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED,    OUTPUT);
  pinMode(PIN_SPEAKER,    OUTPUT);
  pinMode(PIN_SWITCH,     INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("=== BOOT ===");
  enterREST();
}

void loop() {
  // INPUT_PULLUP: LOW = switch latched ON, HIGH = switch latched OFF
  bool switchON = (digitalRead(PIN_SWITCH) == LOW);

  if (switchON && readyForNext) {
    // Switch just latched ON — advance state immediately, block until OFF
    readyForNext = false;
    switch (currentState) {
      case REST:  enterARMED(); break;
      case ARMED: enterALARM(); break;
      case ALARM: enterREST();  break;
    }
  }

  if (!switchON) {
    // Switch is OFF — allow the next press to register
    readyForNext = true;
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

