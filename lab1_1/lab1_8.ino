const int greenPin = 11;
const int yellowPin = 12;
const int redPin = 13;
const int buttonPin = 2;  

bool buttonPressed = false; 

void setup() {
  pinMode(greenPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); 
  Serial.begin(9600);
}

void loop() {
  while (!buttonPressed) {
    digitalWrite(greenPin, HIGH);
    digitalWrite(yellowPin, LOW);
    digitalWrite(redPin, LOW);

    if (digitalRead(buttonPin) == LOW) { 
      delay(5); 
      if (digitalRead(buttonPin) == HIGH) {
        buttonPressed = true;
        Serial.println("Кнопка нажата!");
      }
    }
  }

  digitalWrite(greenPin, HIGH);
  delay(3000);

  for (int i = 0; i < 3; i++) {
    digitalWrite(greenPin, LOW);
    delay(500);
    digitalWrite(greenPin, HIGH);
    delay(500);
  }
  digitalWrite(greenPin, LOW);

  digitalWrite(yellowPin, HIGH);
  delay(2000);
  digitalWrite(yellowPin, LOW);

  digitalWrite(redPin, HIGH);
  delay(3000);

  for (int i = 0; i < 3; i++) {
    delay(500);
    digitalWrite(redPin, LOW);
    delay(500);
    digitalWrite(redPin, HIGH);
  }

  digitalWrite(yellowPin, HIGH);
  delay(2000); 

  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);

  digitalWrite(greenPin, HIGH);
  delay(500); 

  buttonPressed = false;
  Serial.println("Ожидание следующего нажатия кнопки...");
}