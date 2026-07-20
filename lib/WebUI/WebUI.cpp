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
    configureDebugWebSocket();

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
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", _spa->statusResponse.get());
        response->addHeader("Connection", "close");
        request->send(response);
    });

    server.on("/debug", HTTP_GET, [&](AsyncWebServerRequest *request) {
        debugD("uri: %s", request->url().c_str());
        request->send(SPIFFS, "/www/debug.htm");
    });

    // As a fallback we try to load from /www any requested URL
    server.serveStatic("/", SPIFFS, "/www/");

    server.begin();

    initialised = true;
}

void WebUI::configureDebugWebSocket() {
    /*
    * Give WebRemoteDebug access to the WebSocket, without giving it
    * ownership of the web server.
    */
    Debug.attachWebSocket(&_debugSocket);

    _debugSocket.onEvent(
        [this](
            AsyncWebSocket* server,
            AsyncWebSocketClient* client,
            AwsEventType type,
            void* arg,
            uint8_t* data,
            size_t len
        ) {
            handleDebugWebSocketEvent(
                server,
                client,
                type,
                arg,
                data,
                len
            );
        }
    );

    server.addHandler(&_debugSocket);

}

void WebUI::handleDebugWebSocketEvent(
    AsyncWebSocket* server,
    AsyncWebSocketClient* client,
    AwsEventType type,
    void* arg,
    uint8_t* data,
    size_t len
) {
    (void)server;

    switch (type) {
        case WS_EVT_CONNECT: {
            String message =
                "Connected to ESP32 debug WebSocket; level=";

            message += WebRemoteDebug::levelName(
                Debug.getWebDebugLevel()
            );

            message += ". Send 'help' for commands.";

            client->text(message);
            break;
        }

        case WS_EVT_DATA:
            handleDebugWebSocketData(
                client,
                static_cast<AwsFrameInfo*>(arg),
                data,
                len
            );
            break;

        case WS_EVT_DISCONNECT:
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
        default:
            break;
    }
}

void WebUI::handleDebugWebSocketData(
    AsyncWebSocketClient* client,
    AwsFrameInfo* info,
    uint8_t* data,
    size_t len
) {
    if (
        info == nullptr ||
        !info->final ||
        info->index != 0 ||
        info->len != len ||
        info->opcode != WS_TEXT
    ) {
        return;
    }

    String command;
    command.reserve(len);

    for (size_t i = 0; i < len; ++i) {
        command += static_cast<char>(data[i]);
    }

    command.trim();

    if (!processDebugCommand(command, client)) {
        client->text(
            "Unknown command: " + command
        );
    }
}

bool WebUI::processDebugCommand(
    const String& command,
    AsyncWebSocketClient* client
) {
    String normalisedCommand = command;
    normalisedCommand.trim();
    normalisedCommand.toLowerCase();

    if (normalisedCommand == "help") {
        client->text(
            "Commands: help, status, level verbose, "
            "level debug, level info, level warning, "
            "level error, level any, silence, resume"
        );

        return true;
    }

    if (normalisedCommand == "status") {
        String response = "WebSocket clients=";
        response += String(Debug.webClientCount());
        response += ", level=";
        response += WebRemoteDebug::levelName(
            Debug.getWebDebugLevel()
        );
        response += ", silenced=";
        response += Debug.isWebSilenced()
            ? "yes"
            : "no";

        client->text(response);
        return true;
    }

    if (normalisedCommand == "silence") {
        /*
        * Send the acknowledgement before silencing.
        */
        client->text("WebSocket logging silenced");
        Debug.setWebSilenced(true);
        return true;
    }

    if (normalisedCommand == "resume") {
        Debug.setWebSilenced(false);
        client->text("WebSocket logging resumed");
        return true;
    }

    if (normalisedCommand.startsWith("level ")) {
        String requestedLevel =
            normalisedCommand.substring(6);

        requestedLevel.trim();

        const int parsedLevel =
            WebRemoteDebug::parseLevel(requestedLevel);

        if (parsedLevel < 0) {
            client->text("Unknown debug level");
            return true;
        }

        Debug.setWebDebugLevel(
            static_cast<uint8_t>(parsedLevel)
        );

        String response =
            "WebSocket debug level set to ";

        response += WebRemoteDebug::levelName(
            Debug.getWebDebugLevel()
        );

        client->text(response);
        return true;
    }

    return false;
}
