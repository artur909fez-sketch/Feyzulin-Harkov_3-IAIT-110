const int LED1_PIN = 9;   
const int LED2_PIN = 10;   
const int LED3_PIN = 11;  

const int BUTTON1_PIN = 2;
const int BUTTON2_PIN = 3;
const int BUTTON3_PIN = 4;

int button1State = 0;
int button2State = 0;
int button3State = 0;

void setup() {
  DDRB |= (1 << DDB1);  
  DDRB |= (1 << DDB2);  
  DDRB |= (1 << DDB3); 

  DDRD &= ~(1 << DDD2);  
  PORTD |= (1 << PORTD2); 
  
  DDRD &= ~(1 << DDD3);  
  PORTD |= (1 << PORTD3); 
  
  DDRD &= ~(1 << DDD4); 
  PORTD |= (1 << PORTD4); 
  
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
  uint8_t buttonMask = (1 << PIND2) | (1 << PIND3) | (1 << PIND4);
  uint8_t pressedButtons = invertedValue & buttonMask;
  
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

  button1State = PIND & (1 << PIND2);  
  button2State = PIND & (1 << PIND3);  
  button3State = PIND & (1 << PIND4);  
  
  if (button1State == 0) { 
    PORTB |= (1 << PORTB1);  
  } else {
    PORTB &= ~(1 << PORTB1); 
  }

  if (button2State == 0) { 
    PORTB |= (1 << PORTB2);  
  } else {
    PORTB &= ~(1 << PORTB2); 
  }
  
  if (button3State == 0) { 
    PORTB |= (1 << PORTB3); 
  } else {
    PORTB &= ~(1 << PORTB3); 
  }
  delay(200); 
}