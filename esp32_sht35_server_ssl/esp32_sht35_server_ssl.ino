#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
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

// Certificate: ISRG Root X1
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

// Wi-Fi Credentials
const char* ssid = "Flip 6 Nidhin";       // Wi-Fi name
const char* password = "ucjb8873";  // Wi-Fi password

// MQTT Server (ThingsBoard)
const char* mqtt_server = "iotproject.uk";
const int mqtt_port = 8883;
const char* access_token = "fok52cz37t0xfsjs1lpw";   // access token

WiFiClientSecure espClient;
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

  espClient.setInsecure(); //Ignores validating the root certificate

  client.setCallback(callback);
  client.setServer(mqtt_server, mqtt_port);
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



