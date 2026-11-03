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
    configureAppWebSocket();
    configureDebugWebSocket();

    server.on("/reboot", HTTP_GET, [&](AsyncWebServerRequest *request) {
        debugD("uri: %s, client IP: %s", request->url().c_str(), request->client()->remoteIP().toString().c_str());

        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Rebooting ESP...");
        response->addHeader("Connection", "close");
        request->client()->setNoDelay(true);
        request->send(response);
        request->client()->close();
        debugD("Rebooting...");
        delay(200);
        ESP.restart();
    });

    server.on("/fota", HTTP_GET, [&](AsyncWebServerRequest *request) {
        debugD("uri: %s, client IP: %s", request->url().c_str(), request->client()->remoteIP().toString().c_str());
        request->send(200, "text/html", fotaPage);
    });

    server.on("/config", HTTP_GET, [&](AsyncWebServerRequest *request) {
        debugD("uri: %s, client IP: %s", request->url().c_str(), request->client()->remoteIP().toString().c_str());
        request->send(SPIFFS, "/www/config.htm");
    });

    server.on("/fota", HTTP_POST, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s, client IP: %s", request->url().c_str(), request->client()->remoteIP().toString().c_str());
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
        debugD("uri: %s, client IP: %s", request->url().c_str(), request->client()->remoteIP().toString().c_str());
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
        debugD("uri: %s, client IP: %s", request->url().c_str(), request->client()->remoteIP().toString().c_str());
        String configJson = buildConfigJson();
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", configJson);
        response->addHeader("Connection", "close");
        request->send(response);
    });

    server.on("/json", HTTP_GET, [&](AsyncWebServerRequest *request) {
        debugD("uri: %s, client IP: %s", request->url().c_str(), request->client()->remoteIP().toString().c_str());
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
        debugD("uri: %s, client IP: %s", request->url().c_str(), request->client()->remoteIP().toString().c_str());

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
        debugD("uri: %s, client IP: %s", request->url().c_str(), request->client()->remoteIP().toString().c_str());
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "WiFi Manager launching, connect to ESP WiFi...");
        response->addHeader("Connection", "close");
        request->send(response);
        if (_wifiManagerCallback != nullptr) { _wifiManagerCallback(); }
    });

    server.on("/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        debugD("uri: %s, client IP: %s", request->url().c_str(), request->client()->remoteIP().toString().c_str());
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", _spa->statusResponse.get());
        response->addHeader("Connection", "close");
        request->send(response);
    });

    // As a fallback we try to load from /www any requested URL
    server.serveStatic("/", SPIFFS, "/www/");

    server.begin();

    initialised = true;
}

void WebUI::notifySpaUpdated() {
    if (!initialised) {
        return;
    }

    // Remove disconnected clients before deciding whether a broadcast is needed.
    _appSocket.cleanupClients();

    const size_t clientCount = _appSocket.count();
    debugD("notifySpaUpdated called, appSocket count: %u", static_cast<unsigned>(clientCount));

    if (clientCount == 0) {
        return;
    }

    sendStatus();
}

void WebUI::configureAppWebSocket() {
    _appSocket.onEvent(
        [this](
            AsyncWebSocket* server,
            AsyncWebSocketClient* client,
            AwsEventType type,
            void* arg,
            uint8_t* data,
            size_t len
        ) {
            handleAppWebSocketEvent(server, client, type, arg, data, len);
        }
    );

    server.addHandler(&_appSocket);
}

void WebUI::handleAppWebSocketEvent(
    AsyncWebSocket* server,
    AsyncWebSocketClient* client,
    AwsEventType type,
    void* arg,
    uint8_t* data,
    size_t len
) {
    debugD("handleAppWebSocketEvent: type=%d, client ip=%s, client id=%u, data len=%zu", type, client->remoteIP().toString().c_str(), client->id(), len);
    (void)server;

    switch (type) {
        case WS_EVT_CONNECT:
            sendStatus(client);
            break;

        case WS_EVT_DATA:
            handleAppWebSocketData(
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

void WebUI::handleAppWebSocketData(
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
        debugD("handleAppWebSocketData: Invalid frame info");
        return;
    }

    String command;
    command.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        command += static_cast<char>(data[i]);
    }

    debugD("handleAppWebSocketData: Received command: %s", command.c_str());
    command.trim();
    processAppCommand(client, command);
}

void WebUI::processAppCommand(
    AsyncWebSocketClient* client,
    const String& command
) {
    if (command == "get:status") {
        sendStatus(client);
        return;
    }

    if (command == "get:config") {
        sendConfig(client);
        return;
    }

    if (command == "reboot") {
        sendMessage("ack", "null", "Rebooting ESP...", client);
        delay(200);
        ESP.restart();
        return;
    }

    if (command.startsWith("set:")) {
        if (_setSpaCallback == nullptr) {
            sendMessage("error", "null", "setSpaCallback not set", client);
            return;
        }

        const String payload = command.substring(4);
        const int separator = payload.indexOf('=');
        if (separator < 1) {
            sendMessage("error", "null", "Invalid set command", client);
            return;
        }

        const String name = urlDecode(payload.substring(0, separator));
        const String value = urlDecode(payload.substring(separator + 1));
        _setSpaCallback(name, value);
        sendMessage("ack", "null", "Spa update initiated", client);
        return;
    }

    if (command.startsWith("config:")) {
        if (applyFormEncodedConfig(command.substring(7))) {
            sendMessage("ack", "null", "Configuration updated", client);
            sendConfig(client);
        } else {
            sendMessage("error", "null", "Invalid configuration data", client);
        }
        return;
    }

    sendMessage("error", "null", "Unknown command", client);
}

void WebUI::sendMessage(
    const String& type,
    const String& dataJson,
    const String& message,
    AsyncWebSocketClient* client
) {
    String envelope;
    envelope.reserve(
        type.length() +
        dataJson.length() +
        message.length() +
        48
    );

    envelope = "{\"type\":\"";
    envelope += jsonEscape(type);
    envelope += "\",\"data\":";
    envelope += (dataJson.length() > 0 ? dataJson : "null");

    if (message.length() > 0) {
        envelope += ",\"message\":\"";
        envelope += jsonEscape(message);
        envelope += "\"";
    }

    envelope += "}";

    if (client != nullptr) {
        client->text(envelope);
    } else {
        _appSocket.textAll(envelope);
    }
}

bool WebUI::sendStatus(AsyncWebSocketClient* client) {
    debugD("sendStatus called, client: %s", client ? client->remoteIP().toString().c_str() : "broadcast");

    String json;
    if (!generateStatusJson(*_spa, *_mqttClient, json, true)) {
        if (client != nullptr) {
            sendMessage("error", "null", "Error generating status JSON", client);
        }
        return false;
    }

    sendMessage("status", json, "", client);
    return true;
}

void WebUI::sendConfig(AsyncWebSocketClient* client) {
    debugD("sendConfig called, client: %s", client ? client->remoteIP().toString().c_str() : "broadcast");
    sendMessage("config", buildConfigJson(), "", client);
}

String WebUI::buildConfigJson() const {
    String configJson = "{";
    configJson += "\"spaName\":\"" + jsonEscape(_config->SpaName.getValue()) + "\",";
    configJson += "\"softAPAlwaysOn\":" + String(_config->SoftAPAlwaysOn.getValue() ? "true" : "false") + ",";
    configJson += "\"softAPPassword\":\"" + jsonEscape(_config->SoftAPPassword.getValue()) + "\",";
    configJson += "\"mqttServer\":\"" + jsonEscape(_config->MqttServer.getValue()) + "\",";
    configJson += "\"mqttPort\":" + String(_config->MqttPort.getValue()) + ",";
    configJson += "\"mqttUsername\":\"" + jsonEscape(_config->MqttUsername.getValue()) + "\",";
    configJson += "\"mqttPassword\":\"" + jsonEscape(_config->MqttPassword.getValue()) + "\",";
    configJson += "\"spaPollFrequency\":" + String(_config->SpaPollFrequency.getValue());
    configJson += "}";
    return configJson;
}

String WebUI::jsonEscape(const String& value) const {
    String escaped;
    escaped.reserve(value.length() + 8);

    for (size_t i = 0; i < value.length(); ++i) {
        const char c = value.charAt(i);
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }

    return escaped;
}

String WebUI::urlDecode(const String& value) const {
    String decoded;
    decoded.reserve(value.length());

    for (size_t i = 0; i < value.length(); ++i) {
        const char c = value.charAt(i);
        if (c == '+') {
            decoded += ' ';
        } else if (c == '%' && i + 2 < value.length()) {
            const char high = value.charAt(i + 1);
            const char low = value.charAt(i + 2);
            auto hexValue = [](char digit) -> int {
                if (digit >= '0' && digit <= '9') return digit - '0';
                if (digit >= 'A' && digit <= 'F') return digit - 'A' + 10;
                if (digit >= 'a' && digit <= 'f') return digit - 'a' + 10;
                return -1;
            };
            const int highValue = hexValue(high);
            const int lowValue = hexValue(low);
            if (highValue >= 0 && lowValue >= 0) {
                decoded += static_cast<char>((highValue << 4) | lowValue);
                i += 2;
            } else {
                decoded += c;
            }
        } else {
            decoded += c;
        }
    }

    return decoded;
}

bool WebUI::applyFormEncodedConfig(const String& formData) {
    bool softAPAlwaysOn = false;
    size_t start = 0;

    while (start <= formData.length()) {
        const int ampersand = formData.indexOf('&', start);
        const size_t end = ampersand < 0 ? formData.length() : static_cast<size_t>(ampersand);
        const String field = formData.substring(start, end);
        const int separator = field.indexOf('=');

        if (separator >= 0) {
            const String name = urlDecode(field.substring(0, separator));
            const String value = urlDecode(field.substring(separator + 1));

            if (name == "spaName") _config->SpaName.setValue(value);
            else if (name == "softAPPassword") _config->SoftAPPassword.setValue(value);
            else if (name == "softAPAlwaysOn") softAPAlwaysOn = true;
            else if (name == "mqttServer") _config->MqttServer.setValue(value);
            else if (name == "mqttPort") _config->MqttPort.setValue(value.toInt());
            else if (name == "mqttUsername") _config->MqttUsername.setValue(value);
            else if (name == "mqttPassword") _config->MqttPassword.setValue(value);
            else if (name == "spaPollFrequency") _config->SpaPollFrequency.setValue(value.toInt());
        }

        if (ampersand < 0) break;
        start = end + 1;
    }

    _config->SoftAPAlwaysOn.setValue(softAPAlwaysOn);
    _config->writeConfig();
    return true;
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
    debugD("handleDebugWebSocketEvent: type=%d, client ip=%s, client id=%u, data len=%zu", type, client->remoteIP().toString().c_str(), client->id(), len);
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
            "level error, level any, silence, reboot"
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

    if (normalisedCommand == "silence" || normalisedCommand == "s") {
        /*
        * Send the acknowledgement before silencing.
        */
        if (Debug.isWebSilenced()) {
            client->text("WebSocket logging resumed");
        } else {
            client->text("WebSocket logging silenced");
        }
        Debug.setWebSilenced(!Debug.isWebSilenced());
        return true;
    }

    if (normalisedCommand == "reboot") {
        client->text("Rebooting ESP32...");
        delay(200);
        ESP.restart();
        return true;
    }

    if (normalisedCommand.startsWith("level ") || normalisedCommand.length() == 1) {
        String requestedLevel;
        if (normalisedCommand.length() > 1) {
            requestedLevel = normalisedCommand.substring(6);

            requestedLevel.trim();
        } else {
            requestedLevel = normalisedCommand;
        }

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
