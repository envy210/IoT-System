// This code allows the Arduino Uno Rev 4 to connect to WiFi and send MQTT commands to a ThingsBoard server.
// It acts as a command sender to control a remote Arduino Nano ESP32 IoT device (e.g., to start/stop data collection or fan control).

#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "Flip 6 Nidhin";
const char* password = "ucjb8873";

// ThingsBoard server configuration
const char* mqtt_server = "34.147.152.222";            // IP address of the ThingsBoard server
const char* access_token = "rsZ5jgqVVT9b8Q5nbEyU";     // Access token for this device in ThingsBoard
const char* telemetry_topic = "v1/devices/me/telemetry"; // Topic for sending telemetry or command signals

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Connect to WiFi network
void setup_wifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883); // Connect to MQTT server on port 1883 (unencrypted)
}

void loop() {
  if (!client.connected()) {
    client.connect("UnoCommandSender", access_token, "");
  }
  client.loop();

  // Check if a serial command was entered via Serial Monitor
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n'); // Read entire input until newline (user presses Enter)
    input.trim(); // Remove any trailing whitespace or newline characters

    // Accept only single-character commands for safety
    if (input.length() == 1) {
      char cmd = input.charAt(0);

      if (cmd == 's') {
        // Start data recording on ESP32
        String payload = "{\"startEsp\": true}";
        client.publish(telemetry_topic, payload.c_str());
        Serial.println("Start command sent");

      } else if (cmd == 'x') {
        // Stop data recording on ESP32
        String payload = "{\"stopEsp\": true}";
        client.publish(telemetry_topic, payload.c_str());
        Serial.println("Stop command sent");

      } else if (cmd == 'f') {
        // Turn fan ON via ESP32
        String payload = "{\"fanCmd\": true}";
        client.publish(telemetry_topic, payload.c_str());
        Serial.println("Fan ON command sent");

      } else if (cmd == 'o') {
        // Turn fan OFF via ESP32
        String payload = "{\"fanCmd\": false}";
        client.publish(telemetry_topic, payload.c_str());
        Serial.println("Fan OFF command sent");

      } else {
        // Invalid single-character command
        Serial.println("Unknown command. Valid commands: s, x, f, o");
      }

    } else {
      // Too many characters entered
      Serial.println("Please enter only one command character.");
      Serial.println("Valid commands:");
      Serial.println("  's' = Start Recording");
      Serial.println("  'x' = Stop Recording");
      Serial.println("  'f' = Fan ON");
      Serial.println("  'o' = Fan OFF");
    }
  }
}
