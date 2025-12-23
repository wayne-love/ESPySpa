#ifndef ESPA_CONTROL_H
#define ESPA_CONTROL_H

/*
 * EspaControl.h - Header file for ESPA Control Library
 * 
 * This library provides ESP32/ESP8266 devices with remote control capabilities
 * through the ESPA Control Service. It uses WebSocket for bidirectional
 * communication and provides a callback-based interface.
 * 
 * Key Design Principles:
 * - Never crash the device (comprehensive error handling)
 * - Graceful degradation (device works without service)
 * - Automatic recovery (exponential backoff reconnection)
 * - Zero-overhead when disabled (conditional compilation)
 * - Minimal integration (just a few #ifdef blocks needed)
 * 
 * Thread Safety: Not thread-safe. Call all methods from main loop only.
 * Memory Usage: ~2KB RAM for library state + WebSocket buffers
 * 
 * Copyright (c) 2025 ESPA Control Contributors
 * Licensed under MIT License
 */

// Conditional compilation support for testing
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

using namespace websockets;

// Forward declaration
class Config;

// =====================================================================
// eSpa Control Service Configuration
// =====================================================================
// Change these URLs to point to your local development server or production

// Production server (default)
#ifndef ESPA_CONTROL_SERVER_URL
//   #define ESPA_CONTROL_SERVER_URL "https://control.espa.diy"
    #define ESPA_CONTROL_SERVER_URL "http://10.0.0.198:8080"
#endif

// API endpoint paths - used for pairing and device communication
#define ESPA_CONTROL_WS_PATH "/ws/device/"           // WebSocket path: /ws/device/{deviceId}?token={token}
#define ESPA_CONTROL_PAIRING_REQUEST_PATH "/api/device/pairing-request"
#define ESPA_CONTROL_PAIRING_STATUS_PATH "/api/device/pairing-status/"  // Append deviceId

// For local testing, define ESPA_CONTROL_SERVER_URL in your build flags:
// build_flags = -D ESPA_CONTROL_SERVER_URL=\"http://localhost:3000\"
// or
// build_flags = -D ESPA_CONTROL_SERVER_URL=\"http://192.168.1.100:3000\"

/**
 * EspaControl - Main library class for integrating ESP devices with the ESPA Control Servicerol Service
 * 
 * ARCHITECTURE:
 * This library acts as a bridge between your device firmware and the ESPA Control Service.
 * It manages a WebSocket connection for bidirectional communication:
 * 
 *   Device Firmware <---> EspaControl <---> WebSocket <---> ESPA Control Service
 *        ^                                                            |
 *        |                                                            |
 *        +-------- Callbacks for state/commands --------------------+
 * 
 * CALLBACKS:
 * The library uses two callback functions that you provide:
 * 
 * 1. StateCallback: Called when service requests current device state
 *    - Populate a JsonDocument with your device's current status
 *    - Return true if successful, false on error
 *    - Non-blocking: execute quickly and return
 * 
 * 2. CommandCallback: Called when service sends a command to execute
 *    - Parse the JsonDocument to extract command parameters
 *    - Execute the command on your hardware
 *    - Return true if successful, false if command failed
 *    - Non-blocking: don't wait for long operations
 * 
 * ERROR HANDLING:
 * All library operations are wrapped in error handlers. Even if:
 * - Your callbacks throw exceptions
 * - Network fails completely
 * - Service sends malformed data
 * - Memory allocation fails
 * ...your device will continue to operate normally.
 * 
 * CONNECTION MANAGEMENT:
 * The library automatically handles:
 * - Initial connection establishment
 * - Periodic ping/pong keep-alive (every 30 seconds)
 * - Exponential backoff reconnection (1s -> 60s max)
 * - Connection state tracking
 * - Client cleanup
 * 
 * USAGE EXAMPLE:
 *   EspaControl control;
 *   
 *   void setup() {
 *     // Initialize library (generates device ID from MAC)
 *     control.begin(server);
 *     
 *     // Register callbacks
 *     control.onSendState([](JsonDocument& doc) {
 *       doc["power"] = devicePower;
 *       doc["temp"] = currentTemp;
 *       return true;
 *     });
 *     
 *     control.onReceiveCommand([](const JsonDocument& doc) {
 *       if (doc.containsKey("power")) {
 *         devicePower = doc["power"];
 *         digitalWrite(RELAY_PIN, devicePower ? HIGH : LOW);
 *       }
 *       return true;
 *     });
 *   }
 *   
 *   void loop() {
 *     control.loop(); // Must call regularly (handles all async operations)
 *     // ... rest of your code ...
 *   }
 */

// Pairing state enumeration
enum PairingState {
    NOT_PAIRED,       // Device has no auth token
    CODE_SUBMITTED,   // Pairing code submitted, waiting for response
    POLLING,          // Polling for pairing approval
    PAIRED,           // Device is paired and has token
    PAIRING_ERROR     // Error during pairing process
};

class EspaControl {
public:
    /**
     * SetPropertyCallback - Callback function type for setting device properties
     * 
     * This callback is invoked when the service sends commands to change device state.
     * Commands arrive as simple property/value pairs that map directly to your
     * existing setSpaProperty() or similar functions.
     * 
     * COMMAND FORMAT:
     * Commands are sent as JSON objects with property/value pairs:
     * {
     *   "SetTemp": "38.5",
     *   "pump1_state": "ON",
     *   "pump2_speed": "2"
     * }
     * 
     * Your callback receives one property at a time and should:
     * - Apply the property change to your device
     * - Return true if successful, false if failed
     * - Execute quickly (< 100ms recommended)
     * 
     * Example:
     *   control.onSetProperty([](const String& property, const String& value) {
     *     setSpaProperty(property, value);  // Call existing function
     *     return true;
     *   });
     * 
     * Common Properties:
     * - SetTemp: Target temperature (e.g., "38.5")
     * - pump1_state, pump2_state, etc: "ON" or "OFF"
     * - pump1_speed, pump2_speed, etc: Speed level "0"-"5"
     * - blower: "ON" or "OFF"
     * - lights_state: "ON" or "OFF"
     * - lights_brightness: Brightness level "0"-"10"
     * - status_spaMode: Spa mode "NORM", "ECON", etc.
     * 
     * Thread Safety: Called from main loop context only
     * Error Handling: Wrapped in try-catch, exceptions won't crash device
     * 
     * @param property Property name to set
     * @param value Property value as string
     * @return true if property was successfully set, false on error
     */
    typedef std::function<bool(const String& property, const String& value)> SetPropertyCallback;
    
    /**
     * ConnectionCallback - Callback function type for connection events
     * 
     * Called when WebSocket connection is established and ready for communication.
     * Use this to immediately publish device state upon connection.
     * 
     * Example:
     *   control.onConnected([]() {
     *     String json = generateStatusJson();
     *     control.publishState(json);
     *   });
     */
    typedef std::function<void()> ConnectionCallback;


    EspaControl();
    ~EspaControl();

    /**
     * submitPairingCode() - Submit a 6-digit pairing code for device pairing
     * 
     * Call this when user provides a pairing code from the web UI.
     * This initiates the pairing flow by POSTing to /api/device/pairing-request.
     * 
     * @param code 6-digit pairing code from user
     * @return true if submission successful, false on error
     */
    bool submitPairingCode(const String& code);

    /**
     * getPairingState() - Get current pairing state
     * 
     * @return Current PairingState
     */
    PairingState getPairingState() const { return pairingState; }

    /**
     * isPaired() - Check if device is paired
     * 
     * @return true if device has auth token and is paired
     */
    bool isPaired() const { return pairingState == PAIRED && _authToken.length() > 0; }

    /**
     * getDeviceId() - Get the device ID
     * 
     * @return Device ID string (e.g., "ESPA-B43A45B946BC")
     */
    String getDeviceId() const { return deviceId; }

    /**
     * unpair() - Clear pairing and disconnect
     * 
     * Removes stored auth token from NVS and resets pairing state.
     * Disconnects from WebSocket if currently connected.
     * Device returns to NOT_PAIRED state.
     * 
     * Use this when user wants to unpair device or reset pairing.
     */
    void unpair() { clearPairingToken(); }

    /**
     * begin() - Initialize the ESPA Control library
     * 
     * Call this once during setup() after WiFi is connected.
     * 
     * @param config Pointer to Config instance for token persistence
     * 
     * What this method does:
     * 1. Generates unique device ID from WiFi MAC address
     * 2. Configures WebSocket client to connect to cloud service
     * 3. Registers WebSocket event handlers
     * 4. Initiates connection to wss://control.espa.diy/ws/device/{deviceId}
     * 
     * Device ID Generation:
     * Device ID is automatically created from MAC address in format: aabbccddeeff
     * This ensures each device has a unique, stable identifier.
     * 
     * Example:
     *   EspaControl control;
     *   
     *   void setup() {
     *     WiFi.begin(SSID, PASSWORD);
     *     while (WiFi.status() != WL_CONNECTED) delay(100);
     *     
     *     if (control.begin()) {
     *       Serial.println("ESPA Control initialized");
     *     } else {
     *       Serial.println("Initialization failed");
     *       // Device continues to work, just no remote control
     *     }
     *   }
     * 
     * Error Handling:
     * - Returns false if device ID generation fails
     * - Returns false if WiFi not connected
     * - Wrapped in try-catch for safety
     * - Device continues to operate if this fails
     * 
     * Requirements:
     * - WiFi must be connected (not just initialized)
     * - Sufficient heap memory for WebSocket (~2KB)
     * 
     * @return true if initialization successful, false on error
     */
    bool begin(Config* config);

    /**
     * Set the callback for setting device properties
     * This callback will be invoked when commands are received from the service
     * 
     * Example:
     *   control.onSetProperty([](const String& property, const String& value) {
     *     setSpaProperty(property, value);
     *     return true;
     *   });
     * 
     * @param callback Function to call when a property should be set
     */
    void onSetProperty(SetPropertyCallback callback);
    
    /**
     * Set the callback for connection events
     * This callback will be invoked when WebSocket connection is established
     * 
     * Example:
     *   control.onConnected([]() {
     *     String json = generateStatusJson();
     *     control.publishState(json);
     *   });
     * 
     * @param callback Function to call when connection is established
     */
    void onConnected(ConnectionCallback callback);

    /**
     * Publish device state to connected WebSocket clients
     * 
     * Call this whenever your device state changes and you want to notify
     * remote clients. Pass the JSON string that you're already generating
     * for MQTT (or similar).
     * 
     * Example:
     *   String json = generateStatusJson();
     *   control.publishState(json);
     * 
     * @param stateJson JSON string containing complete device state
     * @return true if state was published successfully, false otherwise
     */
    bool publishState(const String& stateJson);

    /**
     * loop() - Main processing function that must be called regularly
     * 
     * **CRITICAL: Call this function in your main loop()!**
     * 
     * This function handles all asynchronous operations:
     * - WebSocket client cleanup
     * - Periodic ping/pong keep-alive (every 30 seconds)
     * - Automatic reconnection with exponential backoff
     * - Connection state management
     * 
     * Call Frequency:
     * - Should be called at least once per loop iteration
     * - Typical call frequency: every 10-50ms
     * - More frequent calls = faster reconnection
     * - Less frequent calls = lower CPU usage
     * 
     * Processing Time:
     * - Normally: < 1ms (just checks timers)
     * - During reconnection: < 10ms
     * - During ping: < 5ms
     * - Non-blocking: never delays your code
     * 
     * Reconnection Logic:
     * When disconnected, automatically attempts reconnection using exponential backoff:
     * - 1st attempt: after 1 second
     * - 2nd attempt: after 2 seconds
     * - 3rd attempt: after 4 seconds
     * - ... doubles each time up to 60 seconds maximum
     * - Resets to 1 second on successful connection
     * 
     * Example Usage:
     *   void loop() {
     *     control.loop(); // Always call this first!
     *     
     *     // Your device logic here
     *     readSensors();
     *     updateOutputs();
     *     handleButtons();
     *     
     *     delay(10);
     *   }
     * 
     * Error Handling:
     * - All operations wrapped in try-catch
     * - Errors logged but never crash device
     * - Failed operations are retried automatically
     * - Library continues functioning even after errors
     * 
     * Thread Safety:
     * - Not thread-safe
     * - Must be called from main loop only
     * - Do not call from ISR or separate task
     */
    void loop();

    /**
     * Check if the library is connected to the control service
     * 
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * Enable or disable debug logging
     * 
     * @param enable true to enable debug output, false to disable
     */
    void setDebug(bool enable);

    /**
     * Set the authentication token received after pairing
     * 
     * This token is sent with the WebSocket connection to authenticate the device.
     * Call this after successful pairing or load from config on startup.
     * 
     * @param token Authentication token from pairing process
     */
    void setAuthToken(const String& token);

    /**
     * Check if device has valid authentication token
     * 
     * @return true if auth token is set, false otherwise
     */
    bool hasAuthToken() const;

    /**
     * Clear authentication token (unpair device)
     * 
     * Disconnects from cloud service and clears stored token.
     * Device will need to be re-paired.
     */
    void clearAuthToken();

private:
    // ==================== WebSocket Management ====================
    
    /**
     * _ws - WebSocket client for connection to cloud service
     * Created in begin(), connects to wss://control.espa.diy/ws/device/{deviceId}
     */
    WebsocketsClient _ws;
    
    /**
     * _serverUrl - Cloud service URL
     * Configurable via ESPA_CONTROL_SERVER_URL build flag
     */
    String _serverUrl;
    
    // ==================== Configuration ====================
    
    /**
     * _deviceId - Unique device identifier (12 hex chars from MAC)
     * Format: aabbccddeeff
     * Generated once in begin(), stable across reboots
     */
    String _deviceId;
    
    /**
     * _authToken - Authentication token from pairing process
     * Sent with WebSocket connection for authentication
     * Empty string if not paired
     */
    String _authToken;
    
    /**
     * _debugEnabled - Flag to enable/disable debug logging to Serial
     * Set via setDebug(). When true, logs all operations.
     */
    bool _debugEnabled;
    
    // ==================== Connection State ====================
    
    /**
     * _connected - Current WebSocket connection state
     * true: At least one client connected
     * false: No clients connected (will attempt reconnection)
     */
    bool _connected;
    
    /**
     * _lastPingTime - Timestamp (millis) of last ping sent
     * Used to trigger periodic keep-alive pings
     */
    uint32_t _lastPingTime;
    
    /**
     * _lastReconnectAttempt - Timestamp (millis) of last reconnection attempt
     * Used with exponential backoff to space out reconnection tries
     */
    uint32_t _lastReconnectAttempt;
    
    /**
     * _reconnectAttempts - Counter of consecutive failed reconnection attempts
     * Resets to 0 on successful connection
     * Used for logging and debugging
     */
    uint32_t _reconnectAttempts;
    
    /**
     * _currentReconnectDelay - Current delay (ms) before next reconnection
     * Starts at BASE_RECONNECT_DELAY (1000ms)
     * Doubles on each failure up to MAX_RECONNECT_DELAY (60000ms)
     * Resets on successful connection
     */
    uint32_t _currentReconnectDelay;
    
    /**
     * _lastErrorTime - Timestamp (millis) of last error log
     * Used for rate-limiting error messages to avoid serial spam
     */
    uint32_t _lastErrorTime;
    
    /**
     * _consecutiveErrors - Counter of consecutive errors
     * Resets on any successful operation
     * After MAX_CONSECUTIVE_ERRORS, backoff increases significantly
     */
    uint32_t _consecutiveErrors;
    
    // ==================== Timing Constants ====================
    
    /** PING_INTERVAL - Time (ms) between keep-alive pings when connected */
    static const uint32_t PING_INTERVAL = 30000;  // 30 seconds
    
    /** BASE_RECONNECT_DELAY - Initial delay (ms) before first reconnection attempt */
    static const uint32_t BASE_RECONNECT_DELAY = 5000;  // 5 seconds
    
    /** MAX_RECONNECT_DELAY - Maximum delay (ms) between reconnection attempts */
    static const uint32_t MAX_RECONNECT_DELAY = 60000;  // 60 seconds
    
    /** ERROR_COOLDOWN - Minimum time (ms) between error log messages */
    static const uint32_t ERROR_COOLDOWN = 10000;  // 10 seconds
    
    /** MAX_CONSECUTIVE_ERRORS - Error count threshold for aggressive backoff */
    static const uint32_t MAX_CONSECUTIVE_ERRORS = 10;
    
    /** Tracking variables for reducing log spam */
    uint32_t _lastLoggedDelay;
    bool _hasLoggedDisconnect;
    
    // ==================== Callbacks ====================
    
    /**
     * _setPropertyCallback - User-provided function to set device properties
     * Registered via onSetProperty()
     * nullptr if not registered
     */
    SetPropertyCallback _setPropertyCallback;
    
    /**
     * _connectionCallback - User-provided function called on connection
     * Registered via onConnected()
     * nullptr if not registered
     */
    ConnectionCallback _connectionCallback;
    
    // ==================== Pairing State ====================
    
    /**
     * pairingState - Current state in the pairing flow
     */
    PairingState pairingState;
    
    /**
     * deviceId - Device ID in format ESPA-{MAC} (e.g., ESPA-B43A45B946BC)
     */
    String deviceId;
    
    /**
     * _config - Pointer to Config instance for persistent token storage
     */
    Config* _config;
    
    /**
     * lastPollTime - Timestamp of last pairing status poll
     */
    unsigned long lastPollTime;
    
    /**
     * pollInterval - Current interval between pairing status polls (ms)
     */
    int pollInterval;
    
    /**
     * pollAttempts - Number of pairing status poll attempts
     */
    int pollAttempts;
    
    /**
     * serverHost - Host portion of server URL (e.g., "10.0.0.198")
     */
    String serverHost;
    
    /**
     * serverPort - Port portion of server URL (e.g., 8080)
     */
    int serverPort;
    
    // ==================== Pairing Flow Management ====================
    
    /**
     * checkPairingStatus() - Poll server for pairing approval status
     * 
     * HTTP GET to /api/device/pairing-status/{deviceId}
     * Response:
     * {
     *   "approved": true,
     *   "token": "auth-token-string"
     * }
     * 
     * Called from loop() when in POLLING state.
     * Implements exponential backoff (5s → 10s → 30s).
     * On approval: saves token to NVS, transitions to PAIRED, connects WebSocket.
     * On timeout: transitions to PAIRING_ERROR.
     */
    void checkPairingStatus();
    
    /**
     * savePairingToken() - Persist auth token to NVS storage
     * 
     * Uses Preferences library to save:
     * - "authToken": authentication token string
     * - "deviceId": device identifier
     * 
     * Namespace: "espaControl"
     * 
     * @param token Authentication token received from server
     */
    void savePairingToken(const String& token);
    
    /**
     * loadPairingToken() - Load auth token from NVS storage
     * 
     * Called during begin() to check for existing pairing.
     * If token found: sets PAIRED state, deviceId, authToken.
     * If not found: sets NOT_PAIRED state.
     * 
     * @return true if token loaded successfully
     */
    bool loadPairingToken();
    
    /**
     * clearPairingToken() - Remove stored auth token from NVS
     * 
     * Called when unpairing device or pairing fails.
     * Clears "authToken" and "deviceId" from Preferences.
     * Transitions to NOT_PAIRED state.
     */
    void clearPairingToken();
    
    /**
     * parseServerUrl() - Extract host and port from server URL
     * 
     * Parses ESPA_CONTROL_SERVER_URL to populate:
     * - serverHost (e.g., "10.0.0.198")
     * - serverPort (e.g., 8080)
     * 
     * Handles http:// and https:// schemes.
     * Called once during begin().
     */
    void parseServerUrl();

    // ==================== WebSocket Management ====================
    
    /**
     * onMessage() - Handle incoming WebSocket message
     * 
     * Called by ArduinoWebsockets library when message received.
     * Parses JSON and routes to appropriate handler.
     * 
     * @param message WebSocket message event
     */
    void onMessage(WebsocketsMessage message);
    
    /**
     * onEvent() - Handle WebSocket connection events
     * 
     * Processes:
     * - ConnectionOpened: Connected to cloud (reset backoff)
     * - ConnectionClosed: Disconnected (start reconnection)
     * - GotPing: Ping received (send pong)
     * - GotPong: Pong received (reset error counter)
     * 
     * @param event WebSocket event type
     * @param data Optional event data
     */
    void onEvent(WebsocketsEvent event, String data);
    
    // ==================== Message Processing ====================
    
    /**
     * handleWebSocketMessage() - Parse and route incoming WebSocket messages
     * 
     * Message Format (JSON):
     * {
     *   "type": "command" | "stateRequest" | "ping",
     *   ... type-specific fields ...
     * }
     * 
     * Processing:
     * 1. Parses JSON (handles malformed JSON gracefully)
     * 2. Routes to appropriate handler based on "type" field
     * 3. Logs unknown message types
     * 
     * Supported Message Types:
     * - "command": Remote command to execute -> handleCommand()
     * - "stateRequest": Request for current state -> handleStateRequest()
     * - "ping": Keep-alive ping -> send pong response
     * 
     * Error Handling:
     * - JSON parse errors: logged, message discarded
     * - Missing type field: logged, message discarded
     * - Unknown type: logged, message discarded
     * - All wrapped in try-catch
     * 
     * @param data Message data string
     */
    void handleWebSocketMessage(const String& data);
    
    /**
     * handleCommand() - Execute property changes from the service
     * 
     * Message Format:
     * {
     *   "type": "command",
     *   "properties": {
     *     "SetTemp": "38.5",
     *     "pump1_state": "ON"
     *   }
     * }
     * 
     * Processing:
     * 1. Validates setProperty callback is registered
     * 2. Extracts "properties" object from message
     * 3. For each property, invokes user's callback (protected by try-catch)
     * 4. Sends acknowledgment to service with success/failure
     * 
     * Command Acknowledgment Format:
     * {
     *   "type": "commandAck",
     *   "deviceId": "aabbccddeeff",
     *   "success": true/false,
     *   "timestamp": 12345
     * }
     * 
     * @param doc Parsed JSON document containing the message
     */
    void handleCommand(const JsonDocument& doc);
    
    /**
     * handleStateRequest() - Handle request for current device state
     * 
     * Simply calls sendState() to transmit current state to service.
     * Wrapped in try-catch for safety.
     */
    void handleStateRequest();
    
    // ==================== Connection Management ====================
    
    /**
     * connectWebSocket() - Placeholder for connection logic
     * 
     * Currently just logs status. Actual connection is managed by
     * AsyncWebSocket library. Future: could trigger manual connection.
     */
    void connectWebSocket();
    
    /**
     * sendPing() - Send keep-alive ping to maintain connection
     * 
     * Ping Message Format:
     * {
     *   "type": "ping",
     *   "deviceId": "aabbccddeeff",
     *   "timestamp": 12345
     * }
     * 
     * Called automatically every PING_INTERVAL (30s) from loop().
     * Resets error counter on success.
     * Wrapped in try-catch.
     */
    void sendPing();
    
    // ==================== Error Handling & Resilience ====================
    
    /**
     * handleError() - Process and log an error
     * 
     * Actions:
     * 1. Increments consecutive error counter
     * 2. Logs error (rate-limited)
     * 3. If errors exceed threshold, max out backoff delay
     * 
     * @param error Error message to log
     */
    void handleError(const char* error);
    
    /**
     * resetReconnectDelay() - Reset exponential backoff to initial values
     * 
     * Called when connection succeeds.
     * Resets:
     * - Reconnection delay to BASE_RECONNECT_DELAY (1s)
     * - Attempt counter to 0
     * - Error counter to 0
     */
    void resetReconnectDelay();
    
    /**
     * increaseReconnectDelay() - Apply exponential backoff
     * 
     * Called when connection/operation fails.
     * Doubles current delay up to MAX_RECONNECT_DELAY (60s).
     * Increments attempt counter.
     */
    void increaseReconnectDelay();
    
    /**
     * shouldAttemptReconnect() - Check if enough time passed for reconnect
     * 
     * Returns true if:
     * - Current time - last attempt >= current delay
     * 
     * Used by loop() to implement exponential backoff.
     * 
     * @return true if reconnection should be attempted now
     */
    bool shouldAttemptReconnect();
    
    // ==================== Utility & Logging ====================
    
    /**
     * log() - Log debug message if debug mode enabled
     * 
     * Format: [EspaControl] message
     * Only outputs if setDebug(true) was called.
     * 
     * @param message C-string message to log
     */
    void log(const char* message);
    
    /**
     * log() - Log debug message (String overload)
     * 
     * @param message Arduino String to log
     */
    void log(const String& message);
    
    /**
     * logError() - Log error with rate limiting
     * 
     * Format: [EspaControl ERROR] message
     * Rate-limited to one message per ERROR_COOLDOWN (5s).
     * Always outputs (regardless of debug setting).
     * 
     * @param error Error message to log
     */
    void logError(const char* error);
    
    /**
     * generateDeviceId() - Create unique device ID from MAC address
     * 
     * Reads WiFi MAC address and converts to 12-character hex string.
     * Format: aabbccddeeff (lowercase, no separators)
     * 
     * Example:
     *   MAC: AA:BB:CC:DD:EE:FF
     *   ID:  aabbccddeeff
     * 
     * Called once during begin().
     * 
     * @return Device ID string, or empty string on error
     */
    String generateDeviceId();
};

#endif // ESPA_CONTROL_H
