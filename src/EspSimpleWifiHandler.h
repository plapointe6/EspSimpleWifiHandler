/*
    EspSimpleWifiHandler.h
    Goal: Handle the wifi connection of both ESP32 and ESP8266
    Github: https://github.com/plapointe6/EspSimpleWifiHandler
*/

#ifndef ESP_SIMPLE_WIFI_HANDLER_H
#define ESP_SIMPLE_WIFI_HANDLER_H

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <Ticker.h>
#else // for ESP32
  #include <WiFi.h>
  #include <WiFiClient.h>
  extern "C" {
    #include "freertos/FreeRTOS.h"
    #include "freertos/timers.h"
  }
  #define Ticker TimerHandle_t
#endif

// 3 seconds of delay before the first connection id autoCOnnect is set to true
#define WIFI_DELAY_BEFORE_FIRST_CONNECTION 3

// Some people have reported instabilities when trying to connect to
// WiFi right after a disconnection, connection failure or after restting the radio.
// This delay prevent these instabilities.
#define WIFI_DELAY_BETWEEN_CONNECTIONS 1

typedef std::function<void()> Callback;

class EspSimpleWifiHandler
{

private:
  const char *_wifiSsid = nullptr;
  const char *_wifiPassword = nullptr;
  const char *_hostname = nullptr;
  bool _autoReconnect = true;

  // Optionnal functionnalities
  bool _enableSerialLogs = false;

  // Event callbacks
  Callback _connectionEstablishedCallback;
  Callback _connectionLostCallback;

  // Timers
  Ticker _wifiConnectTimer;
  Ticker _firstConnectionTimer;

  // WiFi event handlers (specific to ESP8266
  #ifdef ESP8266
    WiFiEventHandler gotIpEventHandler;
    WiFiEventHandler disconnectedEventHandler;
  #endif

public:
  /* Constructors */

  // Use this if you want to set the connection infos later
  EspSimpleWifiHandler();

  EspSimpleWifiHandler(
    const char *wifiSsid,
    const char *wifiPassword,
    const char *hostname = nullptr,
    const bool autoConnect = true);

  ~EspSimpleWifiHandler();


  // Allow to display useful debugging messages. Must be enabled before the first loop call (in setup()) 
  inline void enableDebuggingMessages(const bool enabled = true) { _enableSerialLogs = enabled; };

  // Return true if the WiFi is currently connected
  inline bool isConnected() const { return WiFi.status() == WL_CONNECTED; };


  /* Setters */

  // Set the SSID and password. Only if you did not provide them in the constructor.
  inline void setWifiInfos(const char *wifiSsid, const char *wifiPassword) { _wifiSsid = wifiSsid; _wifiPassword = wifiPassword; };

  // Set the hostname. Only if you did not provide it in the constructor.
  inline void setHostname(const char *hostname) { _hostname = hostname; };

  // Set if this lib will automatically attempt to reconnect on WiFi connection lost. Default to true.
  void setAutoReconnect(const bool autoReconnect) { _autoReconnect = autoReconnect; };


  /* Events callbacks setters */

  inline void onConnectionEstablished(Callback callback) { _connectionEstablishedCallback = callback; };
  inline void onConnectionLost(Callback callback) { _connectionLostCallback = callback; };


  /* Functionnalities */

  // Initialize a wifi connection. Only if you explicitly set the autoConnect to false or you don't provide credential in the constructor
  void beginConnection();

  // Disconect from the WiFi network. Doing this will also set both autoConnect and autoReconnect to flase.
  void disconnect();


private:
  // Connection state handling
  void _onWifiConnected();
  void _onWifiDisconnected();

  // Other private functions
  void _scheduleConnection();
  void _reset();
  void _connect();
  bool _canConnectToWifi();
};

#endif