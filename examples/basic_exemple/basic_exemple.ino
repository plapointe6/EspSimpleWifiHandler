#include "EspSimpleWifiHandler.h"

// Create the WiFi Handler.
// By default, he will automatically connect to the specified WiFi.
EspSimpleWifiHandler wifiHandler("WIFI_SSID", "WIFI_PASSWORD");

// Optionnal WiFI event handler
void onWifiConnected() {
  Serial.println("onWifiConnected: Wifi is connnected !");
}
void onWifiDisconnected() {
  Serial.println("onWifiDisconnected: connection lost.");
}

void setup() {
  Serial.begin(115200);

  // Optionnal: Enable debugging messages
  wifiHandler.enableDebuggingMessages();

  // Optinnal: You can subscribe to WiFI events
  wifiHandler.onConnectionEstablished(onWifiConnected);
  wifiHandler.onConnectionLost(onWifiDisconnected);
}

void loop() {
    
}