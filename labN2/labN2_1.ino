enum State {
  S_GREEN,
  S_YELLOW, 
  S_RED,
  S_WARNING,
  NUM_STATES
};

enum Event {
  E_NONE,
  E_TIMER_EXPIRE,
  E_PED_REQ,
  E_EMERGENCY_ON,
  E_EMERGENCY_OFF,
  E_NIGHT_MODE_TOGGLE,
  NUM_EVENTS
};

const int RED_PIN = 13;
const int YELLOW_PIN = 12;
const int GREEN_PIN = 11;
const int BUTTON_PIN = 2;

const unsigned long GREEN_TIME = 10000;
const unsigned long YELLOW_TIME = 3000;
const unsigned long RED_TIME = 10000;
const unsigned long PED_EXTRA_TIME = 5000;
const unsigned long BLINK_INTERVAL = 500;
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long LONG_PRESS_TIME = 3000;

State currentState = S_GREEN;
unsigned long stateStartTime = 0;
unsigned long stateDuration = 0;
bool pedRequest = false;
bool emergency = false;
bool nightMode = false;

bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
bool buttonPressed = false;
unsigned long buttonPressStart = 0;

unsigned long lastBlinkTime = 0;
bool yellowBlinkState = false;

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.begin(9600);
  Serial.println("Светофор запущен: Метод 1, классический");
  
  goToState(S_GREEN, GREEN_TIME);
}

void goToState(State newState, unsigned long duration) {
  currentState = newState;
  stateStartTime = millis();
  stateDuration = duration;
  setOutputsForState(newState);
  
  logStateTransition(newState);
}

void setOutputsForState(State state) {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  
  switch(state) {
    case S_GREEN:
      digitalWrite(GREEN_PIN, HIGH);
      break;
    case S_YELLOW:
      digitalWrite(YELLOW_PIN, HIGH);
      break;
    case S_RED:
      digitalWrite(RED_PIN, HIGH);
      break;
    case S_WARNING:
      digitalWrite(YELLOW_PIN, HIGH);
      yellowBlinkState = true;
      lastBlinkTime = millis();
      break;
  }
}

void logStateTransition(State newState) {
  Serial.print("Состояние: ");
  switch(newState) {
    case S_GREEN: Serial.print("S_GREEN"); break;
    case S_YELLOW: Serial.print("S_YELLOW"); break;
    case S_RED: Serial.print("S_RED"); break;
    case S_WARNING: Serial.print("S_WARNING"); break;
  }
  Serial.print(" Время: ");
  Serial.println(millis());
}

void readInputs() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading == LOW && !buttonPressed) {
      buttonPressed = true;
      pedRequest = true;
      Serial.println("Событие: E_PED_REQ");
    } else if (reading == HIGH) {
      buttonPressed = false;
    }
  }
  
  lastButtonState = reading;

  static unsigned long buttonPressStart = 0;
  if (reading == LOW && buttonPressStart == 0) {
    buttonPressStart = millis();
  }
  if (reading == HIGH && buttonPressStart != 0) {
    unsigned long pressDuration = millis() - buttonPressStart;
    if (pressDuration > LONG_PRESS_TIME) {
      nightMode = !nightMode;
      Serial.print("Состояние ночного режима: ");
      Serial.println(nightMode ? "Активирован" : "Деактивирован");
    }
    buttonPressStart = 0;
  }
}

void loop() {
  readInputs();

  if (emergency || nightMode) {
    if (currentState != S_WARNING) {
      goToState(S_WARNING, BLINK_INTERVAL);
    }
  }

  switch(currentState) {
    case S_GREEN:
      if (millis() - stateStartTime >= stateDuration) {
        if (pedRequest) {
          goToState(S_YELLOW, YELLOW_TIME);} else {
          goToState(S_YELLOW, YELLOW_TIME);
        }
      }
      break;
      
    case S_YELLOW:
      if (millis() - stateStartTime >= stateDuration) {
        if (pedRequest) {
          pedRequest = false;
          goToState(S_RED, RED_TIME + PED_EXTRA_TIME);
        } else {
          goToState(S_RED, RED_TIME);
        }
      }
      break;
      
    case S_RED:
      if (millis() - stateStartTime >= stateDuration) {
        goToState(S_GREEN, GREEN_TIME);
      }
      break;
      
    case S_WARNING:
      
      if (millis() - lastBlinkTime >= BLINK_INTERVAL) {
        yellowBlinkState = !yellowBlinkState;
        digitalWrite(YELLOW_PIN, yellowBlinkState ? HIGH : LOW);
        lastBlinkTime = millis();
      }
      
     
      if (!emergency && !nightMode) {
        goToState(S_GREEN, GREEN_TIME);
      }
      break;
  }
  

  static unsigned long lastEmergencyToggle = 0;
  if (millis() - lastEmergencyToggle > 30000) {
    emergency = !emergency;
    Serial.print("Состояние аварийного режима: ");
    Serial.println(emergency ? "Активирован" : "Деактивирован");
    lastEmergencyToggle = millis();
  }
}