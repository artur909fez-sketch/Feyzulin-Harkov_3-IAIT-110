const int redPin = 11;   
const int greenPin = 10; 
const int bluePin = 9;  
const int potPin = A0;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.begin(9600);
}

void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

float smoothTransition(float start, float end, float progress) {
  float eased = progress * progress * (3.0 - 2.0 * progress);
  return start + (end - start) * eased;
}

void loop() {
  int potValue = analogRead(potPin);
  float position = potValue / 1023.0;
  
  int segment = position * 5;
  float segmentPos = (position * 5) - segment;
  
  int redValue, greenValue, blueValue;
  
  switch(segment) {
    case 0:
      redValue = 0;
      greenValue = smoothTransition(255, 0, segmentPos);
      blueValue = 255;
      break;
      
    case 1: 
      redValue = smoothTransition(0, 255, segmentPos);
      greenValue = 0;
      blueValue = 255;
      break;
      
    case 2: 
      redValue = 255;
      greenValue = 0;
      blueValue = smoothTransition(255, 0, segmentPos);
      break;
      
    case 3: 
      redValue = 255;
      greenValue = smoothTransition(0, 255, segmentPos);
      blueValue = 0;
      break;
      
    case 4: 
      redValue = smoothTransition(255, 0, segmentPos);
      greenValue = 255;
      blueValue = smoothTransition(0, 255, segmentPos);
      break;
      
    default:
      redValue = 0;
      greenValue = 255;
      blueValue = 255;
  }
  
  redValue = constrain(redValue, 0, 255);
  greenValue = constrain(greenValue, 0, 255);
  blueValue = constrain(blueValue, 0, 255);
  
  setColor(redValue, greenValue, blueValue);
  
  Serial.print("RGB: ");
  Serial.print(redValue);
  Serial.print(", ");
  Serial.print(greenValue);
  Serial.print(", ");
  Serial.println(blueValue);
  
  delay(10);
}