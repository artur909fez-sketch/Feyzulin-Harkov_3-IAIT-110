#include <LedControl.h>

LedControl lc = LedControl(11, 13, 10, 1);

byte frames[][8] = {
  {0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C}, 
  {0x00, 0x18, 0x3C, 0x7E, 0x7E, 0x3C, 0x18, 0x00}, 
  {0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF}, 
  {0x00, 0x24, 0x18, 0x18, 0x18, 0x18, 0x24, 0x00} 
};

int currentFrame = 0;
unsigned long lastChange = 0;

void setup() {
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
}

void loop() {
  if (millis() - lastChange > 500) {
    lastChange = millis();
    showFrame(currentFrame);
    currentFrame = (currentFrame + 1) % 4;
  }
}

void showFrame(int frame) {
  for (int row = 0; row < 8; row++) {
    lc.setRow(0, row, frames[frame][row]);
  }
}