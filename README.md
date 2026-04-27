






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

