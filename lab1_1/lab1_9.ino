const int redPin = 13;   
const int greenPin = 12; 
const int bluePin = 11;  
const int buttonPin = 2; 

bool buttonPressed = false;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);

  setColor(255, 0, 255);
}

void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

void loop() {
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    while (digitalRead(buttonPin) == LOW);
    Serial.println("Кнопка нажата!"); 
    delay(2500);
    for (int i = 0; i < 3; i++) {
      setColor(255, 0, 255);
      delay(500);
      setColor(0, 0, 0);
      delay(500);
    }

    setColor(0, 0, 255);
    delay(1500);

    setColor(0, 255, 255);
    delay(3500);

    for (int i = 0; i < 3; i++) {
      setColor(0, 255, 255);
      delay(500);
      setColor(0, 0, 0);
      delay(500);
    }

    setColor(0, 0, 255);
    delay(1500);

    setColor(255, 0, 255);
    buttonPressed = false; 
  }

  else if (!buttonPressed) {
      setColor(255, 0, 255);
  }
}