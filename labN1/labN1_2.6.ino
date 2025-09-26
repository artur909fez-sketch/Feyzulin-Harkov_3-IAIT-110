const int LED1_PIN = 9;   
const int LED2_PIN = 10;   
const int LED3_PIN = 11;  

const int BUTTON1_PIN = 2;
const int BUTTON2_PIN = 3;
const int BUTTON3_PIN = 4;


uint8_t lastStableState = 0xFF; 
uint8_t currentState = 0xFF;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50; 
const unsigned long LOOP_INTERVAL = 200; 

uint8_t findFirstSetBitLoop(uint8_t value) {
  if (value == 0) return 255; 
  
  for (uint8_t i = 0; i < 8; i++) {
    if (value & (1 << i)) {
      return i;
    }
  }
  return 255; 
}

uint8_t findFirstSetBitCtz(uint8_t value) {
  if (value == 0) return 255; 
  return (uint8_t)__builtin_ctz((unsigned int)value);
}

uint8_t findFirstSetBitFfs(uint8_t value) {
  int result = __builtin_ffs((int)value); 
  if (result == 0) return 255;
  return (uint8_t)(result - 1); 
}


uint8_t readButtonsWithDebounce() {
  uint8_t currentReading = PIND;
  uint8_t buttonMaskD = (1 << PIND2) | (1 << PIND3) | (1 << PIND4);
  uint8_t buttonReading = (~currentReading) & buttonMaskD; 

  if (buttonReading != currentState) {
    lastDebounceTime = millis();
    currentState = buttonReading;
  }
  

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {

    if (buttonReading != lastStableState) {
      lastStableState = buttonReading;
    }
  }
  
  return lastStableState;
}

void setup() {
  DDRB |= (1 << DDB1) | (1 << DDB2) | (1 << DDB3); 

  DDRD &= ~((1 << DDD2) | (1 << DDD3) | (1 << DDD4));  
  PORTD |= (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4); 
  

  currentState = (~PIND) & ((1 << PIND2) | (1 << PIND3) | (1 << PIND4));
  lastStableState = currentState;
  
  Serial.begin(9600);
  Serial.println("Программа запущена (пины светодиодов: 9-11)");
  Serial.println("Режим: антидребезг 50мс, цикл 5 раз/сек");
}

void loop() {
  unsigned long loopStartTime = millis();
  

  uint8_t pressedButtons = readButtonsWithDebounce();
  

  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime > 500) {
    Serial.print("Время: ");
    Serial.print(millis());
    Serial.print(" мс | ");
    
    Serial.print("PIND: ");
    uint8_t portDValue = PIND;
    for (int i = 7; i >= 0; i--) {
      Serial.print((portDValue >> i) & 1);
    }
    
    Serial.print(" | Стабильное состояние кнопок: 0b");
    for (int i = 7; i >= 0; i--) {
      Serial.print((pressedButtons >> i) & 1);
    }
    Serial.println();
    
    lastPrintTime = millis();
  }
  

  uint8_t outputByte = 0;
  if (pressedButtons & (1 << PIND2)) outputByte |= (1 << PORTB1);
  if (pressedButtons & (1 << PIND3)) outputByte |= (1 << PORTB2);
  if (pressedButtons & (1 << PIND4)) outputByte |= (1 << PORTB3);
  

  uint8_t ledMaskB = (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3);
  PORTB = (PORTB & ~ledMaskB) | (outputByte & ledMaskB);
  

  static unsigned long lastDetailTime = 0;
  if (millis() - lastDetailTime > 1000) { 
    Serial.println("--- Детальная информация ---");
    
    Serial.print("Нажатые кнопки (позиции): ");
    bool anyPressed = false;
    for (uint8_t i = 0; i < 8; i++) {
      if (pressedButtons & (1 << i)) {
        Serial.print(i);
        Serial.print(" ");
        anyPressed = true;
      }
    }
    if (!anyPressed) Serial.print("нет");
    Serial.println();
    
  
    uint8_t posLoop = findFirstSetBitLoop(pressedButtons);
    Serial.print("Первый бит (цикл): ");
    if (posLoop != 255) {
      Serial.println(posLoop);
    } else {
      Serial.println("нет");
    }
    
    uint8_t posCtz = findFirstSetBitCtz(pressedButtons);
    Serial.print("Первый бит (ctz): ");
    if (posCtz != 255) {
      Serial.println(posCtz);
    } else {
      Serial.println("нет");
    }
    
    uint8_t posFfs = findFirstSetBitFfs(pressedButtons);
    Serial.print("Первый бит (ffs): ");
    if (posFfs != 255) {
      Serial.println(posFfs);
    } else {
      Serial.println("нет");
    }
    
    Serial.print("Сформированный байт: 0b");
    for (int i = 7; i >= 0; i--) {
      Serial.print((outputByte >> i) & 1);
    }
    Serial.println();
    
    Serial.println("----------------------------");
    lastDetailTime = millis();
  }
  
 
  unsigned long elapsedTime = millis() - loopStartTime;
  if (elapsedTime < LOOP_INTERVAL) {
    delay(LOOP_INTERVAL - elapsedTime);
  }
  

  static bool blinkState = false;
  static unsigned long lastBlinkTime = 0;
  if (millis() - lastBlinkTime > 1000) {
    blinkState = !blinkState;
    if (blinkState) {
      PORTB |= (1 << PORTB5); 
    } else {
      PORTB &= ~(1 << PORTB5); 
    }
    lastBlinkTime = millis();
  }
}