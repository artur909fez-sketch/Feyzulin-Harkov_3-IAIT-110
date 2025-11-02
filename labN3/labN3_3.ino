
const int rowPins[4] = {8, 7, 6, 5};
const int colPins[4] = {11, 10, 9, 4};

char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

volatile byte currentRow = 0;
volatile bool keyDetected = false;
volatile char pressedKey = '\0';
volatile char lastKey = '\0'; 
volatile unsigned long lastKeyTime = 0; 

void setupKeyboardPins() {
    for(int i = 0; i < 4; i++) {
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], HIGH);
    }
    for(int i = 0; i < 4; i++) {
        pinMode(colPins[i], INPUT_PULLUP);
    }
}

void setupTimer0() {
    noInterrupts();
    TCCR0A = 0;
    TCCR0B = 0;
    TCNT0 = 0;
    TCCR0A |= (1 << WGM01);
    TCCR0B |= (1 << CS01) | (1 << CS00);
    OCR0A = 249; // 1 мс
    TIMSK0 |= (1 << OCIE0A);
    interrupts();
}

ISR(TIMER0_COMPA_vect) {
    digitalWrite(rowPins[currentRow], HIGH);

    currentRow = (currentRow + 1) % 4;

    digitalWrite(rowPins[currentRow], LOW);

    for(int col = 0; col < 4; col++) {
        if(digitalRead(colPins[col]) == LOW) {
            char newKey = keys[currentRow][col];
            
            if(newKey != lastKey || (millis() - lastKeyTime) > 200) {
                pressedKey = newKey;
                keyDetected = true;
                lastKey = newKey;
                lastKeyTime = millis();
            }
            break;
        }
    }
}

void setup() {
    Serial.begin(9600);
    setupKeyboardPins();
    setupTimer0();
    Serial.println("Клавиатура 4x4 инициализирована");
}

void loop() {
    if (keyDetected && pressedKey != '\0') {
        Serial.print("Нажата клавиша: ");
        Serial.println(pressedKey);
        keyDetected = false;
        
        delay(100);
    }
}