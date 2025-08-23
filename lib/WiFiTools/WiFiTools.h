#ifndef WIFITOOLS_H
#define WIFITOOLS_H

#include <Arduino.h>
#include <RemoteDebug.h>
#include <WiFi.h>
#include <Config.h>
#include <ESPmDNS.h>
#include <vector>

extern RemoteDebug Debug;

struct NetworkInfo {
        String ssid;
        int32_t rssi;
        wifi_auth_mode_t encryptionType;
};

class WiFiTools {
public:
    WiFiTools(Config *config) {
        _config = config;
    };

    void setup();
    void start();
    void stop();
    void updateSoftAP();
    bool connectToWiFi(String *ssid, String *password);
    int scanWiFiNetworks();
    String getWiFiNetworksJSON();

private:
    bool _running = false;
    TaskHandle_t _taskHandle = NULL;
    Config *_config;
    bool _wifiScanInProgress = false; // Flag to indicate if a Wi-Fi scan is in progress
    ulong _wifiScanStartTime = 0; // Start time of the Wi-Fi scan
    bool _connectToWiFiFlag = false; // Flag to indicate if a Wi-Fi connection is in progress

    static void runTask(void *pvParameters);
    void loop();
    void wifiRestored();
    String sanitizeHostname(const String &hostname);
    void RemoveDuplicateWiFiNetworks(std::vector<NetworkInfo>& networkList);
};

#endif // WIFITOOLS_H
