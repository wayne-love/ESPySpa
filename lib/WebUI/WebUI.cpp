#include "WebUI.h"

WebUI::WebUI(SpaInterface *spa, Config *config) {
    _spa = spa;
    _config = config;
}

void WebUI::setWifiManagerCallback(void (*f)()) {
    _wifiManagerCallback = f;
}

const char * WebUI::getError() {
    return Update.errorString();
}


void WebUI::begin() {
        
    server.reset(new WebServer(80));

    server->on("/", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::indexPageTemplate);
    });

    server->on("/json", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        String json;
        SpaInterface &si = *_spa;
        if (generateStatusJson(si, json, true)) {
            server->send(200, "text/json", json.c_str());
        } else {
            server->send(200, "text/text", "Error generating json");
        }
    });

    server->on("/reboot", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->send(200, "text/html", WebUI::rebootPage);
        debugD("Rebooting...");
        delay(200);
        server->client().stop();
        ESP.restart();
    });

    server->on("/styles.css", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->send(200, "text/css", WebUI::styleSheet);
    });

    server->on("/fota", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::fotaPage);
    });

    server->on("/fota", HTTP_POST, [&]() {
        debugD("uri: %s", server->uri().c_str());
        if (Update.hasError()) {
            server->sendHeader("Connection", "close");
            server->send(200, F("text/plain"), String(F("Update error: ")) + String(getError()));
        } else {
            server->client().setNoDelay(true);
            server->sendHeader("Connection", "close");
            server->send(200, "text/plain", "OK");
            debugD("Rebooting...");
            delay(100);
            server->client().stop();
            ESP.restart();
        }
    }, [&]() {
        debugD("uri: %s", server->uri().c_str());
        HTTPUpload& upload = server->upload();
        if (upload.status == UPLOAD_FILE_START) {
            debugD("Update: %s", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                debugD("Update Error: %s",getError());
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            /* flashing firmware to ESP*/
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                debugD("Update Error: %s",getError());
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
                debugD("Update Success: %u\n", upload.totalSize);
            } else {
                debugD("Update Error: %s",getError());
            }
        }
    });

    server->on("/config", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::configPageTemplate);
    });

    server->on("/config", HTTP_POST, [&]() {
        debugD("uri: %s", server->uri().c_str());
        if (server->hasArg("spaName")) _config->SpaName.setValue(server->arg("spaName"));
        if (server->hasArg("mqttServer")) _config->MqttServer.setValue(server->arg("mqttServer"));
        if (server->hasArg("mqttPort")) _config->MqttPort.setValue(server->arg("mqttPort"));
        if (server->hasArg("mqttUsername")) _config->MqttUsername.setValue(server->arg("mqttUsername"));
        if (server->hasArg("mqttPassword")) _config->MqttPassword.setValue(server->arg("mqttPassword"));
        if (server->hasArg("updateFrequency")) _config->UpdateFrequency.setValue(server->arg("updateFrequency").toInt());
        _config->writeConfigFile();
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", "Updated");
    });

    server->on("/json/config", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        String configJson = "{";
        configJson += "\"spaName\":\"" + _config->SpaName.getValue() + "\",";
        configJson += "\"mqttServer\":\"" + _config->MqttServer.getValue() + "\",";
        configJson += "\"mqttPort\":\"" + _config->MqttPort.getValue() + "\",";
        configJson += "\"mqttUsername\":\"" + _config->MqttUsername.getValue() + "\",";
        configJson += "\"mqttPassword\":\"" + _config->MqttPassword.getValue() + "\",";
        configJson += "\"updateFrequency\":" + String(_config->UpdateFrequency.getValue());
        configJson += "}";
        server->send(200, "application/json", configJson);
    });

    server->on("/set", HTTP_POST, [&]() {
        debugD("uri: %s", server->uri().c_str());
        //In theory with minor modification, we can reuse mqttCallback here
        //for (uint8_t i = 0; i < server->args(); i++) updateSpaSetting("set/" + server->argName(0), server->arg(0));
        if (server->hasArg("temperatures_setPoint")) {
            float newTemperature = server->arg("temperatures_setPoint").toFloat();
            SpaInterface &si = *_spa;
            si.STMP.sendValue(int(newTemperature*10));
            server->send(200, "text/plain", "Temperature updated");
        }
        else if (server->hasArg("status_datetime")) {
            String p = server->arg("status_datetime");
            SpaInterface &si = *_spa;
            tmElements_t tm;
            tm.Year=CalendarYrToTm(p.substring(0,4).toInt());
            tm.Month=p.substring(5,7).toInt();
            tm.Day=p.substring(8,10).toInt();
            tm.Hour=p.substring(11,13).toInt();
            tm.Minute=p.substring(14,16).toInt();
            tm.Second=p.substring(17).toInt();
            si.SpaTime.sendValue(makeTime(tm));
            server->send(200, "text/plain", "Date/Time updated");
        }
        else {
            server->send(400, "text/plain", "Invalid temperature value");
        }
    });

    server->on("/wifi-manager", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", "WiFi Manager launching, connect to ESP WiFi...");
        if (_wifiManagerCallback != nullptr) { _wifiManagerCallback(); }
    });

    server->on("/json.html", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::jsonHTMLTemplate);
    });

    server->on("/status", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        SpaInterface &si = *_spa;
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", si.statusResponse.getValue());
    });

    server->begin();

    initialised = true;
}