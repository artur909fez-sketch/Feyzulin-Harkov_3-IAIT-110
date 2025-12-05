#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <Keypad.h>
#include <EEPROM.h>

// ==================== КОНФИГУРАЦИЯ ====================
// Очереди FreeRTOS
QueueHandle_t xLuxQueue;
QueueHandle_t xLogQueue;

// Дисплей (встроен в плату) - проверьте адрес I2C!
// Возможные адреса: 0x27, 0x3F, 0x20
LiquidCrystal_I2C lcd(0x27, 16, 2); // Попробуйте 0x3F если не работает

// RTC DS1302 (встроен в плату)
ThreeWire myWire(4, 5, 2); // DAT, CLK, RST
RtcDS1302<ThreeWire> Rtc(myWire);

// Клавиатура 4x4 (встроена в плату)
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {6, 7, 8, 9};
byte colPins[COLS] = {10, 11, 12, 13};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Фотоэлемент (внешний, подключается к A0)
const int photoPin = A0;

// ==================== СТРУКТУРЫ ДАННЫХ ====================
struct CalibrationPoint {
    int adc;
    float lux;
};

struct LogEntry {
    float lux;
    uint32_t timestamp;
    char label[21];
};

// Калибровочные данные (ЗАМЕНИТЕ НА ВАШИ ДАННЫЕ!)
const CalibrationPoint CalibrationData[] = {
    {200, 10.0}, {400, 150.0}, {500, 300.0}, {600, 500.0},
    {700, 750.0}, {800, 1000.0}, {850, 1200.0}, {900, 1300.0},
    {950, 1400.0}, {1000, 1500.0}
};
const int CalibrationSize = sizeof(CalibrationData) / sizeof(CalibrationPoint);

// ==================== ПРОТОТИПЫ ФУНКЦИЙ ====================
void vTaskMeasureLuminosity(void *pvParameters);
void vTaskDisplay(void *pvParameters);
void vTaskRTC(void *pvParameters);
void vTaskKeyboard(void *pvParameters);
void vTaskLogger(void *pvParameters);
float calculateLuxFromADC(int D, const CalibrationPoint* data);
void initializeRTC();
void saveToEEPROM(const LogEntry& entry);
void printCalibrationTable();
void showMainDisplay(float lux, int adc);

// ==================== SETUP ====================
void setup() {
    Serial.begin(9600);
    
    // Инициализация дисплея
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Luxmeter v1.0");
    lcd.setCursor(0, 1);
    lcd.print("Initializing...");
    
    // Инициализация RTC
    initializeRTC();
    
    delay(2000);
    lcd.clear();
    
    // Создание очередей FreeRTOS
    xLuxQueue = xQueueCreate(10, sizeof(float));
    xLogQueue = xQueueCreate(5, sizeof(LogEntry));
    
    if (xLuxQueue != NULL && xLogQueue != NULL) {
        // Создание задач FreeRTOS
        xTaskCreate(vTaskMeasureLuminosity, "Measure", 256, NULL, 4, NULL);
        xTaskCreate(vTaskDisplay, "Display", 256, NULL, 3, NULL);
        xTaskCreate(vTaskRTC, "RTC", 256, NULL, 2, NULL);
        xTaskCreate(vTaskKeyboard, "Keyboard", 256, NULL, 3, NULL);
        xTaskCreate(vTaskLogger, "Logger", 256, NULL, 1, NULL);
        
        // Запуск планировщика FreeRTOS
        vTaskStartScheduler();
    } else {
        lcd.clear();
        lcd.print("Error: Queues!");
        while(1); // Остановка при ошибке
    }
}

void loop() {
    // Пусто - все управляется FreeRTOS
}

// ==================== ЗАДАЧА ИЗМЕРЕНИЯ ОСВЕЩЕННОСТИ ====================
void vTaskMeasureLuminosity(void *pvParameters) {
    float luxValue = 0.0;
    
    for (;;) {
        // Чтение значения с фотоэлемента
        int adcValue = analogRead(photoPin);
        
        // Преобразование АЦП в люксы через калибровочную функцию
        luxValue = calculateLuxFromADC(adcValue, CalibrationData);
        
        // Отправка в очередь для дисплея
        if (xQueueSend(xLuxQueue, &luxValue, portMAX_DELAY) != pdPASS) {
            Serial.println("ERROR: Lux queue full!");
        }
        
        // Задержка 200 мс (5 раз в секунду)
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// ==================== ФУНКЦИЯ ИНТЕРПОЛЯЦИИ ====================
float calculateLuxFromADC(int D, const CalibrationPoint* data) {
    // Проверка граничных значений
    if (D <= data[0].adc) return data[0].lux;
    if (D >= data[CalibrationSize - 1].adc) return data[CalibrationSize - 1].lux;
    
    // Линейная интерполяция между точками калибровки
    for (int i = 0; i < CalibrationSize - 1; ++i) {
        if (D >= data[i].adc && D <= data[i+1].adc) {
            float D1 = data[i].adc;
            float E1 = data[i].lux;
            float D2 = data[i+1].adc;
            float E2 = data[i+1].lux;
            float E = E1 + (E2 - E1) * ((float)(D - D1) / (D2 - D1));
            return E;
        }
    }
    
    return 0.0;
}

// ==================== ЗАДАЧА ДИСПЛЕЯ ====================
void vTaskDisplay(void *pvParameters) {
    float receivedLux;
    unsigned long lastDisplayTime = 0;
    uint8_t displayMode = 0;
    
    for (;;) {
        // Получение данных из очереди (блокирующее ожидание)
        if (xQueueReceive(xLuxQueue, &receivedLux, portMAX_DELAY) == pdPASS) {
            int adcValue = analogRead(photoPin);
            
            // Обновление дисплея каждые 500 мс
            if (millis() - lastDisplayTime > 500) {
                lcd.clear();
                
                switch (displayMode % 3) { // 3 режима: освещенность, ADC, время
                    case 0: // Режим освещенности
                        showMainDisplay(receivedLux, adcValue);
                        break;
                        
                    case 1: // Режим ADC
                        lcd.setCursor(0, 0);
                        lcd.print("ADC Value:");
                        lcd.setCursor(0, 1);
                        lcd.print(adcValue);
                        lcd.print("/1023");
                        break;
                        
                    case 2: // Режим времени
                        lcd.setCursor(0, 0);
                        lcd.print("Time:");
                        RtcDateTime now = Rtc.GetDateTime();
                        lcd.setCursor(0, 1);
                        lcd.print(now.Hour()); lcd.print(":");
                        if (now.Minute() < 10) lcd.print("0");
                        lcd.print(now.Minute()); lcd.print(":");
                        if (now.Second() < 10) lcd.print("0");
                        lcd.print(now.Second());
                        break;
                }
                
                lastDisplayTime = millis();
                displayMode++;
            }
            
            // Вывод в Serial для мониторинга (каждые 2 секунды)
            static unsigned long lastSerialPrint = 0;
            if (millis() - lastSerialPrint > 2000) {
                Serial.print("Lux: ");
                Serial.print(receivedLux, 1);
                Serial.print(" lx | ADC: ");
                Serial.println(adcValue);
                lastSerialPrint = millis();
            }
        }
    }
}

// Функция отображения основного экрана
void showMainDisplay(float lux, int adc) {
    lcd.setCursor(0, 0);
    lcd.print("Illum: ");
    lcd.print(lux, 1);
    lcd.print(" lx");
    
    lcd.setCursor(0, 1);
    lcd.print("ADC: ");
    lcd.print(adc);
    lcd.print("    ");
}

// ==================== ЗАДАЧА RTC ====================
void vTaskRTC(void *pvParameters) {
    for (;;) {
        // Чтение и вывод времени RTC
        RtcDateTime now = Rtc.GetDateTime();
        
        Serial.print("RTC Time: ");
        Serial.print(now.Year()); Serial.print("-");
        Serial.print(now.Month()); Serial.print("-");
        Serial.print(now.Day()); Serial.print(" ");
        Serial.print(now.Hour()); Serial.print(":");
        if (now.Minute() < 10) Serial.print("0");
        Serial.print(now.Minute()); Serial.print(":");
        if (now.Second() < 10) Serial.print("0");
        Serial.print(now.Second());
        Serial.println();
        
        // Задержка 1 секунда
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ==================== ЗАДАЧА КЛАВИАТУРЫ ====================
void vTaskKeyboard(void *pvParameters) {
    char lastKey = 0;
    unsigned long lastKeyTime = 0;
    
    for (;;) {
        char key = keypad.getKey();
        
        if (key && key != lastKey) {
            lastKey = key;
            lastKeyTime = millis();
            
            Serial.print("Key pressed: ");
            Serial.println(key);
            
            switch (key) {
                case 'A': // Установка времени (упрощенно)
                    lcd.clear();
                    lcd.print("Time set: TODO");
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    break;
                    
                case 'B': // Запись в лог
                    {
                        LogEntry logEntry;
                        logEntry.lux = calculateLuxFromADC(analogRead(photoPin), CalibrationData);
                        logEntry.timestamp = Rtc.GetDateTime().TotalSeconds();
                        strncpy(logEntry.label, "Manual_Log", 20);
                        
                        if (xQueueSend(xLogQueue, &logEntry, portMAX_DELAY) == pdPASS) {
                            lcd.clear();
                            lcd.print("Log Saved!");
                            vTaskDelay(pdMS_TO_TICKS(1000));
                        }
                    }
                    break;
                    
                case 'C': // Тест точности
                    printCalibrationTable();
                    lcd.clear();
                    lcd.print("Check Serial!");
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    break;
                    
                case 'D': // Дополнительная функция
                    lcd.clear();
                    lcd.print("FreeRTOS OK!");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    break;
            }
        }
        
        // Сброс защиты от повторного нажатия через 200 мс
        if (millis() - lastKeyTime > 200) {
            lastKey = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // Проверка клавиатуры каждые 50 мс
    }
}

// ==================== ЗАДАЧА ЛОГГЕРА ====================
void vTaskLogger(void *pvParameters) {
    LogEntry logEntry;
    int logCount = 0;
    
    for (;;) {
        if (xQueueReceive(xLogQueue, &logEntry, portMAX_DELAY)) {
            // Сохранение в EEPROM
            saveToEEPROM(logEntry);
            logCount++;
            
            // Вывод информации о записи
            Serial.print("LOG #");
            Serial.print(logCount);
            Serial.print(": ");
            Serial.print(logEntry.lux, 1);
            Serial.print(" lx, ");
            Serial.print(logEntry.label);
            Serial.print(", Time: ");
            Serial.println(logEntry.timestamp);
        }
    }
}

// ==================== ФУНКЦИЯ СОХРАНЕНИЯ В EEPROM ====================
void saveToEEPROM(const LogEntry& entry) {
    static int currentAddress = 0;
    const int EEPROM_SIZE = 1024; // Для Arduino UNO
    
    // Проверка переполнения EEPROM
    if (currentAddress + sizeof(entry) >= EEPROM_SIZE) {
        currentAddress = 0; // Начать заново
        Serial.println("EEPROM: Reset to start");
    }
    
    // Сохранение в EEPROM
    EEPROM.put(currentAddress, entry);
    currentAddress += sizeof(entry);
    
    Serial.print("EEPROM: Saved at addr ");
    Serial.println(currentAddress);
}

// ==================== ИНИЦИАЛИЗАЦИЯ RTC ====================
void initializeRTC() {
    Serial.print("Initializing RTC... ");
    
    Rtc.Begin();
    
    // Проверяем, работает ли RTC
    if (!Rtc.IsDateTimeValid()) {
        Serial.println("RTC: Setting compile time");
        // Устанавливаем время компиляции
        RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
        Rtc.SetDateTime(compiled);
    }
    
    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC: Starting now");
        Rtc.SetIsRunning(true);
    }
    
    Serial.println("RTC: OK");
}

// ==================== ФУНКЦИЯ ТЕСТИРОВАНИЯ ТОЧНОСТИ ====================
void printCalibrationTable() {
    Serial.println("\n=== ТАБЛИЦА ТОЧНОСТИ ЛЮКСМЕТРА ===");
    Serial.println("№\tADC\tЭталон\tИзмерено\tПогрешность\tОтн.погр.%");
    Serial.println("-\t---\t------\t--------\t----------\t----------");
    
    float maxRelativeError = 0;
    float sumRelativeError = 0;
    
    for (int i = 0; i < CalibrationSize; i++) {
        int adc = CalibrationData[i].adc;
        float referenceLux = CalibrationData[i].lux;
        float measuredLux = calculateLuxFromADC(adc, CalibrationData);
        float error = measuredLux - referenceLux;
        float relativeError = (error / referenceLux) * 100.0;
        
        Serial.print(i + 1);
        Serial.print("\t");
        Serial.print(adc);
        Serial.print("\t");
        Serial.print(referenceLux, 1);
        Serial.print("\t");
        Serial.print(measuredLux, 1);
        Serial.print("\t\t");
        Serial.print(error, 1);
        Serial.print("\t\t");
        Serial.print(relativeError, 1);
        Serial.println("%");
        
        if (fabs(relativeError) > fabs(maxRelativeError)) {
            maxRelativeError = relativeError;
        }
        sumRelativeError += fabs(relativeError);
    }
    
    float avgError = sumRelativeError / CalibrationSize;
    
    Serial.println("----------------------------------------");
    Serial.print("Максимальная погрешность: ");
    Serial.print(maxRelativeError, 1);
    Serial.println("%");
    Serial.print("Средняя погрешность: ");
    Serial.print(avgError, 1);
    Serial.println("%");
    
    // Определение класса точности
    Serial.print("Класс точности прибора: ");
    if (fabs(maxRelativeError) <= 1.0) {
        Serial.println("0.5");
    } else if (fabs(maxRelativeError) <= 2.0) {
        Serial.println("1.0");
    } else if (fabs(maxRelativeError) <= 5.0) {
        Serial.println("2.5");
    } else {
        Serial.println("4.0");
    }
    Serial.println();
}