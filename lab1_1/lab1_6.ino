const int potPin = A0;
const int ledPin = 11;
const int buttonPin = 2;

int potValue = 0;
int ledBrightness = 0;
int savedBrightness = 0;
int buttonState = HIGH;
int lastButtonState = HIGH;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  buttonState = digitalRead(buttonPin);

  if (buttonState != lastButtonState) {
    delay(50);
    if (digitalRead(buttonPin) != buttonState) {
      lastButtonState = buttonState;
    }
  }

  if (buttonState == LOW) {
    if (lastButtonState == HIGH) {
      savedBrightness = ledBrightness;
    }

    analogWrite(ledPin, savedBrightness);

    Serial.print("Изменение яркости невозможно! Положение потенциометра: ");
    potValue = analogRead(potPin);
    Serial.println(potValue);
  } else {
    potValue = analogRead(potPin);
    ledBrightness = map(potValue, 0, 1023, 0, 255);
    analogWrite(ledPin, ledBrightness);
  }

  lastButtonState = buttonState;
  delay(10);
}