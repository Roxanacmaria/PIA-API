# ESP32 TheMealDB – Wi-Fi & Bluetooth Recipe Client

## Overview

This project implements an **ESP32-based communication system** that combines **Bluetooth Classic, Wi-Fi, HTTP requests, and JSON processing**.

The ESP32 receives commands through Bluetooth, scans and connects to Wi-Fi networks, communicates with a remote recipe API through HTTP requests, processes the received JSON data, and sends the requested information back through Bluetooth.

The project demonstrates how an ESP32 can act as an intermediary between a client application and a web API.

---

## Features

The system supports the following operations:

* Scan for available Wi-Fi networks
* Connect the ESP32 to a selected Wi-Fi network
* Receive commands through Bluetooth Classic
* Send HTTP GET requests to a remote REST API
* Retrieve a list of recipes
* Retrieve detailed information about a specific recipe
* Parse JSON responses using ArduinoJson
* Send formatted JSON responses back through Bluetooth
* Handle invalid JSON commands
* Stop Wi-Fi connection attempts after a 10-second timeout

---

## Technologies Used

### Hardware

* ESP32 development board
* Wi-Fi connection
* Bluetooth Classic connection

### Software & Libraries

The project is developed using the Arduino framework.

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "BluetoothSerial.h"
```

The libraries are used as follows:

* **Arduino.h** – provides the basic Arduino functionality, including `setup()`, `loop()`, and `delay()`.
* **WiFi.h** – manages Wi-Fi scanning and network connections.
* **ArduinoJson.h** – parses incoming JSON commands and creates JSON responses.
* **HTTPClient.h** – performs HTTP requests to the remote API.
* **BluetoothSerial.h** – enables Bluetooth Classic serial communication on the ESP32.

---

## System Architecture

The communication flow of the project is:

```text
Client Application
       │
       │ Bluetooth Classic
       ▼
     ESP32
       │
       ├── Wi-Fi Network Scanning
       │
       └── Wi-Fi Connection
              │
              │ HTTP GET
              ▼
         Recipe REST API
              │
              │ JSON Response
              ▼
            ESP32
              │
              │ Bluetooth Classic
              ▼
       Client Application
```

The ESP32 therefore acts as a bridge between the client application and the remote API.

---

## Communication Protocol

Commands are sent to the ESP32 as JSON objects through Bluetooth Classic.

The ESP32 reads the received message and passes it to:

```cpp
handleCommand(input);
```

The `handleCommand()` function parses the JSON and determines which operation must be executed based on the `action` field.

Four main actions are supported:

```text
getNetworks
connect
getData
getDetails
```

---

## 1. Scan Wi-Fi Networks – `getNetworks`

This command searches for Wi-Fi networks available around the ESP32.

Example command:

```json
{
  "action": "getNetworks"
}
```

The ESP32 uses:

```cpp
WiFi.scanNetworks();
```

For every detected network, it creates a JSON response containing:

* SSID
* signal strength (RSSI)
* encryption type
* team ID

Example response:

```json
{
  "ssid": "Network_Name",
  "strength": -55,
  "encryption": 3,
  "teamId": "A19"
}
```

Each detected network is sent individually to the client through Bluetooth.

---

## 2. Connect to Wi-Fi – `connect`

The client can request the ESP32 to connect to a specific Wi-Fi network.

Example command:

```json
{
  "action": "connect",
  "ssid": "Network_Name",
  "password": "password"
}
```

The ESP32 first switches to station mode:

```cpp
WiFi.mode(WIFI_STA);
```

It then attempts to connect using:

```cpp
WiFi.begin(ssid.c_str(), password.c_str());
```

The connection attempt has a timeout of approximately **10 seconds**.

Example response:

```json
{
  "teamId": "A19",
  "ssid": "Network_Name",
  "connected": true
}
```

If the connection cannot be established within the timeout period, `connected` will be `false`.

---

## 3. Get Recipe List – `getData`

Once connected to Wi-Fi, the ESP32 can request recipe information from the remote API.

Example command:

```json
{
  "action": "getData"
}
```

The ESP32 sends an HTTP GET request to the recipe endpoint.

The returned JSON array is parsed and the ESP32 extracts the following information for each recipe:

* recipe ID
* recipe name
* image URL
* team ID

Example response:

```json
{
  "id": "52772",
  "name": "Teriyaki Chicken Casserole",
  "image": "recipe_image_url",
  "teamId": "A19"
}
```

Each recipe is then transmitted to the client through Bluetooth.

---

## 4. Get Recipe Details – `getDetails`

The `getDetails` command retrieves additional information about a specific recipe.

Example command:

```json
{
  "action": "getDetails",
  "id": "52772"
}
```

The recipe ID is appended to the API endpoint and an HTTP GET request is performed.

The ESP32 extracts information such as:

* ID
* recipe name
* image
* category
* instructions
* YouTube video
* team ID

Example response structure:

```json
{
  "id": "52772",
  "name": "Teriyaki Chicken Casserole",
  "image": "recipe_image_url",
  "teamId": "A19",
  "description": "Name: ...\nCategory: ...\nArea: ...\nVideo: ..."
}
```

The formatted information is then transmitted back through Bluetooth.

---

## Invalid JSON Handling

Incoming Bluetooth messages are parsed using ArduinoJson:

```cpp
deserializeJson(jsonDoc, jsonData);
```

If the received message is not valid JSON, the ESP32 sends:

```json
{
  "error": "Invalid JSON"
}
```

This prevents malformed commands from being processed.

---

## Bluetooth Communication

Bluetooth Classic is initialized in `setup()`:

```cpp
SerialBT.begin("ESP32_TheMealDB");
```

The ESP32 therefore appears to nearby Bluetooth devices as:

```text
ESP32_TheMealDB
```

The main loop continuously checks for incoming Bluetooth data:

```cpp
if (SerialBT.available())
{
    String input = SerialBT.readStringUntil('\n');
    handleCommand(input);
}
```

Every received command must therefore end with a newline character (`\n`).

---

## How the Project Works

The complete workflow is:

1. A client connects to **ESP32_TheMealDB** through Bluetooth Classic.
2. The client sends a JSON command.
3. The ESP32 parses the command.
4. `getNetworks` can be used to discover available Wi-Fi networks.
5. `connect` connects the ESP32 to the selected network.
6. `getData` requests the available recipes from the remote API.
7. `getDetails` requests detailed information for a selected recipe.
8. The ESP32 processes the API JSON response.
9. The relevant information is converted into a new JSON object.
10. The result is transmitted back to the client through Bluetooth Classic.

---

## Example Workflow

### Scan networks

```json
{"action":"getNetworks"}
```

### Connect to Wi-Fi

```json
{
  "action":"connect",
  "ssid":"MyWiFi",
  "password":"mypassword"
}
```

### Request recipes

```json
{"action":"getData"}
```

### Request recipe details

```json
{
  "action":"getDetails",
  "id":"52772"
}
```
