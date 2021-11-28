#include "EspSimpleWifiHandler.h"

/* ======== COnstructors ========= */

EspSimpleWifiHandler::EspSimpleWifiHandler()
  : EspSimpleWifiHandler(nullptr, nullptr, nullptr, false)
{}

EspSimpleWifiHandler::EspSimpleWifiHandler(
  const char *wifiSsid,
  const char *wifiPassword,
  const char *hostname,
  const bool autoConnect) : 
  _wifiSsid(wifiSsid),
  _wifiPassword(wifiPassword),
  _hostname(hostname)
{
  // Setup timer (specific for ESP32)
  #ifndef ESP8266
    _firstConnectionTimer = xTimerCreate(
      "wifiFirstConnectionTimer",
      pdMS_TO_TICKS(WIFI_DELAY_BEFORE_FIRST_CONNECTION * 1000),
      pdFALSE, // Don't reload after it's finished
      this,
      [](TimerHandle_t xTimer) {
        auto _this = static_cast<EspSimpleWifiHandler *>(pvTimerGetTimerID(xTimer));
        _this->beginConnection();
      }
    );
    _wifiConnectTimer = xTimerCreate(
      "wifiConnectTimer",
      pdMS_TO_TICKS(WIFI_DELAY_BETWEEN_CONNECTIONS * 1000),
      pdFALSE,
      this,
      [](TimerHandle_t xTimer) {
        auto _this = static_cast<EspSimpleWifiHandler *>(pvTimerGetTimerID(xTimer));
        _this->_connect();
      }
    );
  #endif

  // Register the WiFi connection / disconnection events
  #ifdef ESP8266
    gotIpEventHandler = WiFi.onStationModeGotIP([this](const WiFiEventStationModeGotIP &event) {
      this->_onWifiConnected();
    });
    disconnectedEventHandler = WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected& event) {
        this->_onWifiDisconnected();
    });
  #else // for ESP32
    WiFi.onEvent(
      [this](WiFiEvent_t event, system_event_info_t info) {
        this->_onWifiConnected();
      },
      WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP
    );

    WiFi.onEvent(
      [this](WiFiEvent_t event, system_event_info_t info) {
        this->_onWifiDisconnected();
      },
      WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED
    );
  #endif

  // If autoConnect is set to true, schedule a timer for autoConnection
  if (autoConnect) {
    #ifdef ESP8266
      _firstConnectionTimer.once(WIFI_DELAY_BEFORE_FIRST_CONNECTION, [this]{ this->beginConnection(); });
    #else
      xTimerStart(_firstConnectionTimer, 0);
    #endif
  }
}

EspSimpleWifiHandler::~EspSimpleWifiHandler() {
  #ifdef ESP32
    xTimerDelete(_firstConnectionTimer, 0);
    xTimerDelete(_wifiConnectTimer, 0);
  #endif
}

// =============== WiFi Functionnalities =================

// Initialize a wifi connection.
void EspSimpleWifiHandler::beginConnection() {
  // Ensure we can connect to WiFi
  if (!_canConnectToWifi())
    return;

  // Ensure that no timer are running
  #ifdef ESP8266
    _firstConnectionTimer.detach();
    _wifiConnectTimer.detach();
  #else
    xTimerStop(_firstConnectionTimer, 0);
    xTimerStop(_wifiConnectTimer, 0);
  #endif

  if (_enableSerialLogs)
    Serial.println("WiFi: Initializing ...");

  // Set the hostname if needed
  if (_hostname) {
    #ifdef ESP32
      WiFi.setHostname(_hostname);
    #else
      WiFi.hostname(_hostname);
    #endif
  }

  // Schedule a wifi connection
  _scheduleConnection();
}

// Disconnect from WiFi
void EspSimpleWifiHandler::disconnect() {
  if (_enableSerialLogs)
    Serial.println("WiFi: Disconnecting ...");

  _autoReconnect = false;

  // Stop timers
  #ifdef ESP8266
    _firstConnectionTimer.detach();
    _wifiConnectTimer.detach();
  #else
    xTimerStop(_firstConnectionTimer, 0);
    xTimerStop(_wifiConnectTimer, 0);
  #endif

  // Disconnect and reset
  _reset();
}

// =============== Connection state handling =================

void EspSimpleWifiHandler::_onWifiConnected() {
  if (_enableSerialLogs)
    Serial.printf("WiFi: Connected (%fs), ip : %s \n", millis() / 1000.0, WiFi.localIP().toString().c_str());

  if (_connectionEstablishedCallback)
    _connectionEstablishedCallback();
}

void EspSimpleWifiHandler::_onWifiDisconnected() {
  if (_enableSerialLogs)
    Serial.printf("WiFi! Lost connection (%fs). \n", millis() / 1000.0);

  if (_autoReconnect)
    _scheduleConnection();

  if (_connectionLostCallback)
    _connectionLostCallback();
}

// ================== Other Private functions ====================-

// Reset WiFi and schedule a connection
void EspSimpleWifiHandler::_scheduleConnection() {
  // To avoid issues, disconnect and reset params
  _reset();

  // Schedule a connection
  #ifdef ESP8266
    _wifiConnectTimer.once(WIFI_DELAY_BETWEEN_CONNECTIONS, [this]{ this->_connect(); });
  #else
    xTimerStart(_wifiConnectTimer, 0);
  #endif
}

// Disconnect, reset the WiFi radio and prepare for a new connection
void EspSimpleWifiHandler::_reset() {
  #if defined(ESP8266)
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
  #else
    wifi_mode_t m = WiFi.getMode();
    if (!(m & WIFI_MODE_STA))
      WiFi.mode(WIFI_STA);
    WiFi.disconnect(false, true);
    if (!(m & WIFI_MODE_STA))
      WiFi.mode(m);
  #endif
}

// Connect to the WiFi network
void EspSimpleWifiHandler::_connect() {
  if (!_canConnectToWifi())
    return;

  if (_enableSerialLogs)
    Serial.printf("WiFi: Connecting to %s ... \n", _wifiSsid);

  if (_wifiPassword && strlen_P(_wifiPassword) > 0)
    WiFi.begin(_wifiSsid, _wifiPassword);
  else
    WiFi.begin(_wifiSsid);
}

bool EspSimpleWifiHandler::_canConnectToWifi() {
  // Dont connect if the ssid is not set.
  if (!_wifiSsid || strlen_P(_wifiSsid) == 0) {
    if (_enableSerialLogs)
      Serial.printf("WiFi! Can't init a connection: WiFi not configured. \n");
    return false;
  }

  // Don't connect if we are already connected
  if (isConnected()) {
    if (_enableSerialLogs)
      Serial.printf("WiFi! Can't init a connection: WiFi already connected. \n");
    return false;
  } else {
    return true;
  }
}
