const int LED_PIN = 13;
const int BUTTON_PIN = 2;
volatile bool fastMode = false;
unsigned long previousMillis = 0;
int ledState = LOW;
bool lastButtonState = HIGH;


const unsigned long SLOW_INTERVAL = 1000;
const unsigned long FAST_INTERVAL = 200;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, CHANGE);
  
  Serial.begin(9600);
  Serial.println("Программа запущена с режимом Change");
}

void loop() {
  unsigned long currentInterval = fastMode ? FAST_INTERVAL : SLOW_INTERVAL;
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= currentInterval) {
    previousMillis = currentMillis;
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(LED_PIN, ledState);
  }
}

void buttonISR() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  int currentButtonState = digitalRead(BUTTON_PIN);
  
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    if (interruptTime - lastInterruptTime > 200) {
      fastMode = !fastMode;
      previousMillis = interruptTime;
      ledState = LOW;
      digitalWrite(LED_PIN, ledState);
      
      Serial.print("Режим работы: ");
      Serial.println(fastMode ? "Интенсивный" : "Медленный");
    }
    lastInterruptTime = interruptTime;
  }
  
  lastButtonState = currentButtonState;
}