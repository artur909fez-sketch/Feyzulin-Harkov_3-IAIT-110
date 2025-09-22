const int potPin = A0;
const int ledPin = 11;

int potValue = 0;
int ledBrightness = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  potValue = analogRead(potPin);

  ledBrightness = map(potValue, 0, 1023, 0, 255);

  analogWrite(ledPin, ledBrightness);

  Serial.print("Значение потенциометра: ");
  Serial.println(potValue);

  delay(10);
}