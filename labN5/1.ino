const int photoResistorPin = A0;

void setup() {
  Serial.begin(9600);
  Serial.println("=== Luxmeter ADC Readings ===");
  Serial.println("N\tADC\tVoltage");
}

void loop() {
  static int measurementCount = 0;
  int adcValue = analogRead(photoResistorPin);
  float voltage = adcValue * (5.0 / 1023.0);
  
  measurementCount++;
  Serial.print(measurementCount);
  Serial.print("\t");
  Serial.print(adcValue);
  Serial.print("\t");
  Serial.print(voltage, 2);
  Serial.println(" V");
  
  delay(2000); // Замер каждые 2 секунды
}