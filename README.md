# Simple and automatic WiFi handling for ESP8266 and ESP32

This library automatically manages the WiFi connection.
You only need to set your SSID and password.

What this library does:
- Automatically connect to the WiFi network.
- Automatically reconnect after a connection is lost.
- If enabled, display useful debugging messages to Serial.
- Take advantage of WiFi events and Timers to make this library completely non-blocking and event-driven.
- Compatible with both ESP32 and ESP8266.
- Provide callbacks for connection or disconnection events.

What this library does not:
- AP mode to set credentials with a web GUI.
- Other things that will make this library less light and simple.

This library is meant to be light and simple. Other libraries exist if you need advanced functionalities. However, take note that a more complex library is less light and simple.

# Get started right now

```c++
#include "EspSimpleWifiHandler.h"

EspSimpleWifiHandler wifiHandler("my_ssid", "my_password");

void onWifiConnected() {
  Serial.println("Wifi is connnected !");
}

void setup() {
  Serial.begin(115200);
  wifiHandler.enableDebuggingMessages();
  wifiHandler.onConnectionEstablished(onWifiConnected);
}

void loop() {
  // You can check if WiFi is connected by doing:
  if (wifiHandler.isConnected()) {
    // Code to run only if WiFI is connected
  }
}
```

Output:
```
WiFi: Initializing ...
WiFi: Connecting to your_ssid ...
WiFi: Connected (7.842000s), ip : 192.168.1.999
Wifi is connnected !
```

# Some documentation

## Constructors

Set WiFi infos right now (recommended):
```c++
  EspSimpleWifiHandler(
    const char *wifiSsid,
    const char *wifiPassword,
    const char *hostname = nullptr,     // if set, this lib will automatically set your hostname
    const bool autoConnect = true);     // if set, the wifi will not connect automatically.
```

If you want to set WiFi later:
```c++
  EspSimpleWifiHandler();
```

## Available functions

```c++

// Allow to display useful debugging messages. Must be enabled before the first loop call (in setup()) 
enableDebuggingMessages();

// Return true if the WiFi is currently connected
isConnected();

// Set the SSID and password. Only if you did not provide them in the constructor.
setWifiInfos(wifiSsid, wifiPassword);

// Set the hostname. Only if you did not provide it in the constructor.
setHostname(hostname);

// Set if this lib will automatically attempt to reconnect on WiFi connection lost. Default to true.
setAutoReconnect(const bool autoReconnect);

// Events callbacks setters
onConnectionEstablished(callback);
onConnectionLost(callback);

// Initialize a wifi connection. Only if you explicitly set the autoConnect to false or if you did not provide credential in the constructor
beginConnection();

// Disconect from the WiFi network. Doing this will also set both autoConnect and autoReconnect to flase.
disconnect();
```

