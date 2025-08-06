#include "WiFiTools.h"

void WiFiTools::setup() {
    debugD("Setting up WiFiTools...");

    WiFi.setHostname(sanitizeHostname(_config->SpaName.getValue()).c_str());

    if (_config->SoftAPAlwaysOn.getValue()) {
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(WiFi.getHostname(), _config->SoftAPPassword.getValue().c_str());
    } else {
        WiFi.mode(WIFI_STA);
    }

    WiFi.begin();
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        debugI("Connected to Wi-Fi as %s", WiFi.getHostname());
        wifiRestored();
    } else {
        debugW("Failed to connect to Wi-Fi, starting AP mode");
        if (!_config->SoftAPAlwaysOn.getValue()) {
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP(WiFi.getHostname(), _config->SoftAPPassword.getValue().c_str());
        }
    }

    Debug.begin(WiFi.getHostname());  // The hostname here is used for display in the Debug library; see https://github.com/JoaoLopesF/RemoteDebug for details. It does not affect network functionality.
}

void WiFiTools::start() {
    debugD("Starting WiFiTools...");

    _running = true;
    xTaskCreate(runTask, "WiFiToolsTask", 2048, this, 1, &_taskHandle);
}

void WiFiTools::stop() {
    debugD("Stopping WiFiTools...");

    _running = false;
    if (_taskHandle != NULL) {
        vTaskDelete(_taskHandle);
        _taskHandle = NULL;
    }
}

void WiFiTools::runTask(void *pvParameters) {
    WiFiTools *self = static_cast<WiFiTools *>(pvParameters);

    while (self->_running) {
        self->loop();
        vTaskDelay(pdMS_TO_TICKS(3000)); // Run every 30s
    }
}

void WiFiTools::loop() {

    if (_wifiScanInProgress) {
        debugD("WiFi scan in progress, waiting for completion...");
    } else if (_connectToWiFiFlag) {
         debugD("WiFi connection in progress...");
    } else if (WiFi.status() != WL_CONNECTED) {
        debugI("WiFi not connected, disconnecting...");
        WiFi.disconnect();
        delay(200); // Give some time to disconnect
        debugI("Attempting to reconnect to WiFi...");
        WiFi.begin();
        if (WiFi.waitForConnectResult() == WL_CONNECTED) {
            debugI("Wifi reconnected");
            wifiRestored();
        } else {
            debugE("Failed to reconnect to Wi-Fi");
            if (WiFi.getMode() == WIFI_STA && !_config->SoftAPAlwaysOn.getValue()) {
                debugD("Failed to connect to Wi-Fi, starting AP mode");
                WiFi.mode(WIFI_AP_STA);
                WiFi.softAP(WiFi.getHostname(), _config->SoftAPPassword.getValue().c_str()); // Start the AP with the hostname and password
            } else {
                debugD("Failed to connect to Wi-Fi, but already in AP mode");
            }
        }
    }
}

void WiFiTools::updateSoftAP() {
    debugD("Changing SoftAP settings...");

    if (_config->SoftAPAlwaysOn.getValue()) {
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(_config->SpaName.getValue().c_str(), _config->SoftAPPassword.getValue().c_str());
        debugI("Soft AP enabled");
    } else {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
        debugI("Soft AP disabled");
    }
}

void WiFiTools::wifiRestored() {
    debugI("Wi-Fi connection restored");
    if (WiFi.getMode() == WIFI_AP_STA && !_config->SoftAPAlwaysOn.getValue()) {
        WiFi.softAPdisconnect(true); // Disable AP mode if reconnected
        WiFi.mode(WIFI_STA);
    }
    MDNS.end(); // Stop mDNS responder (if it was running)
    if (!MDNS.begin(WiFi.getHostname())) {
        debugE("Failed to start mDNS responder");
    } else {
        debugI("mDNS responder restarted");
    }
}

String WiFiTools::sanitizeHostname(const String &input) {
    String sanitized = "";

    // Process each character in the input string
    for (size_t i = 0; i < input.length() && i < 32; i++) {
        char c = input[i];
        // Keep only alphanumeric characters and hyphens
        if (isalnum(c) || c == '-') {
        sanitized += c;
        }
    }

    return sanitized;
}

void WiFiTools::RemoveDuplicateWiFiNetworks(std::vector<NetworkInfo>& networkList) {
    int n = WiFi.scanComplete();

    for (int i = 0; i < n; ++i) {
        String ssid = WiFi.SSID(i);
        int32_t rssi = WiFi.RSSI(i);
        wifi_auth_mode_t enc = WiFi.encryptionType(i);

        if (ssid.length() == 0) continue; // Skip hidden or empty SSIDs

        // Search for existing entry with the same SSID
        auto it = std::find_if(networkList.begin(), networkList.end(),
            [&](const NetworkInfo& net) { return net.ssid == ssid; });

        if (it == networkList.end()) {
            networkList.push_back(NetworkInfo{ ssid, rssi, enc });
        } else {
            // Keep the entry with the stronger signal
            if (it->rssi < rssi) {
                it->rssi = rssi;
                it->encryptionType = enc;
            }
        }
    }

    // Sort by rssi descending (strongest signal first)
    std::sort(networkList.begin(), networkList.end(), [](const NetworkInfo& a, const NetworkInfo& b) {
        return a.rssi > b.rssi;
    });
}

int WiFiTools::scanWiFiNetworks() {
    if (!_wifiScanInProgress) {
        if (WiFi.status() != WL_CONNECTED) {
            debugD("WiFi not connected, disable STA to start scan");
            WiFi.disconnect(); // Ensure WiFi is disconnected before scanning
        }
        WiFi.scanDelete(); // Clear previous scan results
        WiFi.scanNetworks(true, false); // async = true, show hidden = false
        _wifiScanInProgress = true;
        _wifiScanStartTime = millis();
        debugD("Starting WiFi scan...");
        return WIFI_SCAN_RUNNING;
    } else {
        int scanComplete = WiFi.scanComplete();
        if (scanComplete == WIFI_SCAN_RUNNING) {
            debugD("WiFi scan already in progress");
            return WIFI_SCAN_RUNNING;
        } else if (scanComplete >= 0) {
            _wifiScanInProgress = false;
            debugD("WiFi scan completed successfully");
            return scanComplete;
        } else if (millis() - _wifiScanStartTime > 30000) {
            // If scan is still running after 30 seconds, assume it failed
            _wifiScanInProgress = false;
            debugD("WiFi scan timed out");
            return -3;
        } else {
            debugD("WiFi scan failed");
            return WIFI_SCAN_FAILED;
        }
    }
}

String WiFiTools::getWiFiNetworksJSON() {
    std::vector<NetworkInfo> networkList;
    RemoveDuplicateWiFiNetworks(networkList);

    String json = "[";
    bool first = true;
    for (const auto& entry : networkList) {
        if (!first) json += ",";
        else first = false;
        json += "{";
        json += "\"ssid\":\"" + entry.ssid + "\",";
        json += "\"rssi\":" + String(entry.rssi) + ",";
        json += "\"secure\":" + String(entry.encryptionType != WIFI_AUTH_OPEN ? "true" : "false");
        json += "}";
    }
    json += "]";
    return json;
}

bool WiFiTools::connectToWiFi(String *ssid, String *password) {
    if (_connectToWiFiFlag) {
        debugD("WiFi connection already in progress, ignoring new request");
        return false;
    }

    _connectToWiFiFlag = true;
    debugD("Cleaning up previous WiFi connections...");
    WiFi.disconnect(false, true); // Clear previous connections
    debugD("Connecting to WiFi SSID: %s with password: %s", ssid->c_str(), password->c_str());
    if (password->length() == 0) {
        WiFi.begin(ssid->c_str());
    } else {
        WiFi.begin(ssid->c_str(), password->c_str());
    }

    if (WiFi.waitForConnectResult(10000) != WL_CONNECTED) {
        debugE("Failed to connect to WiFi SSID: %s", ssid->c_str());
    }

    _connectToWiFiFlag = false; // Reset connection flag
    return (WiFi.status() == WL_CONNECTED);
}
