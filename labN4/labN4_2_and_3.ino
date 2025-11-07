const uint8_t segPins[] = {2,3,4,5,6,7,8,9};
const uint8_t digitPins[] = {10,11,12,13};

const uint8_t digitPatterns[10] = {
  0b00111111,
  0b00000110,
  0b01011011,
  0b01001111,
  0b01100110,
  0b01101101,
  0b01111101,
  0b00000111,
  0b01111111,
  0b01101111
};

int displayNumber = 1234;
unsigned long lastUpdate = 0;
const int DIGIT_DELAY = 5;
int currentDigit = 0;

void setup() {
  for (int i = 0; i < 8; i++) pinMode(segPins[i], OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH);
  }
}

void loop() {
  if (millis() - lastUpdate >= DIGIT_DELAY) {
    lastUpdate = millis();
    updateDisplay();
    displayNumber = analogRead(A0) / 1023.0 * 9999;
  }
}

void updateDisplay() {
  digitalWrite(digitPins[currentDigit], HIGH);
  currentDigit = (currentDigit + 1) % 4;
  int digitValue = getDigit(displayNumber, 3 - currentDigit);
  setSegments(digitPatterns[digitValue]);
  digitalWrite(digitPins[currentDigit], LOW);
}

int getDigit(int number, int position) {
  for (int i = 0; i < position; i++) number /= 10;
  return number % 10;
}

void setSegments(uint8_t pattern) {
  for (int i = 0; i < 8; i++) digitalWrite(segPins[i], (pattern >> i) & 1);
}