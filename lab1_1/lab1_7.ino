const int greenPin = 11;
const int yellowPin = 12;
const int redPin = 13;
const int greenTime = 5;
const int redTime = 6;

void setup() {
  pinMode(greenPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {

  digitalWrite(greenPin, HIGH);
  for (int i = greenTime; i > 0; i--) {
    if (i <= 3) { 
      for (int j = 0; j < 1; j++) { 
        digitalWrite(greenPin, LOW);
        delay(500);
        digitalWrite(greenPin, HIGH);
        delay(500);
      }
    } else {
      delay(2000);
    }

    Serial.print("Зеленый - осталось секунд: ");
    Serial.println(i - 1);
  }
  digitalWrite(greenPin, LOW);


  digitalWrite(yellowPin, HIGH);
  delay(2000);
  digitalWrite(yellowPin, LOW);


  digitalWrite(redPin, HIGH);
  for (int i = redTime; i > 0; i--) {
    delay(1000);
    Serial.print("Красный - осталось секунд: ");
    Serial.println(i - 1);

    if (i == 3) {
      digitalWrite(yellowPin, HIGH); 
    }
  }
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);


  delay(0);
}