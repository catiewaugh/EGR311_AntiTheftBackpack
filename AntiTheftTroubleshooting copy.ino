// PIN SETUP
const int buttonPin = 4;

const int yellowLED = 6;
const int greenLED  = 8;
const int redLED    = 7;

const int speakerPin = 9;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  pinMode(speakerPin, OUTPUT);

  Serial.begin(9600);

  Serial.println("Starting no-MPU test...");
}

void loop() {

  // BUTTONS
  bool buttonOn = (digitalRead(buttonPin) == LOW);

  Serial.print("Button: ");
  Serial.println(buttonOn ? "ON" : "OFF");

  // LEDS
  Serial.println("Testing LEDs...");

  digitalWrite(greenLED, HIGH);
  delay(2000);
  digitalWrite(greenLED, LOW);

  digitalWrite(yellowLED, HIGH);
  delay(2000);
  digitalWrite(yellowLED, LOW);

  digitalWrite(redLED, HIGH);
  delay(2000);
  digitalWrite(redLED, LOW);

  delay(2000);

  // SPEAKER
  Serial.println("Testing speaker...");

  tone(speakerPin, 1000);
  delay(1000);

  tone(speakerPin, 1500);
  delay(1000);

  noTone(speakerPin);

  delay(2000);

  // BUTTON-CONTROL
  Serial.println("Button controls output now...");

  for (int i = 0; i < 20; i++) {
    buttonOn = (digitalRead(buttonPin) == LOW);

    if (buttonOn) {
      // ON → red + sound
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
      tone(speakerPin, 1500);
    } else {
      // OFF → green only
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, LOW);
      digitalWrite(yellowLED, LOW);
      noTone(speakerPin);
    }

    delay(200);
  }

  Serial.println("---- Restarting Test Loop ----");
  delay(2000);
}
