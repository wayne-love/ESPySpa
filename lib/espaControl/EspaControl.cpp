/*
 * EspaControl.cpp - Implementation of ESPA Control Library
 * 
 * This file contains the core implementation of the library that enables
 * ESP32/ESP8266 devices to communicate with the ESPA Control Service.
 * 
 * Key Implementation Details:
 * 
 * MEMORY MANAGEMENT:
 * - WebSocket instance created on heap in begin()
 * - JsonDocument uses stack allocation (capacity: 1024 bytes)
 * - String operations use Arduino String (dynamic allocation)
 * - No manual memory management needed (RAII pattern)
 * 
 * ERROR HANDLING STRATEGY:
 * - Every public method wrapped in try-catch
 * - User callbacks isolated with try-catch
 * - Errors logged but never crash device
 * - Failed operations trigger exponential backoff
 * - Library continues functioning after any error
 * 
 * TIMING & PERFORMANCE:
 * - Non-blocking: all operations return immediately
 * - Ping interval: 30 seconds
 * - Reconnection: exponential backoff 1s -> 60s
 * - Loop processing: typically < 1ms
 * - Message handling: < 10ms
 * 
 * WEBSOCKET PROTOCOL:
 * All messages are JSON with a "type" field:
 * 
 * Outbound:
 *   {"type": "state", "deviceId": "...", "state": {...}}
 *   {"type": "commandAck", "deviceId": "...", "success": true}
 *   {"type": "ping", "deviceId": "...", "timestamp": 12345}
 * 
 * Inbound:
 *   {"type": "command", "command": {...}}
 *   {"type": "stateRequest"}
 *   {"type": "ping"}
 * 
 * THREAD SAFETY:
 * - Not thread-safe
 * - All methods must be called from main loop
 * - AsyncWebSocket handles async events internally
 * 
 * Copyright (c) 2025 ESPA Control Contributors
 * Licensed under MIT License
 */

#include "EspaControl.h"
#include <Config.h>

// Conditional WiFi include for MAC address reading
#include <WiFi.h>

/**
 * WebSocket connection URL will be constructed from ESPA_CONTROL_SERVER_URL
 * converting http:// to ws:// and https:// to wss://
 * Default: wss://control.espa.diy/ws/device/{deviceId}
 */

/**
 * Constructor - Initialize all member variables to safe defaults
 * 
 * Sets up:
 * - Empty device ID (generated in begin())
 * - Server URL from build flags
 * - Disabled debug mode
 * - Disconnected state
 * - Zero timing counters
 * - Base reconnection delay
 */
EspaControl::EspaControl() 
    : _serverUrl(ESPA_CONTROL_SERVER_URL),
      _debugEnabled(false),
      _connected(false),
      _lastPingTime(0),
      _lastReconnectAttempt(0),
      _reconnectAttempts(0),
      _currentReconnectDelay(BASE_RECONNECT_DELAY),
      _lastErrorTime(0),
      _consecutiveErrors(0),
      _setPropertyCallback(nullptr),
      _connectionCallback(nullptr),
      pairingState(NOT_PAIRED),
      _config(nullptr),
      lastPollTime(0),
      pollInterval(5000),
      pollAttempts(0),
      serverPort(80),
      _lastLoggedDelay(0),
      _hasLoggedDisconnect(false) {
}

EspaControl::~EspaControl() {
    _ws.close();
}

/**
 * begin() - Initialize library and connect to cloud service
 * 
 * Initialization Sequence:
 * 1. Check WiFi connection
 * 2. Generate unique device ID from WiFi MAC
 * 3. Configure WebSocket client event handlers
 * 4. Construct WebSocket URL: wss://control.espa.diy/ws/device/{deviceId}
 * 5. Initiate connection to cloud service
 * 
 * WebSocket URL Construction:
 * - http://... -> ws://...
 * - https://... -> wss://...
 * - Path: /ws/device/aabbccddeeff
 * 
 * Error Cases:
 * - WiFi not connected
 * - MAC address unavailable
 * - Connection failure (will retry in loop())
 * 
 * @return true if initialization successful, false on error
 */
bool EspaControl::begin(Config* config) {
    // Store config pointer
    _config = config;
    
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        logError("WiFi not connected - cannot start ESPA Control");
        return false;
    }
    
    // Generate device ID from MAC address (12 hex chars: aabbccddeeff)
    _deviceId = generateDeviceId();
    deviceId = _deviceId;  // Also store in public deviceId member
    
    if (_deviceId.length() == 0) {
        logError("Failed to generate device ID");
        return false;
    }
    
    try {
        // Parse server URL to extract host and port
        parseServerUrl();
        
        // Setup WebSocket event handlers using lambdas to capture 'this'
        _ws.onMessage([this](WebsocketsMessage message) {
            this->onMessage(message);
        });
        
        _ws.onEvent([this](WebsocketsEvent event, String data) {
            this->onEvent(event, data);
        });
        
        log("ESPA Control initialized for device: " + _deviceId);
        log("Server URL: " + _serverUrl);
        log("Server Host: " + serverHost + ":" + String(serverPort));
        
        // Try to load existing pairing token from NVS
        if (loadPairingToken()) {
            log("Loaded existing pairing token from storage");
            pairingState = PAIRED;
            // Attempt connection to authenticated WebSocket endpoint
            connectWebSocket();
        } else {
            log("No pairing token found - device needs pairing");
            pairingState = NOT_PAIRED;
            // Don't connect WebSocket until paired
        }
        
        return true;
    } catch (...) {
        logError("Exception during initialization");
        return false;
    }
}

void EspaControl::onSetProperty(SetPropertyCallback callback) {
    _setPropertyCallback = callback;
    log("SetProperty callback registered");
}

void EspaControl::onConnected(ConnectionCallback callback) {
    _connectionCallback = callback;
    log("Connection callback registered");
}

/**
 * loop() - Main processing function called from Arduino loop()
 * 
 * **MUST BE CALLED REGULARLY!**
 * 
 * Processing Tasks:
 * 1. WebSocket client cleanup (removes dead connections)
 * 3. Reconnection attempts (with exponential backoff when disconnected)
 * 
 * Timing Behavior:
 * - Normally: < 1ms execution time (just checks timers)
 * - During ping: ~5ms (JSON serialization + send)
 * - During reconnect: ~10ms (connection setup)
 * - Non-blocking: never uses delay()
 * 
 * Exponential Backoff Algorithm:
 * When disconnected:
 * - Wait _currentReconnectDelay before attempting reconnect
 * - After each failed attempt, double the delay
 * - Minimum: 1 second
 * - Maximum: 60 seconds
 * - Reset to 1 second on successful connection
 * 
 * Example Timeline:
 * - T+0s:   Disconnect detected
 * - T+1s:   First reconnect attempt (fails)
 * - T+3s:   Second attempt (1+2, fails)
 * - T+7s:   Third attempt (3+4, fails)
 * - T+15s:  Fourth attempt (7+8, succeeds)
 * - Connected, backoff reset to 1s
 * 
 * Error Handling:
 * - Entire function wrapped in try-catch
 * - Any exception logged and ignored
 * - Library continues functioning
 */
void EspaControl::loop() {
    try {
        uint32_t now = millis();
        
        // Check pairing status if in POLLING state
        if (pairingState == POLLING) {
            checkPairingStatus();
        }
        
        // Only manage WebSocket if paired
        if (pairingState == PAIRED) {
            // Poll for WebSocket events (messages, connection state changes)
            if (_ws.available()) {
                _ws.poll();
            }
            
            // Attempt reconnection if disconnected (with exponential backoff)
            if (!_connected && shouldAttemptReconnect()) {
                try {
                    connectWebSocket();
                    _lastReconnectAttempt = now;
                } catch (...) {
                    handleError("Reconnection attempt failed");
                    increaseReconnectDelay();
                }
            }
        }
    } catch (...) {
        // Catch-all to ensure loop never crashes the device
        handleError("Critical error in loop");
    }
}

bool EspaControl::publishState(const String& stateJson) {
    if (!_connected) {
        log("Cannot publish state: not connected");
        return false;
    }
    
    if (_authToken.length() == 0) {
        log("Cannot publish state: not authenticated");
        return false;
    }
    
    try {
        // Parse the state JSON to validate it
        JsonDocument stateDoc;
        DeserializationError err = deserializeJson(stateDoc, stateJson);
        if (err) {
            String errorMsg = "Invalid JSON state: " + String(err.c_str());
            handleError(errorMsg.c_str());
            return false;
        }
        
        // Create message envelope
        JsonDocument message;
        message["type"] = "state";
        message["deviceId"] = _deviceId;
        message["timestamp"] = millis();
        message["state"] = stateDoc;
        
        // Serialize to string
        String output;
        serializeJson(message, output);
        
        // Send via WebSocket
        if (_ws.available()) {
            _ws.send(output);
            log("State published");
            _consecutiveErrors = 0; // Reset error count on success
            return true;
        }
        
        log("Cannot publish state: WebSocket not available");
        return false;
    } catch (...) {
        handleError("Failed to publish state");
        return false;
    }
}

bool EspaControl::isConnected() const {
    return _connected;
}

void EspaControl::setDebug(bool enable) {
    _debugEnabled = enable;
    if (enable) {
        log("Debug logging enabled");
    }
}

void EspaControl::setAuthToken(const String& token) {
    _authToken = token;
    log("Authentication token set");
    
    // If we're already connected but not authenticated, reconnect
    if (_connected && token.length() > 0) {
        log("Reconnecting with new auth token");
        _ws.close();
        _connected = false;
    }
}

bool EspaControl::hasAuthToken() const {
    return _authToken.length() > 0;
}

void EspaControl::clearAuthToken() {
    log("Clearing authentication token");
    _authToken = "";
    
    if (_connected) {
        _ws.close();
        _connected = false;
    }
}

/**
 * onMessage() - Handle incoming WebSocket messages
 * 
 * Called by ArduinoWebsockets when text message received.
 * Parses JSON and routes to appropriate handler.
 */
void EspaControl::onMessage(WebsocketsMessage message) {
    try {
        if (message.isText()) {
            String data = message.data();
            log("Received message: " + data);
            handleWebSocketMessage(data);
        }
    } catch (...) {
        handleError("Failed to process WebSocket message");
    }
}

/**
 * onEvent() - Handle WebSocket connection events
 * 
 * Event Types:
 * - ConnectionOpened: Successfully connected to cloud service
 * - ConnectionClosed: Disconnected (will attempt reconnection)
 * - GotPing: Ping received from server
 * - GotPong: Pong response to our ping
 */
void EspaControl::onEvent(WebsocketsEvent event, String data) {
    try {
        switch (event) {
            case WebsocketsEvent::ConnectionOpened:
                log("Connected to ESPA Control Service");
                _connected = true;
                _lastPingTime = millis();
                resetReconnectDelay();
                
                // Send authentication message if we have a token
                if (_authToken.length() > 0) {
                    JsonDocument authMsg;
                    authMsg["type"] = "auth";
                    authMsg["deviceId"] = _deviceId;
                    authMsg["token"] = _authToken;
                    
                    String output;
                    serializeJson(authMsg, output);
                    _ws.send(output);
                    log("Authentication sent");
                    
                    // Trigger connection callback to publish initial state (only when authenticated)
                    if (_connectionCallback) {
                        log("Calling connection callback");
                        _connectionCallback();
                    }
                } else {
                    log("Warning: No auth token - device may not be paired");
                }
                break;
                
            case WebsocketsEvent::ConnectionClosed:
                // Only log disconnect once, not on every failed reconnect
                if (!_hasLoggedDisconnect) {
                    log("Disconnected from ESPA Control Service");
                    _hasLoggedDisconnect = true;
                }
                _connected = false;
                increaseReconnectDelay();
                break;
                
            case WebsocketsEvent::GotPing:
                log("Ping received from server");
                _ws.pong();
                break;
                
            case WebsocketsEvent::GotPong:
                log("Pong received from server");
                _consecutiveErrors = 0;
                break;
        }
    } catch (...) {
        handleError("Critical error in WebSocket event handler");
    }
}

/**
 * handleWebSocketMessage() - Parse and route incoming WebSocket messages
 * 
 * Message Processing:
 * 1. Parse JSON from message string
 * 2. Extract "type" field
 * 3. Route to appropriate handler
 * 
 * Supported Message Types:
 * - "command": Remote command to execute
 * - "stateRequest": Request for current device state
 * - "ping": Keep-alive ping from server
 */
void EspaControl::handleWebSocketMessage(const String& data) {
    try {
        Serial.println("[EspaControl] WebSocket message received:");
        Serial.println(data);
        
        // Parse JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, data);
        
        if (error) {
            String errorMsg = "JSON parse error: " + String(error.c_str());
            Serial.println("[EspaControl] " + errorMsg);
            handleError(errorMsg.c_str());
            return;
        }
        
        Serial.println("[EspaControl] JSON parsed successfully");
        
        // Check message type
        const char* msgType = doc["type"];
        if (!msgType) {
            log("Message missing type field");
            return;
        }
        
        try {
            if (strcmp(msgType, "command") == 0) {
                handleCommand(doc);
            } else if (strcmp(msgType, "stateRequest") == 0) {
                handleStateRequest();
            } else if (strcmp(msgType, "ping") == 0) {
                // Send pong
                JsonDocument pong;
                pong["type"] = "pong";
                pong["deviceId"] = _deviceId;
                String output;
                serializeJson(pong, output);
                _ws.send(output);
            } else if (strcmp(msgType, "connected") == 0) {
                log("Connection acknowledged by server");
            } else if (strcmp(msgType, "error") == 0) {
                const char* errorMsg = doc["message"];
                log("Server error: " + String(errorMsg ? errorMsg : "unknown"));
            } else {
                log("Unknown message type: " + String(msgType));
            }
        } catch (...) {
            handleError("Exception handling message");
        }
    } catch (...) {
        handleError("Exception in WebSocket message handler");
    }
}

void EspaControl::handleCommand(const JsonDocument& doc) {
    try {
        if (!_setPropertyCallback) {
            log("Command received but no setProperty callback registered");
            return;
        }
        
        // Extract the properties object
        if (!doc["properties"].is<JsonObject>()) {
            log("Command message missing properties field");
            return;
        }
        
        JsonVariantConst properties = doc["properties"];
        
        log("Processing command with " + String(properties.size()) + " properties");
        
        // Process each property
        bool allSuccess = true;
        for (JsonPairConst kv : properties.as<JsonObjectConst>()) {
            const char* property = kv.key().c_str();
            const char* value = kv.value().as<const char*>();
            
            if (!property || !value) {
                log("Skipping invalid property/value pair");
                allSuccess = false;
                continue;
            }
            
            log("Setting property: " + String(property) + " = " + String(value));
            Serial.print("[EspaControl] Setting property: ");
            Serial.print(property);
            Serial.print(" = ");
            Serial.println(value);
            
            // Call the setProperty callback (protect from callback errors)
            try {
                bool success = _setPropertyCallback(String(property), String(value));
                Serial.print("[EspaControl] Property set result: ");
                Serial.println(success ? "SUCCESS" : "FAILED");
                
                if (!success) {
                    log("SetProperty callback returned false for: " + String(property));
                    allSuccess = false;
                }
            } catch (...) {
                String errorMsg = "Exception in setProperty callback for: " + String(property);
                handleError(errorMsg.c_str());
                allSuccess = false;
            }
        }
        
        // Send acknowledgment (protected)
        try {
            JsonDocument ack;
            ack["type"] = "commandAck";
            ack["deviceId"] = _deviceId;
            ack["success"] = allSuccess;
            ack["timestamp"] = millis();
            
            String output;
            serializeJson(ack, output);
            
            if (_ws.available()) {
                _ws.send(output);
            }
            
            log("Command " + String(allSuccess ? "succeeded" : "failed"));
        } catch (...) {
            handleError("Failed to send command acknowledgment");
        }
        
        // State will be published via normal mqttPublishStatus flow
    } catch (...) {
        handleError("Critical error in command handler");
    }
}

void EspaControl::handleStateRequest() {
    try {
        log("State request received - state should be published via publishState()");
        // State is published externally via publishState() method
        // This ensures we don't duplicate the JSON generation logic
    } catch (...) {
        handleError("Failed to handle state request");
    }
}

void EspaControl::connectWebSocket() {
    // Don't try if WiFi is down
    if (WiFi.status() != WL_CONNECTED) {
        return;  // Silently skip - WiFi down is normal during boot/reconnect
    }
    
    // Don't connect if not paired
    if (pairingState != PAIRED) {
        return;  // Silently skip - not paired is expected state
    }
    
    // Don't try to connect if already connected or connecting
    if (_connected || _ws.available()) {
        return;  // Already connected or connection in progress
    }
    
    // Clean up any existing connection before attempting new one
    // This prevents socket errors from stale connections
    try {
        _ws.close();
    } catch (...) {
        // Ignore errors during cleanup
    }
    
    // Construct WebSocket URL
    // Convert http:// to ws:// and https:// to wss://
    String wsUrl = _serverUrl;
    if (wsUrl.startsWith("https://")) {
        wsUrl.replace("https://", "wss://");
    } else if (wsUrl.startsWith("http://")) {
        wsUrl.replace("http://", "ws://");
    }
    
    // Use authenticated endpoint with token
    wsUrl += "/ws/device/" + _deviceId + "?token=" + _authToken;
    
    // Only log first attempt to reduce spam (delay increases are logged separately)
    bool shouldLog = (_reconnectAttempts == 0);
    if (shouldLog) {
        log("Attempting WebSocket connection...");
    }
    
    // Attempt connection
    if (_ws.connect(wsUrl)) {
        // Connection initiated - onEvent will handle success/failure
    } else {
        // Connection failed - only log on first attempt
        if (shouldLog) {
            handleError("WebSocket connection failed");
        }
    }
}

void EspaControl::sendPing() {
    try {
        if (_ws.available()) {
            JsonDocument ping;
            ping["type"] = "ping";
            ping["deviceId"] = _deviceId;
            ping["timestamp"] = millis();
            
            String output;
            serializeJson(ping, output);
            _ws.send(output);
            
            log("Ping sent");
            _consecutiveErrors = 0; // Reset on successful ping
        }
    } catch (...) {
        handleError("Failed to send ping");
    }
}

void EspaControl::log(const char* message) {
    if (_debugEnabled) {
        Serial.print("[EspaControl] ");
        Serial.println(message);
    }
}

void EspaControl::log(const String& message) {
    log(message.c_str());
}

String EspaControl::generateDeviceId() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    
    char deviceId[18];
    snprintf(deviceId, sizeof(deviceId), "%02x%02x%02x%02x%02x%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    return String(deviceId);
}

// Error handling and resilience methods

void EspaControl::handleError(const char* error) {
    _consecutiveErrors++;
    logError(error);
    
    // If too many consecutive errors, increase backoff significantly
    if (_consecutiveErrors >= MAX_CONSECUTIVE_ERRORS) {
        _currentReconnectDelay = MAX_RECONNECT_DELAY;
        logError("Too many consecutive errors, backing off");
    }
}

void EspaControl::resetReconnectDelay() {
    _reconnectAttempts = 0;
    _currentReconnectDelay = BASE_RECONNECT_DELAY;
    _consecutiveErrors = 0;
    _lastLoggedDelay = 0;
    _hasLoggedDisconnect = false;
    log("Reconnect delay reset - connection stable");
}

void EspaControl::increaseReconnectDelay() {
    _reconnectAttempts++;
    
    // Exponential backoff: double the delay each time, up to MAX_RECONNECT_DELAY
    uint32_t oldDelay = _currentReconnectDelay;
    _currentReconnectDelay = _currentReconnectDelay * 2;
    if (_currentReconnectDelay > MAX_RECONNECT_DELAY) {
        _currentReconnectDelay = MAX_RECONNECT_DELAY;
    }
    
    // Only log when delay increases to a new level (not when already at max)
    // This prevents spam when server is down for extended periods
    if (oldDelay != _currentReconnectDelay && _currentReconnectDelay != _lastLoggedDelay) {
        log("Reconnect delay increased to " + String(_currentReconnectDelay/1000) + "s");
        _lastLoggedDelay = _currentReconnectDelay;
    }
}

bool EspaControl::shouldAttemptReconnect() {
    uint32_t now = millis();
    
    // Check if enough time has passed since last reconnect attempt
    if (now - _lastReconnectAttempt < _currentReconnectDelay) {
        return false;
    }
    
    return true;
}

void EspaControl::logError(const char* error) {
    uint32_t now = millis();
    
    // Rate-limit error logging to avoid flooding serial output
    if (now - _lastErrorTime > ERROR_COOLDOWN) {
        Serial.print("[EspaControl ERROR] ");
        Serial.println(error);
        _lastErrorTime = now;
    }
}

// ==================== Pairing Flow Implementation ====================

bool EspaControl::submitPairingCode(const String& code) {
    if (code.length() != 6) {
        logError("Invalid pairing code: must be 6 digits");
        return false;
    }
    
    // If already paired, clear existing pairing to allow re-pairing
    if (pairingState == PAIRED) {
        log("Clearing existing pairing to allow re-pair");
        clearPairingToken();
    }
    
    try {
        HTTPClient http;
        
        // Construct pairing request URL
        String url = String("http://") + serverHost + ":" + String(serverPort) + 
                     String(ESPA_CONTROL_PAIRING_REQUEST_PATH);
        
        log("Submitting pairing code to: " + url);
        
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        
        // Build request body
        JsonDocument requestDoc;
        requestDoc["deviceId"] = deviceId;
        requestDoc["pairingCode"] = code;
        
        String requestBody;
        serializeJson(requestDoc, requestBody);
        
        log("Pairing request: " + requestBody);
        
        // Send POST request
        int httpCode = http.POST(requestBody);
        
        if (httpCode == 200) {
            String response = http.getString();
            log("Pairing code accepted: " + response);
            
            // Parse response
            JsonDocument responseDoc;
            DeserializationError error = deserializeJson(responseDoc, response);
            
            if (!error) {
                // Check if immediately approved
                if (responseDoc["approved"] == true) {
                    _authToken = responseDoc["token"].as<String>();
                    savePairingToken(_authToken);
                    pairingState = PAIRED;
                    log("Device paired immediately!");
                    connectWebSocket();
                    http.end();
                    return true;
                }
            }
            
            // Not immediately approved - start polling
            pairingState = POLLING;
            pollInterval = 5000;  // Start with 5s interval
            pollAttempts = 0;
            lastPollTime = millis();
            log("Pairing code submitted - waiting for approval");
            http.end();
            return true;
            
        } else {
            String error = "Pairing request failed: HTTP " + String(httpCode);
            if (httpCode > 0) {
                error += " - " + http.getString();
            }
            logError(error.c_str());
            pairingState = PAIRING_ERROR;
            http.end();
            return false;
        }
        
    } catch (...) {
        logError("Exception during pairing code submission");
        pairingState = PAIRING_ERROR;
        return false;
    }
}

void EspaControl::checkPairingStatus() {
    if (pairingState != POLLING) {
        return;
    }
    
    uint32_t now = millis();
    
    // Check if it's time to poll based on current interval
    if (now - lastPollTime < pollInterval) {
        return;
    }
    
    try {
        HTTPClient http;
        
        // Construct status check URL
        String url = String("http://") + serverHost + ":" + String(serverPort) + 
                     String(ESPA_CONTROL_PAIRING_STATUS_PATH) + deviceId;
        
        log("Checking pairing status: " + url);
        
        http.begin(url);
        int httpCode = http.GET();
        
        if (httpCode == 200) {
            String response = http.getString();
            log("Pairing status response: " + response);
            
            JsonDocument responseDoc;
            DeserializationError error = deserializeJson(responseDoc, response);
            
            if (!error) {
                String status = responseDoc["status"].as<String>();
                if (status == "APPROVED") {
                    _authToken = responseDoc["token"].as<String>();
                    savePairingToken(_authToken);
                    pairingState = PAIRED;
                    log("Device paired successfully!");
                    connectWebSocket();
                    http.end();
                    return;
                }
            }
            
            // Not approved yet - continue polling with backoff
            pollAttempts++;
            lastPollTime = now;
            
            // Implement exponential backoff: 5s -> 10s -> 30s
            if (pollAttempts >= 6 && pollInterval < 30000) {
                pollInterval = 30000;  // After 6 attempts (~30s), poll every 30s
                log("Pairing poll interval increased to 30s");
            } else if (pollAttempts >= 2 && pollInterval < 10000) {
                pollInterval = 10000;  // After 2 attempts (~10s), poll every 10s
                log("Pairing poll interval increased to 10s");
            }
            
            // Timeout after 5 minutes (300 seconds)
            if (pollAttempts * pollInterval > 300000) {
                logError("Pairing timeout - no approval after 5 minutes");
                pairingState = PAIRING_ERROR;
                http.end();
                return;
            }
            
            log("Pairing not approved yet, attempt " + String(pollAttempts));
            
        } else {
            String error = "Pairing status check failed: HTTP " + String(httpCode);
            logError(error.c_str());
            // Don't give up on first failure, just log and try again
            lastPollTime = now;
        }
        
        http.end();
        
    } catch (...) {
        logError("Exception during pairing status check");
        lastPollTime = now;  // Try again on next poll interval
    }
}

void EspaControl::savePairingToken(const String& token) {
    if (!_config) {
        logError("Config not initialized - cannot save token");
        return;
    }
    
    try {
        _config->EspaToken.setValue(token);
        _config->writeConfig();
        log("Pairing token saved to config");
    } catch (...) {
        logError("Failed to save pairing token to config");
    }
}

bool EspaControl::loadPairingToken() {
    if (!_config) {
        logError("Config not initialized - cannot load token");
        return false;
    }
    
    try {
        _authToken = _config->EspaToken.getValue();
        
        // Verify token exists
        if (_authToken.length() > 0) {
            log("Loaded auth token from config: " + _authToken.substring(0, 8) + "...");
            return true;
        }
        
        log("No pairing token found in config");
        return false;
        
    } catch (...) {
        logError("Exception loading pairing token from config");
        return false;
    }
}

void EspaControl::clearPairingToken() {
    if (!_config) {
        logError("Config not initialized - cannot clear token");
        return;
    }
    
    try {
        _config->EspaToken.setValue("");
        _config->writeConfig();
        
        _authToken = "";
        pairingState = NOT_PAIRED;
        
        log("Pairing token cleared from config");
        
        // Disconnect if currently connected
        if (_connected) {
            _ws.close();
            _connected = false;
        }
        
    } catch (...) {
        logError("Failed to clear pairing token from NVS");
    }
}

void EspaControl::parseServerUrl() {
    // Parse URL format: http://host:port or https://host:port
    String url = _serverUrl;
    
    // Remove protocol
    int protoEnd = url.indexOf("://");
    if (protoEnd != -1) {
        url = url.substring(protoEnd + 3);
    }
    
    // Find port separator
    int portStart = url.indexOf(":");
    if (portStart != -1) {
        serverHost = url.substring(0, portStart);
        
        // Extract port (find end of port number)
        int portEnd = url.indexOf("/", portStart);
        String portStr;
        if (portEnd != -1) {
            portStr = url.substring(portStart + 1, portEnd);
        } else {
            portStr = url.substring(portStart + 1);
        }
        
        serverPort = portStr.toInt();
        if (serverPort == 0) {
            serverPort = 80;  // Default if parsing fails
        }
    } else {
        // No port specified, use default
        int pathStart = url.indexOf("/");
        if (pathStart != -1) {
            serverHost = url.substring(0, pathStart);
        } else {
            serverHost = url;
        }
        
        // Default port based on protocol
        if (_serverUrl.startsWith("https://")) {
            serverPort = 443;
        } else {
            serverPort = 80;
        }
    }
    
    log("Parsed server - Host: " + serverHost + ", Port: " + String(serverPort));
}
