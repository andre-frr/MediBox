#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char *SSID = WIFI_SSID;
const char *PASS = WIFI_PASS;

const char *MQTT_H = MQTT_HOST;
const int MQTT_PNUM = 1883;

const char *MQTT_CLIENT_ID = "ESP32-MEDIBOX";
const char *MQTT_U = MQTT_USER;
const char *MQTT_P = MQTT_PASS;

const char *TOPIC_STATUS = "MediBox/Status";
const char *TOPIC_HEARTBEAT = "MediBox/Heartbeat";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long currentMillis = 0;
unsigned long lastWifiCheck = 0;
unsigned long lastMqttCheck = 0;
unsigned long lastHeartbeat = 0;

const unsigned long WIFI_CHECK_MS = 1000;
const unsigned long MQTT_CHECK_MS = 1000;
const unsigned long HEARTBEAT_MS = 5000;

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASS);
    mqtt.setServer(MQTT_H, MQTT_PNUM);
}

void WiFiCheck() {
    if (currentMillis - lastWifiCheck > WIFI_CHECK_MS) {
        lastWifiCheck = currentMillis;
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.disconnect();
            WiFi.begin(SSID, PASS);
        }
    }
}

void MqttCheck() {
    if (currentMillis - lastMqttCheck >= MQTT_CHECK_MS) {
        lastMqttCheck = currentMillis;
        if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) {
            if (mqtt.connect(MQTT_CLIENT_ID, MQTT_U, MQTT_P, TOPIC_STATUS, 0, true, "offline")) {
                mqtt.publish(TOPIC_STATUS, "online", true);
            }
        }
    }

    if (mqtt.connected()) {
        mqtt.loop();
    }
}

void HeartbeatCheck() {
    if (currentMillis - lastHeartbeat >= HEARTBEAT_MS) {
        lastHeartbeat = currentMillis;
        if (mqtt.connected()) {
            mqtt.publish(TOPIC_HEARTBEAT, "alive", false);
        }
    }
}

void loop() {
    currentMillis = millis();
    WiFiCheck();
    MqttCheck();
    HeartbeatCheck();
}
