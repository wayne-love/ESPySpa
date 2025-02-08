#include "WebUI.h"

WebUI::WebUI(SpaInterface *spa, Config *config, MQTTClientWrapper *mqttClient) {
    _spa = spa;
    _config = config;
    _mqttClient = mqttClient;
}

void WebUI::setWifiManagerCallback(void (*f)()) {
    _wifiManagerCallback = f;
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
        if (request->hasParam("mqttServer", true)) _config->MqttServer.setValue(request->getParam("mqttServer", true)->value());
        if (request->hasParam("mqttPort", true)) _config->MqttPort.setValue(request->getParam("mqttPort", true)->value().toInt());
        if (request->hasParam("mqttUsername", true)) _config->MqttUsername.setValue(request->getParam("mqttUsername", true)->value());
        if (request->hasParam("mqttPassword", true)) _config->MqttPassword.setValue(request->getParam("mqttPassword", true)->value());
        if (request->hasParam("updateFrequency", true)) _config->UpdateFrequency.setValue(request->getParam("updateFrequency", true)->value().toInt());
        _config->writeConfig();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Updated");
        response->addHeader("Connection", "close");
        request->send(response);
    });

    server.on("/json/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        String configJson = "{";
        configJson += "\"spaName\":\"" + _config->SpaName.getValue() + "\",";
        configJson += "\"mqttServer\":\"" + _config->MqttServer.getValue() + "\",";
        configJson += "\"mqttPort\":\"" + String(_config->MqttPort.getValue()) + "\",";
        configJson += "\"mqttUsername\":\"" + _config->MqttUsername.getValue() + "\",";
        configJson += "\"mqttPassword\":\"" + _config->MqttPassword.getValue() + "\",";
        configJson += "\"updateFrequency\":" + String(_config->UpdateFrequency.getValue());
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
        // In theory with minor modification, we can reuse mqttCallback here
        // for (uint8_t i = 0; i < request->params(); i++) updateSpaSetting("set/" + request->getParam(i)->name(), request->getParam(i)->value());
        if (request->hasParam("temperatures_setPoint", true)) {
            float newTemperature = request->getParam("temperatures_setPoint", true)->value().toFloat();
            _spa->setSTMP(int(newTemperature * 10));
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Temperature updated");
            response->addHeader("Connection", "close");
            request->send(response);
        } else if (request->hasParam("status_datetime", true)) {
            String p = request->getParam("status_datetime", true)->value();
            tmElements_t tm;
            tm.Year = CalendarYrToTm(p.substring(0, 4).toInt());
            tm.Month = p.substring(5, 7).toInt();
            tm.Day = p.substring(8, 10).toInt();
            tm.Hour = p.substring(11, 13).toInt();
            tm.Minute = p.substring(14, 16).toInt();
            tm.Second = p.substring(17).toInt();
            _spa->setSpaTime(makeTime(tm));
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Date/Time updated");
            response->addHeader("Connection", "close");
            request->send(response);
        } else {
            AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", "Invalid temperature value");
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

    // As a fallback we try to load from /www any requested URL
    server.serveStatic("/", SPIFFS, "/www/");

    server.begin();

    initialised = true;
}