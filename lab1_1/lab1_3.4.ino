const int ledPin = 13;      
const int buttonPin = 2;     

void setup() {
  pinMode(ledPin, OUTPUT);     
  pinMode(buttonPin, INPUT_PULLUP); 
  Serial.begin(9600);           
  Serial.println("Программа запущена."); 
}

void loop() {
  
  int buttonState = digitalRead(buttonPin);

  
  if (buttonState == LOW) { 
    digitalWrite(ledPin, HIGH);
    Serial.println("Кнопка нажата!"); 
  } else {
    
    digitalWrite(ledPin, LOW);
    Serial.println("Кнопка отпущена."); 
  }

  delay(50); 
}