const int ledPin = 13;

void setup() {
    pinMode(ledPin, OUTPUT);

    TCCR1A = 0;
    TCCR1B = 0;

    TCCR1B |= (1 << WGM12);

    OCR1A = 2499;

    TIMSK1 |= (1 << OCIE1A);

    TCCR1B |= (1 << CS12) | (1 << CS10);

    sei();
}

ISR(TIMER1_COMPA_vect) {
    static bool ledState = false;
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
}

void loop() {
    delay(100); 
}