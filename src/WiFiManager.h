#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
/**
 * @class WiFiManager
 * @brief Manages Wi-Fi connections and access point functionality for ESP32.
 * 
 * The WiFiManager class is responsible for handling Wi-Fi connectivity on the ESP32 platform. 
 * It abstracts the processes of connecting to a specified Wi-Fi network or starting an access point 
 * (AP) mode for configuration. The class utilizes the ESPAsyncWebServer library to serve web pages 
 * and handle incoming requests related to Wi-Fi configuration and GPIO control.
 * 
 * @note This class relies on a ConfigManager instance to store and retrieve Wi-Fi credentials
 * and settings. It also manages LED states and provides endpoints for client interactions.
 * 
 * Usage:
 * - Initialize an instance of WiFiManager.
 * - Call the begin() method to start the Wi-Fi manager, which will either connect to Wi-Fi 
 *   or start an access point based on the stored configuration.
 * - Set access point credentials using setAPCredentials() if needed.
 * - Utilize the web server to handle user interactions for configuring Wi-Fi or controlling GPIOs.
 * 
 * Public Methods:
 * - `WiFiManager()`: Constructor that initializes the WiFiManager instance.
 * - `void begin()`: Starts the Wi-Fi manager, determining the mode (Wi-Fi or AP).
 * - `void setAPCredentials(const char* ssid, const char* password)`: Sets the SSID and password 
 *   for the access point.
 * - `void setServerCallback()`: Configures the server routes and callbacks for handling web requests.
 * 
 * Private Methods:
 * - `void connectToWiFi()`: Attempts to connect to the specified Wi-Fi network using stored credentials.
 * - `void startAccessPoint()`: Initializes the ESP32 in access point mode with the configured SSID and password.
 * - `void handleRoot(AsyncWebServerRequest* request)`: Handles requests to the root URL by serving the 
 *   welcome page.
 * - `void handleSetWiFi(AsyncWebServerRequest* request)`: Serves the Wi-Fi credentials input page.
 * - `void handleSaveWiFi(AsyncWebServerRequest* request)`: Processes incoming requests to save Wi-Fi 
 *   credentials.
 * - `void handleGPIO(AsyncWebServerRequest* request)`: Serves the GPIO control page.
 * 
 * Member Variables:
 * - `ConfigManager* configManager`: Pointer to the ConfigManager for accessing configuration settings.
 * - `AsyncWebServer server`: An instance of AsyncWebServer to handle HTTP requests.
 * - `bool isAPMode`: Indicates whether the Wi-Fi manager is currently operating in AP mode.
 * - `String apSSID`: SSID for the access point.
 * - `String apPassword`: Password for the access point.
 * - `bool led1State`: State of LED 1 (on/off).
 * - `bool led2State`: State of LED 2 (on/off).
 */

#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>
#include "ConfigManager.h"


class WiFiManager {
public:
    // Constructor
    WiFiManager(ConfigManager* configManager);
    // Destructor to clean up allocated managers


    void begin();
    void setAPCredentials(const char* ssid, const char* password);    
    void setServerCallback();
    uint8_t getSignalStrengthPercent();
    char Message[100];
    bool isStillConnected();

private:
    bool led1State = false;
    bool led2State = false;
    void connectToWiFi();
    void startAccessPoint();
    void handleRoot(AsyncWebServerRequest* request);
    void handleSetWiFi(AsyncWebServerRequest* request);
    void handleSaveWiFi(AsyncWebServerRequest* request);

    

    ConfigManager* configManager;
    AsyncWebServer server;
    bool isAPMode;
    String apSSID;
    String apPassword;
    WiFiUDP ntpUDP;
};


#endif // WIFI_MANAGER_H