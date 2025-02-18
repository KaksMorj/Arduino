#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <ArduinoJson.h>
#include <SimpleTimer.h>
#include "time.h"


SimpleTimer mqtt_get(10000);

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "test.mosquitto.org";
const char* mqtt_topic = "topic439543921345236"; // MQTT topic to subscribe and publish

const int relayPin = 33; // GPIO pin where the relay is connected

WiFiClient espClient;
PubSubClient client(espClient);

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

  Serial.println("\n\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic1: ");
  Serial.println(topic);

  // Convert payload to string
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message: ");
  Serial.println(message);

  // Parse JSON message
  DynamicJsonDocument doc(200);
  deserializeJson(doc, message);

  // Extract temperature from JSON
  float temperature = doc["temperature"];
  
  // Check temperature and control relay
  if (temperature > 27.0) {
    digitalWrite(relayPin, HIGH); // Turn on relay
    Serial.println("Temperature > 27°C - Relay ON");
  } else {
    digitalWrite(relayPin, LOW); // Turn off relay
    Serial.println("Temperature <= 27°C - Relay OFF");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ThermostatNode")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
    delay(5000);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Ensure relay is off initially

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (mqtt_get.isReady()){
  client.loop();
  }
}
