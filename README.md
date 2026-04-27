### Code Description (Final MPU-6050 Anti-Theft Backpack System)

This program is a motion-based security system using an **Arduino Nano and MPU-6050 sensor**. It has three states: **REST (safe), ARMED (monitoring), and ALARM (intrusion detected)**. The system is turned on and off using a latching switch. When ARMED, the sensor records a baseline position, then continuously checks for movement. If movement exceeds a set threshold, the system triggers an alarm with a red LED and buzzer sound. A short delay after arming prevents false alarms during setup movement.

---

### Variable Descriptions

#### Pins

* `PIN_SWITCH (4)` → Switch to change system mode (REST / ARMED / reset ALARM)
* `PIN_LED_GREEN (8)` → Green LED for REST (safe)
* `PIN_LED_YELLOW (6)` → Yellow LED for ARMED (monitoring)
* `PIN_LED_RED (7)` → Red LED for ALARM (intrusion)
* `PIN_SPEAKER (9)` → Buzzer for alarm sound

---

#### MPU-6050 Sensor

* `AccX, AccY, AccZ` → Detect movement in straight-line directions
* `GyroX, GyroY, GyroZ` → Detect rotation/motion speed
* `AccError* / GyroError*` → Calibration values to reduce sensor noise

---

#### Motion Tracking

* `roll, pitch, yaw` → Current orientation of the sensor
* `baselineRoll, baselinePitch, baselineYaw` → Reference position when system is armed
* `THRESHOLD (0.5)` → Minimum movement needed to trigger alarm

---

#### System Control

* `currentState` → Tracks system mode (REST, ARMED, ALARM)
* `readyForNext` → Prevents multiple switch triggers per press
* `armedAt` → Timer used for 2-second delay after arming

---

#### Alarm Timing

* `lastToneTime` → Controls speed of buzzer beeping
* `toneHigh` → Switches between two alarm tones

---

### How It Works

* System starts in **REST (green LED)**
* Switch → **ARMED (yellow LED)**
* Sensor takes a **baseline reading**
* After 2 seconds, movement is checked
* If movement is too large → **ALARM (red LED + buzzer)**
* Switch can reset system back to REST
* Alarm sound alternates between two tones for warning effect









Troubleshooting Code Description (v6)

This version implements a simplified anti-theft control system without the MPU-6050 sensor. 
The system uses a latching switch to cycle through three states: REST, ARMED, and ALARM. 
Each full switch toggle (ON → OFF → ON) advances the state machine exactly once, preventing multiple triggers from a single input. 
LEDs provide direct visual feedback for each state, while the buzzer activates only in the ALARM state with an alternating tone pattern. 
The design focuses on validating core hardware functionality (switch, LEDs, and speaker) and ensuring stable state transitions before integrating motion sensing.


Variable / Component Descriptions
PIN_SWITCH — Digital input from latching switch used to control state transitions
PIN_LED_GREEN — Output pin controlling green LED (REST state indicator)
PIN_LED_YELLOW — Output pin controlling yellow LED (ARMED state indicator)
PIN_LED_RED — Output pin controlling red LED (ALARM state indicator)
PIN_SPEAKER — Output pin controlling buzzer sound generation
State (REST, ARMED, ALARM) — Finite state machine defining system mode
currentState — Stores current active system state
readyForNext — Debounce-style logic flag ensuring one state change per full switch cycle
lastToneTime — Timing reference used to control buzzer frequency switching
toneHigh — Toggles between two alarm frequencies for siren effect
setLEDs() — Controls LED outputs based on active state
stopAlarm() — Disables buzzer output
enterREST() — Sets system to idle state
enterARMED() — Sets system to monitoring state
enterALARM() — Sets system to alarm state with sound activation

