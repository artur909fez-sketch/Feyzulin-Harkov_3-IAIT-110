#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include <Adafruit_FT6206.h>

#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_FT6206 ts = Adafruit_FT6206();

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(2);
  
  if (!ts.begin()) {
    Serial.println("Сенсор не найден!");
    while (1);
  }
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("TFT + Touch Ready");
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    tft.fillCircle(p.x, p.y, 5, ILI9341_RED);
  }
}