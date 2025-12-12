#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/////////////////// SETTINGS /////////////////////////////

// Wi-Fi
const char* ssid = "samgtu_emp";
const char* password = "SNye6Gsyc6";

// MQTT
const char* mqtt_server = "m1.wqtt.ru";
const int mqtt_port = 17192;
const char* mqtt_user = "u_7QOW7Z";
const char* mqtt_password = "6rcppizj";

// Пины для реле (светофора)
const int RELAY1 = 5;   // D1 = GPIO5 - Красный
const int RELAY2 = 4;   // D2 = GPIO4 - Желтый  
const int RELAY3 = 0;   // D3 = GPIO0 - Зеленый

// Топики
const String all_topic = "/topic/all";          // Для включения всех реле
const String light_sensor_topic = "/topic/light_level"; // Для отправки освещенности

/////////////////////////////////////////////////////////

WiFiClient espClient;
PubSubClient client(espClient);

bool relay1_on = false;
bool relay2_on = false;
bool relay3_on = false;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void updateRelays() {
  // Управляем всеми реле
  digitalWrite(RELAY1, relay1_on ? HIGH : LOW);
  digitalWrite(RELAY2, relay2_on ? HIGH : LOW);
  // D3 требует инвертированной логики!
  digitalWrite(RELAY3, relay3_on ? LOW : HIGH);
  
  Serial.print("Relay states - D1: ");
  Serial.print(relay1_on ? "ON" : "OFF");
  Serial.print(" | D2: ");
  Serial.print(relay2_on ? "ON" : "OFF");
  Serial.print(" | D3: ");
  Serial.print(relay3_on ? "ON" : "OFF");
  Serial.println();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String data_pay = "";
  for (int i = 0; i < length; i++) {
    data_pay += (char)payload[i];
  }

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(data_pay);
  
  String topicStr = String(topic);
  
  // Обрабатываем команду для всех реле
  if(topicStr == all_topic) {
    if(data_pay == "ON" || data_pay == "1" || data_pay == "on") {
      relay1_on = true;
      relay2_on = true;
      relay3_on = true;
      Serial.println("ALL Relays ON");
    } else if(data_pay == "OFF" || data_pay == "0" || data_pay == "off") {
      relay1_on = false;
      relay2_on = false;
      relay3_on = false;
      Serial.println("ALL Relays OFF");
    }
    updateRelays();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266-" + WiFi.macAddress();

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      
      // Подписываемся только на топик для всех реле
      client.subscribe(all_topic.c_str());
      
      Serial.println("Subscribed to: " + all_topic);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  Serial.println("\n===== ESP8266 AUTOMATIC LIGHT CONTROL =====");
  Serial.println("Pins: D1(GPIO5), D2(GPIO4), D3(GPIO0)");
  
  // Настройка пинов
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  
  // Изначально все реле выключены
  digitalWrite(RELAY1, LOW);   // D1 выкл
  digitalWrite(RELAY2, LOW);   // D2 выкл
  digitalWrite(RELAY3, HIGH);  // D3 выкл (HIGH = выключено для D3)
  
  Serial.println("All relays initialized to OFF");
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Отправляем данные об освещенности каждую секунду
  static unsigned long lastPublish = 0;
  if (millis() - lastPublish >= 1000) {
    lastPublish = millis();
    
    // Читаем значение с фоторезистора (A0)
    int lightLevel = analogRead(A0);
    
    // Создаем JSON с данными
    String lightData = "{\"value\":" + String(lightLevel) + "}";
    
    // Публикуем в MQTT
    client.publish(light_sensor_topic.c_str(), lightData.c_str());
    
    Serial.println("Light level: " + String(lightLevel));
  }
}
