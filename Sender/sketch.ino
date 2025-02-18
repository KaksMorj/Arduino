#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <ArduinoJson.h>
#include <SimpleTimer.h>
#include "time.h"

SimpleTimer timer_sensor(5000); // Timer for polling sensor every 5 seconds
SimpleTimer timer_mqtt(10000);  // Timer for transmitting data every 10 seconds

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "test.mosquitto.org";
const char* mqtt_topic = "topic439543921345236"; // MQTT topic to subscribe and publish

const int DHT_PIN = 33; // GPIO pin where DHT22 is connected

DHTesp dht; // Use DHTesp class for DHT22 sensor

WiFiClient espClient;
PubSubClient client(espClient);

const long gmtOffset_sec = 3 * 3600; // GMT+3 
const int daylightOffset_sec = 0;

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

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);

  dht.setup(DHT_PIN, DHTesp::DHT22); // Initialize DHT22 sensor with DHTesp library

  // Initialize and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Poll sensor every 5 seconds
  if (timer_sensor.isReady()) {
    float temperature = dht.getTemperature();

    // Create JSON object
    StaticJsonDocument<200> doc;
    doc["temperature"] = temperature;
    doc["datetime"] = printLocalTime(); // Add datetime to JSON object

    // Serialize JSON object to a string
    String jsonString;
    serializeJson(doc, jsonString);
    
    Serial.println("Message to be published:");
    Serial.println(jsonString);

    // Publish message
    client.publish(mqtt_topic, jsonString.c_str());

    timer_sensor.reset();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("SensorNode")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
    delay(5000);
  }
}

String printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "Failed to obtain time";
  }

  char output[30];
  strftime(output, 30, "%Y-%m-%d %H:%M:%S", &timeinfo); // Format datetime as "YYYY-MM-DD HH:MM:SS"

  return output;
}
