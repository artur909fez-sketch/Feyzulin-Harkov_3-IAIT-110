const int redPin = 11;   
const int greenPin = 10; 
const int bluePin = 9;  
const int encoderCLK = 2;    
const int encoderDT = 3;     
const int encoderSW = 4;     

int lastCLKState;
int currentCLKState;
int brightness = 255;        
bool buttonPressed = false;
bool brightnessMode = false; 

int currentRed = 0;
int currentGreen = 255;
int currentBlue = 255;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(encoderCLK, INPUT);
  pinMode(encoderDT, INPUT);
  pinMode(encoderSW, INPUT_PULLUP);
  
  Serial.begin(9600);
  
  lastCLKState = digitalRead(encoderCLK);
}

void setColor(int red, int green, int blue) {
  float brightFactor = brightness / 255.0;
  analogWrite(redPin, red * brightFactor);
  analogWrite(greenPin, green * brightFactor);
  analogWrite(bluePin, blue * brightFactor);

  currentRed = red;
  currentGreen = green;
  currentBlue = blue;
}

float smoothTransition(float start, float end, float progress) {
  float eased = progress * progress * (3.0 - 2.0 * progress);
  return start + (end - start) * eased;
}

void handleEncoder() {
  currentCLKState = digitalRead(encoderCLK);
  
  if (currentCLKState != lastCLKState && currentCLKState == HIGH) {
    if (digitalRead(encoderDT) != currentCLKState) {
      if (brightnessMode) {
        brightness = constrain(brightness + 5, 0, 255);
      }
    } else {
      if (brightnessMode) {
        brightness = constrain(brightness - 5, 0, 255);
      }
    }
    Serial.print("Brightness: ");
    Serial.println(brightness);
  }
  lastCLKState = currentCLKState;
}

void handleButton() {
  if (digitalRead(encoderSW) == LOW && !buttonPressed) {
    delay(50);
    if (digitalRead(encoderSW) == LOW) {
      buttonPressed = true;
      brightnessMode = !brightnessMode;
      
      if (brightnessMode) {
        Serial.println("Brightness mode ON");
      } else {
        Serial.println("Color mode ON");
      }
    }
  }
  
  if (digitalRead(encoderSW) == HIGH && buttonPressed) {
    buttonPressed = false;
    delay(50);
  }
}

void loop() {
  handleEncoder();
  handleButton();
  
  if (!brightnessMode) {
    int potValue = analogRead(A0);
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
    
    Serial.print("Color RGB: ");
    Serial.print(redValue);
    Serial.print(", ");
    Serial.print(greenValue);
    Serial.print(", ");
    Serial.print(blueValue);
    Serial.print(" | Brightness: ");
    Serial.println(brightness);
  } else {
    setColor(currentRed, currentGreen, currentBlue);
  }
  
  delay(10);
}