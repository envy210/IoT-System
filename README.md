# IoT System - MQTT Security Analysis Project

This repository contains the Arduino firmware code developed for the MEng thesis project titled: **"Network Traffic Analysis of Unencrypted and TLS-Encrypted MQTT in an IoT System"**.

The project implements a simple IoT system to compare network traffic characteristics when using standard unencrypted MQTT versus MQTT secured with TLS.

## System Overview

The system consists of three main components:

1.  **Command Initiator (Arduino Uno Rev 4 WiFi):** Reads simple commands from the Arduino IDE Serial Monitor and publishes corresponding messages as telemetry to the cloud platform via MQTT.
2.  **Cloud Platform (ThingsBoard on GCP):** An instance of ThingsBoard hosted on Google Cloud Platform acts as the MQTT broker, processes incoming data/commands via its Rule Engine, and sends RPC commands to the IoT Node. HAProxy is used as a reverse proxy to handle TLS termination for secure MQTT connections.
3.  **IoT Node (Arduino Nano ESP32):** Connects to Wi-Fi, subscribes to RPC commands from ThingsBoard, controls a local fan and OLED display, reads temperature/humidity data (SHT35 sensor), and publishes sensor/state data back to ThingsBoard via MQTT.

## Code Structure

The repository contains Arduino firmware (`.ino` files) for the Command Initiator and the IoT Node, organized into four folders:

* **`uno_start_esp`**: Firmware for the Arduino Uno Rev 4 WiFi using **unencrypted MQTT** (connects to port 1883).
* **`uno_start_esp_ssl`**: Firmware for the Arduino Uno Rev 4 WiFi using **TLS-encrypted MQTT** (connects to port 8883 via HAProxy). Requires appropriate root certificate flashed onto the Uno.
* **`esp32_sht35_server`**: Firmware for the Arduino Nano ESP32 IoT Node using **unencrypted MQTT** (connects to port 1883).
* **`esp32_sht35_server_ssl`**: Firmware for the Arduino Nano ESP32 IoT Node using **TLS-encrypted MQTT** (connects to port 8883 via HAProxy). Note: This code uses `client.setInsecure()` to bypass server certificate validation due to implementation challenges on the ESP32.

Each folder contains a `.ino` file with the corresponding name (e.g., `uno_start_esp/uno_start_esp.ino`).

## Hardware Setup (IoT Node)

A Fritzing diagram file (`IoT-System.fzz`) is included in this repository, illustrating the circuit connections for the IoT Node (Arduino Nano ESP32, SHT3x Sensor, Fan circuit with transistor, OLED Display).

*Note: The wiring logic shown in the diagram is accurate, but the specific component representations may differ slightly from the actual hardware used due to Fritzing library limitations (e.g., finding exact component matches). The actual implementation used a breadboard and was powered by a USB power bank (not explicitly shown in the Fritzing file).*

## Server Setup

To run this code successfully, a backend setup identical or similar to the one used in the project is required:

1.  A server (e.g., a Google Cloud VM) hosting the ThingsBoard platform.
2.  PostgreSQL database configured for ThingsBoard.
3.  ThingsBoard configured with two devices (one for the Uno, one for the ESP32) and their respective access tokens.
4.  A ThingsBoard Rule Chain configured to process telemetry from the Uno device and trigger RPC calls to the ESP32 device.
5.  For the `_ssl` versions of the code:
    * HAProxy installed and configured on the server to listen for MQTT TLS connections on port 8883, perform TLS termination (using a valid certificate, e.g., from Let's Encrypt), and forward decrypted traffic to the internal ThingsBoard MQTT broker on port 1883.
    * Appropriate firewall rules allowing traffic on ports 8883 (for TLS MQTT) and potentially 443 (for HTTPS access to ThingsBoard UI).

## Running the Code

1.  Choose the appropriate code version (unencrypted or `_ssl`) for both the Uno and ESP32 based on the desired test scenario.
2.  Update the following variables within the `.ino` files:
    * Wi-Fi SSID and Password.
    * `mqtt_server`: The domain name or IP address of your ThingsBoard/HAProxy server.
    * `access_token`: The correct ThingsBoard device access token for the Uno or ESP32.
3.  Ensure the necessary Arduino libraries are installed (e.g., `PubSubClient`, `WiFiS3` or ESP32 `WiFi`, `Adafruit_SHT31`, `Adafruit_GFX`, `Adafruit_SSD1306`, `ArduinoJson`).
4.  For the `uno_start_esp_ssl` code, ensure the required Root CA certificate (e.g., ISRG Root X1 for Let's Encrypt) is correctly flashed onto the Arduino Uno Rev 4 WiFi.
5.  Upload the code to the respective Arduino boards.
6.  Monitor output using the Arduino IDE Serial Monitor. Send single-character commands ('s', 'x', 'f', 'o') to the Uno's Serial Monitor to trigger actions.

## Important Notes

* **Server Availability:** The Google Cloud server instance used for the original project development will be **shut down after June 15th, 2025**, to avoid incurring further costs. The code will not function without a running backend (ThingsBoard, HAProxy).
* **ThingsBoard Access:** Login details for the original ThingsBoard instance can be provided upon specific request before the server shutdown date, for verification purposes only.
* **ESP32 TLS Security:** The `esp32_sht35_server_ssl` code uses `client.setInsecure()` and **does not perform server certificate validation**. This was a workaround for implementation difficulties but represents a security risk (potential Man-in-the-Middle attack) and should not be used in production environments without addressing certificate validation.

