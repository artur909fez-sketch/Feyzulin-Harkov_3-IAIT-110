#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

String messages[] = {
  "System Ready",
  "Temp: 25.5C",
  "Humidity: 60%",
  "Pressure: 1013",
  "All Systems OK"
};

int currentMessage = 0;
unsigned long lastChange = 0;
bool showCustom = false;

void setup() {
  lcd.begin(16, 2);
  lcd.print("Arduino LCD");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {
  if (millis() - lastChange > 2000) {
    lastChange = millis();
    
    lcd.clear();
    
    if (showCustom) {
      lcd.setCursor(0, 0);
      lcd.print("Custom Message:");
      lcd.setCursor(0, 1);
      lcd.print("> Hello World! <");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Status: ");
      lcd.setCursor(0, 1);
      lcd.print(messages[currentMessage]);
      
      currentMessage = (currentMessage + 1) % 5;
    }
    
    showCustom = !showCustom;
  }
}