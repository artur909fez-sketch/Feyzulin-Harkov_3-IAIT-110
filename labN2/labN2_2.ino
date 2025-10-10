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

struct Transition {
  State nextState;
  unsigned long duration;
};

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

Transition stateTable[NUM_STATES][NUM_EVENTS] = {
  {
    {S_GREEN, GREEN_TIME},
    {S_YELLOW, YELLOW_TIME},
    {S_GREEN, GREEN_TIME},
    {S_WARNING, BLINK_INTERVAL},
    {S_GREEN, GREEN_TIME},
    {S_WARNING, BLINK_INTERVAL}
  },
  {
    {S_YELLOW, YELLOW_TIME},
    {S_RED, RED_TIME},
    {S_YELLOW, YELLOW_TIME},
    {S_WARNING, BLINK_INTERVAL},
    {S_YELLOW, YELLOW_TIME},
    {S_WARNING, BLINK_INTERVAL}
  },
  {
    {S_RED, RED_TIME},
    {S_GREEN, GREEN_TIME},
    {S_RED, RED_TIME},
    {S_WARNING, BLINK_INTERVAL},
    {S_RED, RED_TIME},
    {S_WARNING, BLINK_INTERVAL}
  },
  {
    {S_WARNING, BLINK_INTERVAL},
    {S_WARNING, BLINK_INTERVAL},
    {S_WARNING, BLINK_INTERVAL},
    {S_WARNING, BLINK_INTERVAL},
    {S_GREEN, GREEN_TIME},
    {S_GREEN, GREEN_TIME}
  }
};

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.begin(9600);
  Serial.println("Светофор запущен: Метод 2, табличный");
  
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
  Serial.print(millis());
  Serial.print(" мc > Переход: ");
  switch(newState) {
    case S_GREEN: Serial.println("Зеленый"); break;
    case S_YELLOW: Serial.println("Желтый"); break;
    case S_RED: Serial.println("Красный"); break;
    case S_WARNING: Serial.println("Мигающий Желтый"); break;
  }
}

Event readInputs() {
  int reading = digitalRead(BUTTON_PIN);
  Event event = E_NONE;
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading == LOW && !buttonPressed) {
      buttonPressed = true;
      
      if (!pedRequest) {
        pedRequest = true;
        Serial.println("Получен запрос пешехода, зеленый горит полный цикл");
      }
    } else if (reading == HIGH) {
      buttonPressed = false;
    }
  }
  
  static unsigned long buttonPressStart = 0;
  if (reading == LOW && buttonPressStart == 0) {
    buttonPressStart = millis();
  }
  if (reading == HIGH && buttonPressStart != 0) {
    unsigned long pressDuration = millis() - buttonPressStart;
    if (pressDuration > LONG_PRESS_TIME) {
      nightMode = !nightMode;
      event = E_NIGHT_MODE_TOGGLE;
      Serial.print("Состояние ночного режима: ");
      Serial.println(nightMode ? "Активирован" : "Деактивирован");
    }
    buttonPressStart = 0;
  }
  
  lastButtonState = reading;
  return event;
}

void processEvent(Event event) {
  if (event == E_NONE) return;
  
  Transition transition = stateTable[currentState][event];
  
  switch(currentState) {
    case S_GREEN:
      if (event == E_TIMER_EXPIRE && pedRequest) {
        transition.nextState = S_YELLOW;
        transition.duration = YELLOW_TIME;
        Serial.println("Зеленый догорел, переход к желтому с учетом запроса пешехода");
      }
      break;
      
    case S_YELLOW:
      if (event == E_TIMER_EXPIRE && pedRequest) {
        transition.nextState = S_RED;
        transition.duration = RED_TIME + PED_EXTRA_TIME;
        pedRequest = false;
        Serial.println("Дополнительное время для пешеходов");
      }
      break;
      
    case S_WARNING:
      if (event == E_EMERGENCY_OFF && nightMode) {
        transition.nextState = S_WARNING;
        transition.duration = BLINK_INTERVAL;
      }
      if (event == E_NIGHT_MODE_TOGGLE && !nightMode) {
        transition.nextState = S_GREEN;
        transition.duration = GREEN_TIME;
      }
      break;
  }
  
  if (transition.nextState != currentState) {
    goToState(transition.nextState, transition.duration);
  }
}

void loop() {
  Event event = readInputs();
  
  static bool lastEmergency = false;
  if (emergency && !lastEmergency) {
    processEvent(E_EMERGENCY_ON);
  } else if (!emergency && lastEmergency) {
    processEvent(E_EMERGENCY_OFF);
  }
  lastEmergency = emergency;
  
  if (millis() - stateStartTime >= stateDuration) {
    processEvent(E_TIMER_EXPIRE);
  }
  
  processEvent(event);
  
  if (currentState == S_WARNING) {
    if (millis() - lastBlinkTime >= BLINK_INTERVAL) {
      yellowBlinkState = !yellowBlinkState;
      digitalWrite(YELLOW_PIN, yellowBlinkState ? HIGH : LOW);
      lastBlinkTime = millis();
    }
  }
  
  static unsigned long lastEmergencyToggle = 0;
  if (millis() - lastEmergencyToggle > 28000) {
    emergency = !emergency;
    Serial.print("Состояние аварийного режима: ");
    Serial.println(emergency ? "Активирован" : "Деактивирован");
    lastEmergencyToggle = millis();
  }
}