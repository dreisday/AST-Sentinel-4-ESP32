#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi & MQTT Configuration
const char* ssid = "InsertYourSSID";
const char* password = "InsertYourWiFiPassword";
const char* mqtt_server = "InsertYourMQTTServer"; // e.g. "mqtt.example.com"
const int mqtt_port = 1883; // Default MQTT port change if needed
const char* mqtt_user = "YourMQTTUsername"; // Optional, if your broker requires authentication
const char* mqtt_pass = "YourMQTTPassword"; // Optional, if your broker requires authentication

// MQTT Topics
const char* status_topic = "intercom/status";
const char* command_topic = "intercom/command";

// GPIO Pins
const int RING_PIN = 13;        // Input from ring detect relay
const int ANSWER_RELAY = 12;    // NC relay to unlatch handset
const int UNLOCK_RELAY = 14;    // Relay to short D to GND

enum State { IDLE, RINGING, ANSWERED, UNLOCKED };
State currentState = IDLE;

unsigned long unlockTime = 0;
WiFiClient intercom_esp;
PubSubClient client(intercom_esp);
client.setBufferSize(512);

// Helper: Publish current status to MQTT
void publish_status(const char* status) {
  client.publish(status_topic, status, true);  // Retained
  Serial.printf("Published status: %s\n", status);
}

// MQTT Discovery with Device Block
void publish_discovery() {
  // Allocate buffer (adjust size if needed)
  StaticJsonDocument<512> doc;

  // Common device info
  JsonObject device = doc.createNestedObject("device");
  device["identifiers"][0] = "esp32-intercom";
  device["name"] = "ESP32 Intercom"; // Name of the device - this can be changed and is displayed in Home Assistant
  device["manufacturer"] = "DIY";
  device["model"] = "ESP32 MQTT Intercom";

  // --- Sensor discovery ---
  doc["name"] = "Intercom Status";
  doc["unique_id"] = "intercom_status_sensor";
  doc["state_topic"] = status_topic;
  doc["icon"] = "mdi:phone";

  char sensorBuffer[512];
  serializeJson(doc, sensorBuffer);
  client.publish("homeassistant/sensor/intercom_status/config", sensorBuffer, true);

  // --- Button discovery ---
  doc.clear();
  doc["name"] = "Unlock Intercom";
  doc["unique_id"] = "intercom_unlock_button";
  doc["command_topic"] = command_topic;
  doc["payload_press"] = "unlock";
  device = doc.createNestedObject("device");
  device["identifiers"][0] = "esp32-intercom";
  device["name"] = "ESP32 Intercom";
  device["manufacturer"] = "DIY";
  device["model"] = "ESP32 MQTT Intercom";

  char buttonBuffer[512];
  serializeJson(doc, buttonBuffer);
  client.publish("homeassistant/button/intercom_unlock/config", buttonBuffer, true);

  Serial.println("Discovery messages published.");
}


// WiFi setup
void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT reconnect logic
void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32Intercom", mqtt_user, mqtt_pass)) {
      Serial.println("MQTT connected.");

      // Subscribe to command topic
      client.subscribe(command_topic);
      Serial.println("Subscribed to command topic.");

      // Publish discovery messages
      Serial.println("Publishing MQTT discovery...");
      publish_discovery();

      // Publish idle status
      publish_status("idle");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5s");
      delay(5000);
    }
  }
}

// Handle incoming MQTT messages
void mqtt_callback(char* topic, byte* message, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)message[i];

  Serial.printf("MQTT message on %s: %s\n", topic, msg.c_str());

  // Check command and whether ring is active
  bool ringActive = digitalRead(RING_PIN) == LOW;

  if (String(topic) == command_topic && msg == "unlock" && ringActive) {
    Serial.println("Triggering relays for unlock sequence...");
    digitalWrite(ANSWER_RELAY, HIGH);  // Turn on answer relay
    delay(1500);                       // Wait 1 second
    digitalWrite(UNLOCK_RELAY, HIGH);  // Turn on unlock relay
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(RING_PIN, INPUT_PULLUP);
  pinMode(ANSWER_RELAY, OUTPUT);
  pinMode(UNLOCK_RELAY, OUTPUT);
  digitalWrite(ANSWER_RELAY, LOW);
  digitalWrite(UNLOCK_RELAY, LOW);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);
}

String lastPublishedStatus = "";

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Read pin states
  bool ringActive = digitalRead(RING_PIN) == LOW;
  bool answerActive = digitalRead(ANSWER_RELAY) == HIGH;
  bool unlockActive = digitalRead(UNLOCK_RELAY) == HIGH;

  // Determine status
  String newStatus = "idle";
  if (unlockActive) {
    newStatus = "unlocked";
  } else if (answerActive) {
    newStatus = "answered";
  } else if (ringActive) {
    newStatus = "ringing";
  }

  // Only publish if the status changed
  if (newStatus != lastPublishedStatus) {
    publish_status(newStatus.c_str());
    lastPublishedStatus = newStatus;
  }

  // Handle unlock timeout
  static unsigned long unlockTime = 0;
  if (unlockActive && unlockTime == 0) {
    unlockTime = millis();
  }
  if (unlockTime > 0 && millis() - unlockTime > 10000) {
    digitalWrite(ANSWER_RELAY, LOW);
    digitalWrite(UNLOCK_RELAY, LOW);
    unlockTime = 0;
  }
}
