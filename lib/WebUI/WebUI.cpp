#include "WebUI.h"

WebUI::WebUI(SpaInterface *spa, Config *config, MQTTClientWrapper *mqttClient) {
    _spa = spa;
    _config = config;
    _mqttClient = mqttClient;
}

const char * WebUI::getError() {
    return Update.errorString();
}

void WebUI::begin() {
    server.on("/reboot", HTTP_GET, [&](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        request->send(200, "text/plain", "Rebooting ESP...");
        debugD("Rebooting...");
        delay(200);
        request->client()->setNoDelay(true);
        request->client()->stop();
        ESP.restart();
    });

    server.on("/fota", HTTP_GET, [&](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        request->send(200, "text/html", fotaPage);
    });

    server.on("/config", HTTP_GET, [&](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        request->send(SPIFFS, "/www/config.htm");
    });

    server.on("/fota", HTTP_POST, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        if (Update.hasError()) {
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", String("Update error: ") + String(this->getError()));
            response->addHeader("Connection", "close");
            request->send(response);
        } else {
            request->client()->setNoDelay(true);
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
            response->addHeader("Connection", "close");
            request->send(response);
        }
    }, [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (index == 0) {
            static int updateType = U_FLASH; // Default to firmware update

            if (request->hasArg("updateType")) {
                String type = request->arg("updateType");
                if (type == "filesystem") {
                    updateType = U_SPIFFS;
                    debugD("Filesystem update selected.");
                } else if (type == "application") {
                    updateType = U_FLASH;
                    debugD("Application (firmware) update selected.");
                } else {
                    debugD("Unknown update type: %s", type.c_str());
                    //server->send(400, "text/plain", "Invalid update type");
                    //return;
                }
            } else {
                debugD("No update type specified. Defaulting to application update.");
            }

            debugD("Update: %s", filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN, updateType)) { // start with max available size
                debugD("Update Error: %s", this->getError());
            }
        }
        if (Update.write(data, len) != len) {
            debugD("Update Error: %s", this->getError());
        }
        if (final) {
            if (Update.end(true)) { // true to set the size to the current progress
                debugD("Update Success: %u\n", index + len);
            } else {
                debugD("Update Error: %s", this->getError());
            }
        }
    });

    server.on("/config", HTTP_POST, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        if (request->hasParam("spaName", true)) _config->SpaName.setValue(request->getParam("spaName", true)->value());
        if (request->hasParam("softAPAlwaysOn", true)) _config->SoftAPAlwaysOn.setValue(true);
        else _config->SoftAPAlwaysOn.setValue(false); // Default to false if not provided
        if (request->hasParam("softAPPassword", true)) _config->SoftAPPassword.setValue(request->getParam("softAPPassword", true)->value());
        if (request->hasParam("mqttServer", true)) _config->MqttServer.setValue(request->getParam("mqttServer", true)->value());
        if (request->hasParam("mqttPort", true)) _config->MqttPort.setValue(request->getParam("mqttPort", true)->value().toInt());
        if (request->hasParam("mqttUsername", true)) _config->MqttUsername.setValue(request->getParam("mqttUsername", true)->value());
        if (request->hasParam("mqttPassword", true)) _config->MqttPassword.setValue(request->getParam("mqttPassword", true)->value());
        if (request->hasParam("spaPollFrequency", true)) _config->SpaPollFrequency.setValue(request->getParam("spaPollFrequency", true)->value().toInt());
        _config->writeConfig();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Updated");
        response->addHeader("Connection", "close");
        request->send(response);
    });

    server.on("/json/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        String configJson = "{";
        configJson += "\"spaName\":\"" + _config->SpaName.getValue() + "\",";
        configJson += "\"softAPAlwaysOn\":" + String(_config->SoftAPAlwaysOn.getValue() ? "true" : "false") + ",";
        configJson += "\"softAPPassword\":\"" + _config->SoftAPPassword.getValue() + "\",";
        configJson += "\"mqttServer\":\"" + _config->MqttServer.getValue() + "\",";
        configJson += "\"mqttPort\":\"" + String(_config->MqttPort.getValue()) + "\",";
        configJson += "\"mqttUsername\":\"" + _config->MqttUsername.getValue() + "\",";
        configJson += "\"mqttPassword\":\"" + _config->MqttPassword.getValue() + "\",";
        configJson += "\"spaPollFrequency\":" + String(_config->SpaPollFrequency.getValue());
        configJson += "}";
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", configJson);
        response->addHeader("Connection", "close");
        request->send(response);
    });

    server.on("/json", HTTP_GET, [&](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        String json;
        AsyncWebServerResponse *response;
        if (generateStatusJson(*_spa, *_mqttClient, json, true)) {
            response = request->beginResponse(200, "application/json", json);
        } else {
            response = request->beginResponse(200, "text/plain", "Error generating json");
        }
        response->addHeader("Connection", "close");
        request->send(response);
    });

    // Handle /set endpoint (POST)
    server.on("/set", HTTP_POST, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());

        if (_setSpaCallback != nullptr) {
            for (uint8_t i = 0; i < request->params(); i++) {
                _setSpaCallback(request->getParam(i)->name(), request->getParam(i)->value());
            }
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Spa update initiated");
            response->addHeader("Connection", "close");
            request->send(response);
        } else {
            AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", "setSpaCallback not set");
            response->addHeader("Connection", "close");
            request->send(response);
        }
    });

    // Handle /wifi-manager endpoint (GET)
    server.on("/wifi-manager", HTTP_GET, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "WiFi Manager launching, connect to ESP WiFi...");
        response->addHeader("Connection", "close");
        request->send(response);
        if (_wifiManagerCallback != nullptr) { _wifiManagerCallback(); }
    });

    server.on("/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", _spa->statusResponse.getValue());
        response->addHeader("Connection", "close");
        request->send(response);
    });

    server.on("/scan", HTTP_GET, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        if (!wifiScanInProgress) {
            if (WiFi.status() != WL_CONNECTED) {
                debugD("WiFi not connected, disable STA to start scan");
                WiFi.disconnect(); // Ensure WiFi is disconnected before scanning
            }
            WiFi.scanNetworks(true, false); // async = true, show hidden = false
            wifiScanInProgress = true;
            wifiScanStartTime = millis();
            debugD("Starting WiFi scan...");
            request->send(202, "application/json", "{\"status\":\"scan_started\"}");
        } else {
            int scanComplete = WiFi.scanComplete();
            if (scanComplete == WIFI_SCAN_RUNNING) {
                debugD("WiFi scan already in progress");
                request->send(202, "application/json", "{\"status\":\"scan_in_progress\"}");
            } else if (scanComplete >= 0) {
                std::vector<NetworkInfo>& networkMap = *(new std::vector<NetworkInfo>());
                RemoveDuplicateWiFiNetworks(networkMap);
                bool first = true;
                String json = "[";
                for (const auto& entry : networkMap) {
                    if (!first) json += ",";
                    else first = false;
                    json += "{";
                    json += "\"ssid\":\"" + entry.ssid + "\",";
                    json += "\"rssi\":" + String(entry.rssi) + ",";
                    json += "\"secure\":" + String(entry.encryptionType != WIFI_AUTH_OPEN ? "true" : "false");
                    json += "}";
                }
                json += "]";
                WiFi.scanDelete(); // clear results for next scan
                wifiScanInProgress = false;
                debugD("WiFi scan completed successfully");
                request->send(200, "application/json", json);
            } else if (millis() - wifiScanStartTime > 30000) {
                // If scan is still running after 30 seconds, assume it failed
                wifiScanInProgress = false;
                WiFi.scanDelete(); // Clear previous scan results
                debugD("WiFi scan timed out or failed");
                request->send(500, "application/json", "{\"error\":\"scan timeout\"}");
            } else {
                debugD("WiFi scan already in progress");
                request->send(202, "application/json", "{\"status\":\"scan_in_progress\"}");
            }
        }
    });

    server.on("/connect", HTTP_POST, [this](AsyncWebServerRequest *request){
        debugD("uri: %s", request->url().c_str());
        String ssid, password;

        if (wifiConnect) {
            debugD("WiFi connection already in progress, ignoring new request");
            request->send(429, "application/json", "{\"error\":\"Connection already in progress\"}");
            return;
        }
        if (request->hasParam("ssid", true)) {
            ssid = request->getParam("ssid", true)->value();
            ssid.trim(); // Remove leading/trailing whitespace
            debugD("ssid: %s", ssid.c_str());
        }
        if (request->hasParam("password", true)) {
            password = request->getParam("password", true)->value();
            password.trim(); // Remove leading/trailing whitespace
            debugD("password: %s", password.c_str());
        }

        if (ssid.length() == 0) {
            debugD("SSID not provided");
            request->send(400, "application/json", "{\"error\":\"SSID required\"}");
            return;
        }

        wifiConnect = true;
        debugD("Cleaning up previous WiFi connections...");
        WiFi.disconnect(false, true); // Clear previous connections
        debugD("Connecting to WiFi SSID: %s with password: %s", ssid.c_str(), password.c_str());
        if (password.length() == 0) {
            WiFi.begin(ssid.c_str());
        } else {
            WiFi.begin(ssid.c_str(), password.c_str());
        }

        // Optional: wait for connection (blocking, or use task/timer for async)
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(250);
            debugD("Waiting for WiFi connection...");
        }

        wifiConnect = false; // Reset connection flag
        if (WiFi.status() == WL_CONNECTED) {
            debugD("Connected to WiFi SSID: %s", ssid.c_str());
            request->send(200, "application/json", "{\"success\":true}");
        } else {
            debugD("Failed to connect to WiFi SSID: %s", ssid.c_str());
            request->send(500, "application/json", "{\"success\":false,\"reason\":\"Connection failed\"}");
        }
    });


    // As a fallback we try to load from /www any requested URL
    server.serveStatic("/", SPIFFS, "/www/");

    server.begin();

    initialised = true;
}

void WebUI::RemoveDuplicateWiFiNetworks(std::vector<NetworkInfo>& networkList) {
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
