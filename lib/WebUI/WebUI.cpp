#include "WebUI.h"
#ifdef ENABLE_ESPA_CONTROL
#include <HTTPClient.h>
#include <WiFi.h>
#endif

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

        if (_setSpaCallback != nullptr) {
            _setSpaCallback("reboot", "200");
            request->send(200, "text/html", "Called setSpaCallback for reboot...");
        } else {
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Rebooting ESP...");
            response->addHeader("Connection", "close");
            request->send(response);
            debugD("Rebooting...");
            delay(200);
            request->client()->setNoDelay(true);
            request->client()->stop();
            ESP.restart();
        }
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
#ifdef ENABLE_ESPA_CONTROL
        if (generateStatusJson(*_spa, *_mqttClient, json, true, _espaControl)) {
#else
        if (generateStatusJson(*_spa, *_mqttClient, json, true)) {
#endif
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

#ifdef ENABLE_ESPA_CONTROL
    // eSpa Control Pairing API Endpoints
    
    // Get eSpa Control configuration (server URL)
    server.on("/api/espa-control/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        bool isPaired = _espaControl ? _espaControl->isPaired() : false;
        String json = "{\"serverUrl\":\"" + String(ESPA_CONTROL_SERVER_URL) + \
                      "\",\"isPaired\":" + (isPaired ? "true" : "false") + "}";
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
        response->addHeader("Connection", "close");
        request->send(response);
    });
    
    // Get device ID
    server.on("/api/espa-control/device-id", HTTP_GET, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        String deviceId = _espaControl ? _espaControl->getDeviceId() : "unknown";
        String json = "{\"deviceId\":\"" + deviceId + "\"}";
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
        response->addHeader("Connection", "close");
        request->send(response);
    });

    // Handle pairing request
    server.on("/api/espa-control/pair", HTTP_POST, [this](AsyncWebServerRequest *request){}, NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        debugD("uri: %s", request->url().c_str());
        
        if (!_espaControl) {
            AsyncWebServerResponse *response = request->beginResponse(400, "application/json", 
                "{\"success\":false,\"message\":\"eSpa Control not initialized\"}");
            response->addHeader("Connection", "close");
            request->send(response);
            return;
        }

        // Parse JSON body
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, data, len);
        
        if (error) {
            AsyncWebServerResponse *response = request->beginResponse(400, "application/json",
                "{\"success\":false,\"message\":\"Invalid JSON\"}");
            response->addHeader("Connection", "close");
            request->send(response);
            return;
        }

        String deviceId = doc["deviceId"].as<String>();
        String pairingCode = doc["pairingCode"].as<String>();

        if (pairingCode.length() != 6) {
            AsyncWebServerResponse *response = request->beginResponse(400, "application/json",
                "{\"success\":false,\"message\":\"Invalid pairing code\"}");
            response->addHeader("Connection", "close");
            request->send(response);
            return;
        }

        // Use EspaControl's pairing method
        bool success = _espaControl->submitPairingCode(pairingCode);
        
        if (success) {
            debugI("Pairing code submitted successfully");
            String json = "{\"success\":true,\"message\":\"Pairing code submitted\"}";
            AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
            response->addHeader("Connection", "close");
            request->send(response);
        } else {
            debugE("Failed to submit pairing code");
            String json = "{\"success\":false,\"message\":\"Failed to submit pairing code\"}";
            AsyncWebServerResponse *response = request->beginResponse(400, "application/json", json);
            response->addHeader("Connection", "close");
            request->send(response);
        }
    });

    // Get pairing status
    server.on("/api/espa-control/pairing-status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        
        if (!_espaControl) {
            AsyncWebServerResponse *response = request->beginResponse(400, "application/json",
                "{\"status\":\"NONE\",\"message\":\"eSpa Control not initialized\"}");
            response->addHeader("Connection", "close");
            request->send(response);
            return;
        }

        // Get pairing state from EspaControl
        PairingState state = _espaControl->getPairingState();
        String statusStr;
        
        switch(state) {
            case NOT_PAIRED:
                statusStr = "NOT_PAIRED";
                break;
            case CODE_SUBMITTED:
                statusStr = "CODE_SUBMITTED";
                break;
            case POLLING:
                statusStr = "POLLING";
                break;
            case PAIRED:
                statusStr = "PAIRED";
                break;
            case PAIRING_ERROR:
                statusStr = "ERROR";
                break;
            default:
                statusStr = "UNKNOWN";
                break;
        }
        
        String json = "{\"status\":\"" + statusStr + "\",\"isPaired\":" + 
                      (_espaControl->isPaired() ? "true" : "false") + "}";
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
        response->addHeader("Connection", "close");
        request->send(response);
    });

    // Unpair device - clear authentication token
    server.on("/api/espa-control/unpair", HTTP_POST, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        
        if (_espaControl) {
            _espaControl->unpair();
            debugI("Device unpaired via web UI");
        }
        
        // Also clear config token for backwards compatibility
        _config->EspaToken.setValue("");
        _config->writeConfig();
        
        // Clear pairing state
        _pendingPairingDeviceId = "";
        _pairingStatus = "";
        
        String json = "{\"success\":true,\"message\":\"Device unpaired successfully\"}";
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
        response->addHeader("Connection", "close");
        request->send(response);
    });
#endif

    // As a fallback we try to load from /www any requested URL
    server.serveStatic("/", SPIFFS, "/www/");

    server.begin();

    initialised = true;
}