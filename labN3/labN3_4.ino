#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int rowPins[4] = {8, 7, 6, 5}; 
const int colPins[4] = {11, 10, 9, 4}; 
const int KEY_INT_PIN = 2; 

char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

volatile unsigned long timerCount = 0;
volatile bool scanFlag = false;

ISR(TIMER1_COMPA_vect) {
    timerCount++;
}

void startScan() {
    scanFlag = true;
}

void setupKeyboardPins() {
    for(int i = 0; i < 4; i++) {
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], HIGH); 
        pinMode(colPins[i], INPUT_PULLUP); 
    }
}

void setupTimer1() {
    noInterrupts();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 31249; 
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12) | (1 << CS10); 
    TIMSK1 |= (1 << OCIE1A); 
    interrupts();
}

char scanKeyboard() {
    for (int i = 0; i < 4; i++) {
        digitalWrite(rowPins[i], LOW);
        
        delayMicroseconds(100);
        
        for (int j = 0; j < 4; j++) {
            if (digitalRead(colPins[j]) == LOW) {

                while(digitalRead(colPins[j]) == LOW) {
                    delay(10);
                }
                
                digitalWrite(rowPins[i], HIGH);
                return keys[i][j];
            }
        }
        digitalWrite(rowPins[i], HIGH);
    }
    return 0; 
}

void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    setupKeyboardPins();
    setupTimer1();

    pinMode(KEY_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(KEY_INT_PIN), startScan, FALLING);

    interrupts();

    lcd.setCursor(0, 0);
    lcd.print("System Ready   ");
}

void loop() {
    static unsigned long lastDisplayUpdate = 0;
    static unsigned long lastTimerCount = 0;
    
    if (timerCount != lastTimerCount) {
        if (timerCount - lastDisplayUpdate >= 2) { 
            lcd.setCursor(0, 0);
            lcd.print("Count: ");
            lcd.print(timerCount);
            lcd.print("   ");
            lastDisplayUpdate = timerCount;
        }
        lastTimerCount = timerCount;
    }

    if (scanFlag) {
        char pressedKey = scanKeyboard();

        if (pressedKey != 0) {
            lcd.setCursor(0, 1);
            lcd.print("Key: ");
            lcd.print(pressedKey);
            lcd.print("           ");
            Serial.print("Key Press Triggered Scan: ");
            Serial.println(pressedKey);

            if (pressedKey == 'A') {
                timerCount = 0;
                lcd.setCursor(0, 0);
                lcd.print("Count: 0       ");
            }
        } else {
            lcd.setCursor(0, 1);
            lcd.print("No Key         ");
        }
        scanFlag = false;
    }
}