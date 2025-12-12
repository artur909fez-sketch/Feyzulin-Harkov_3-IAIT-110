import paho.mqtt.client as mqtt
import json
import time

# ========== НАСТРОЙКИ ==========
BROKER = 'm1.wqtt.ru'
PORT = 17192
USER = 'u_7QOW7Z'
PASSWORD = '6rcppizj'

# Топики
TOPIC_LIGHT = '/topic/light_level'  # Читаем освещенность
TOPIC_ALL = '/topic/all'            # Управляем всеми реле

# Порог
THRESHOLD = 900

# ========== MQTT КОЛБЭКИ ==========
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Подключено к MQTT")
        client.subscribe(TOPIC_LIGHT)
    else:
        print(f"Ошибка: {rc}")

def on_message(client, userdata, msg):
    try:
        # Парсим JSON
        data = json.loads(msg.payload.decode())
        light_value = data.get('value', 0)
        
        print(f"Освещенность: {light_value}")
        
        # Включаем/выключаем все реле
        if light_value < THRESHOLD:
            client.publish(TOPIC_ALL, "ON")
            print(f"ВКЛ (свет: {light_value} < {THRESHOLD})")
        else:
            client.publish(TOPIC_ALL, "OFF")
            print(f"ВЫКЛ (свет: {light_value} >= {THRESHOLD})")
            
    except Exception as e:
        print(f"Ошибка: {e}")

# ========== ЗАПУСК ==========
client = mqtt.Client()
client.username_pw_set(USER, PASSWORD)
client.on_connect = on_connect
client.on_message = on_message

print(f"Подключение к {BROKER}...")
client.connect(BROKER, PORT, 60)

# Бесконечный цикл
client.loop_forever()
