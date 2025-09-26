const int LED1_PIN = 9;   
const int LED2_PIN = 10;   
const int LED3_PIN = 11;  

const int BUTTON1_PIN = 2;
const int BUTTON2_PIN = 3;
const int BUTTON3_PIN = 4;

void setup() {

  DDRB |= (1 << DDB1) | (1 << DDB2) | (1 << DDB3); 


  DDRD &= ~((1 << DDD2) | (1 << DDD3) | (1 << DDD4));  
  PORTD |= (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4); 
  
  Serial.begin(9600);
  Serial.println("Программа запущена (пины светодиодов: 9-11)");
}

void loop() {
  uint8_t portDValue = PIND;

  Serial.print("PIND: ");
  for (int i = 7; i >= 0; i--) {
    Serial.print((portDValue >> i) & 1);
  }
  Serial.println();


  uint8_t invertedValue = ~portDValue;
  uint8_t buttonMaskD = (1 << PIND2) | (1 << PIND3) | (1 << PIND4);
  uint8_t pressedButtons = invertedValue & buttonMaskD;
  
  Serial.print("Нажатые кнопки (позиции бит с единицами): ");
  bool anyPressed = false;
  for (uint8_t i = 0; i < 8; i++) {
    if (pressedButtons & (1 << i)) {
      Serial.print(i);
      Serial.print(" ");
      anyPressed = true;
    }
  }
  if (!anyPressed) {
    Serial.print("нет");
  }
  Serial.println();


  uint8_t outputByte = 0;
  if (pressedButtons & (1 << PIND2)) outputByte |= (1 << PORTB1);
  if (pressedButtons & (1 << PIND3)) outputByte |= (1 << PORTB2);
  if (pressedButtons & (1 << PIND4)) outputByte |= (1 << PORTB3);
  

  Serial.print("Сформированный байт для PORTB: 0b");
  for (int i = 7; i >= 0; i--) {
    Serial.print((outputByte >> i) & 1);
  }
  Serial.println();


  uint8_t ledMaskB = (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3);

  // Способ 1: Управление через маску 
  PORTB = (PORTB & ~ledMaskB) | (outputByte & ledMaskB);

  /*
  // Способ 2: Управление через bitSet/bitClear
  if (outputByte & (1 << PORTB1)) {
    bitSet(PORTB, PORTB1);
  } else {
    bitClear(PORTB, PORTB1);
  }
  if (outputByte & (1 << PORTB2)) {
    bitSet(PORTB, PORTB2);
  } else {
    bitClear(PORTB, PORTB2);
  }
  if (outputByte & (1 << PORTB3)) {
    bitSet(PORTB, PORTB3);
  } else {
    bitClear(PORTB, PORTB3);
  }
  */
  
  Serial.println("---");
  delay(200); 
}