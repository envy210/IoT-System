#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Adafruit_SHT31.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

bool shouldSendData = false;

// ----- OLED Display Settings -----
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define FAN_PIN 4

// Wi-Fi Credentials
const char* ssid = "Flip 6 Nidhin";       // Wi-Fi name
const char* password = "ucjb8873";  // Wi-Fi password

// MQTT Server (ThingsBoard)
const char* mqtt_server = "34.147.152.222";  // 
const char* access_token = "fok52cz37t0xfsjs1lpw";   // access token

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Function to connect to Wi-Fi
void setup_wifi() {
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
      attempts++;
      if (attempts > 20) {  // Restart after 20 attempts
          Serial.println("\nWi-Fi Connection Failed! Restarting...");
          ESP.restart();  
      }
  }

  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function to connect to MQTT broker (ThingsBoard)
void reconnect() {
    while (!client.connected()) {
        Serial.print("Connecting to ThingsBoard...");
        if (client.connect("ESP32 Sensor", access_token, "")) {
            client.subscribe("v1/devices/me/rpc/request/+");
            Serial.println("Connected!");
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" Trying again in 5 seconds...");
            delay(5000);
        }
    }
}

void setup() {
  Serial.begin(115200);

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    while(1); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  // Show boot message
  display.setCursor(0,0);
  display.println("Booting...");
  display.display();



  Wire.begin();
  
  if (!sht31.begin(0x45)) {  // SHT35 default I2C address is 0x45
    display.println("SHT35 not found!");
    display.display();
    //Serial.println("Couldn't find SHT35 sensor! Check wiring.");
    while (1);
  }
  //Serial.println("SHT35 Sensor Found!");
  display.println("SHT35 found!");
  display.display();
  delay(1000);

  // Connect to WiFi
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Connecting to WiFi...");
  display.display();
  setup_wifi();

  // WiFi connected
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi Connected!");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
  delay(1000);

  client.setCallback(callback);
  client.setServer(mqtt_server, 1883);
  reconnect();

  // Final boot message
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Device Ready");
  display.println("Waiting for command...");
  display.display();
}

void loop() {
  if (!client.connected()) {
      reconnect();
  }

  client.loop();

  // Read sensor data
  if (shouldSendData) {
    float temperature = sht31.readTemperature();
    float humidity = sht31.readHumidity();

    // Ensure valid data before sending
    if (!isnan(temperature) && !isnan(humidity)) {
        Serial.print("Temperature: "); Serial.print(temperature); Serial.print("°C | ");
        Serial.print("Humidity: "); Serial.print(humidity); Serial.println("%");

        // Create JSON payload
        String payload = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
        Serial.print("Sending data: ");
        Serial.println(payload);

        // Update OLED display with latest values
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Sending Data...");
        display.print("Temp: ");
        display.print(temperature);
        display.println(" C");
        display.print("Humidity: ");
        display.print(humidity);
        display.println(" %");
        display.display();

        // Publish data to ThingsBoard
        client.publish("v1/devices/me/telemetry", payload.c_str());
    } else {
        Serial.println("Failed to read from SHT35!");
    }
    delay(5000); // Send data every 5 seconds
  }

  
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("RPC callback triggered: ");
  Serial.print("Topic: ");
  Serial.println(topic);

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Payload: ");
  Serial.println(message);

  if (String(topic).startsWith("v1/devices/me/rpc/request/")) {
    // Parse JSON payload
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    String method = doc["method"];
    if (method == "start") {
      bool run = doc["params"]["run"];
      if (run) {
        shouldSendData = true;
        Serial.println("Start command received. Beginning data transmission... + Fan On");

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Start received");
        display.println("Recording data...");
        display.display();
      } else {
        shouldSendData = false;
        digitalWrite(FAN_PIN, LOW);
        Serial.println("Stop command received. Stopping data transmission. + Fan Off");
        client.publish("v1/devices/me/telemetry", "{\"fanState\":\"off\"}");

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Stop command received");
        display.println("Waiting for command...");
        display.println("Fan OFF");
        display.display();
      }
    }else if (method == "fan") {
      bool fanOn = doc["params"]["on"];

      if (fanOn) {
        digitalWrite(FAN_PIN, HIGH);
        Serial.println("FAN command received: ON");

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("FAN ON");
        display.display();

        // Publish fan telemetry
        client.publish("v1/devices/me/telemetry", "{\"fanState\":\"on\"}");
      } else {
        digitalWrite(FAN_PIN, LOW);
        Serial.println("FAN command received: OFF");

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("FAN OFF");
        display.display();

        // Publish fan telemetry
        client.publish("v1/devices/me/telemetry", "{\"fanState\":\"off\"}");
      }
    }


    // Extract request ID from topic (after last '/')
    String topicStr = String(topic);
    int lastSlash = topicStr.lastIndexOf('/');
    String requestId = topicStr.substring(lastSlash + 1);

    // Build response topic and send reply
    String responseTopic = "v1/devices/me/rpc/response/" + requestId;
    String responsePayload = "{\"status\":\"received\"}";
    client.publish(responseTopic.c_str(), responsePayload.c_str());
    Serial.println("Sent RPC response");
  }
}



