#ifndef WEB_REMOTE_DEBUG_H
#define WEB_REMOTE_DEBUG_H

#include <Arduino.h>
#include <Print.h>
#include <RemoteDebug.h>
#include <ESPAsyncWebServer.h>

class WebRemoteDebug : public Print {
public:
    static constexpr uint8_t PROFILER = RemoteDebug::PROFILER;
    static constexpr uint8_t VERBOSE  = RemoteDebug::VERBOSE;
    static constexpr uint8_t DEBUG    = RemoteDebug::DEBUG;
    static constexpr uint8_t INFO     = RemoteDebug::INFO;
    static constexpr uint8_t WARNING  = RemoteDebug::WARNING;
    static constexpr uint8_t ERROR    = RemoteDebug::ERROR;
    static constexpr uint8_t ANY      = RemoteDebug::ANY;

    WebRemoteDebug() = default;

    bool begin(
        const String& hostName,
        uint8_t startingDebugLevel = RemoteDebug::DEBUG
    ) {
        _webLevel = startingDebugLevel;
        return _remote.begin(hostName, startingDebugLevel);
    }

    bool begin(
        const String& hostName,
        uint16_t telnetPort,
        uint8_t startingDebugLevel = RemoteDebug::DEBUG
    ) {
        _webLevel = startingDebugLevel;
        return _remote.begin(
            hostName,
            telnetPort,
            startingDebugLevel
        );
    }

    /**
     * Attach an AsyncWebSocket owned elsewhere, normally by WebUI.
     *
     * The socket must remain alive for as long as this wrapper uses it.
     */
    void attachWebSocket(AsyncWebSocket* webSocket) {
        _webSocket = webSocket;

        /*
         * Reserve these buffers once to reduce repeated heap allocations
         * and fragmentation during bursts of verbose logging.
         */
        _webLine.reserve(MAX_WEB_LINE_LENGTH);
        _webBatch.reserve(MAX_WEB_BATCH_LENGTH);
    }

    void detachWebSocket() {
        flushWebSocketLine();
        sendPendingWebBatch(true);

        _webSocket = nullptr;
        clearWebBuffers();
    }

    /**
     * Call regularly from loop().
     */
    void handle() {
        _remote.handle();

        if (_webSocket == nullptr) {
            return;
        }

        sendPendingWebBatch();

        const uint32_t now = millis();

        if (now - _lastCleanup >= CLEANUP_INTERVAL_MS) {
            _lastCleanup = now;
            _webSocket->cleanupClients();
        }
    }

    /**
     * RemoteDebug's debug macros call isActive(level) before printf().
     *
     * The result is remembered so the following write() calls can route
     * the formatted output independently to RemoteDebug and WebSocket.
     */
    bool isActive(uint8_t level = RemoteDebug::DEBUG) {
        _levelCheckPending = true;
        _currentLevel = level;

        _currentRemoteActive = _remote.isActive(level);
        _currentWebActive =
            hasWebClients() &&
            levelIsEnabled(level, _webLevel) &&
            !_webSilenced;

        return _currentRemoteActive || _currentWebActive;
    }

    size_t write(uint8_t value) override {
        return write(&value, 1);
    }

    size_t write(
        const uint8_t* buffer,
        size_t size
    ) override {
        if (buffer == nullptr || size == 0) {
            return 0;
        }

        /*
         * Direct calls such as Debug.println() do not call isActive() first,
         * so they are treated as unconditional output.
         */
        const bool sendRemote = _levelCheckPending
            ? _currentRemoteActive
            : true;

        const bool sendWeb = _levelCheckPending
            ? _currentWebActive
            : (hasWebClients() && !_webSilenced);

        if (sendRemote) {
            _remote.write(buffer, size);
        }

        if (sendWeb) {
            appendWebSocketData(buffer, size);
        }

        /*
         * A newline completes the current logging operation and resets the
         * routing state for the next message.
         */
        for (size_t i = 0; i < size; ++i) {
            if (buffer[i] == '\n') {
                resetRoutingState();
            }
        }

        return size;
    }

    void flush() override {
        flushWebSocketLine();
        sendPendingWebBatch(true);
        _remote.flush();
    }

    void setWebDebugLevel(uint8_t level) {
        if (level <= RemoteDebug::ANY) {
            _webLevel = level;
        }
    }

    uint8_t getWebDebugLevel() const {
        return _webLevel;
    }

    void setWebSilenced(bool silenced) {
        _webSilenced = silenced;

        if (silenced) {
            clearWebBuffers();
        }
    }

    bool isWebSilenced() const {
        return _webSilenced;
    }

    size_t webClientCount() const {
        if (_webSocket == nullptr) {
            return 0;
        }

        return _webSocket->count();
    }

    bool hasWebClients() const {
        return webClientCount() > 0;
    }

    size_t webBatchLength() const {
        return _webBatch.length();
    }

    size_t webBatchCapacity() const {
        return MAX_WEB_BATCH_LENGTH;
    }

    uint32_t droppedWebLines() const {
        return _droppedWebLines;
    }

    uint32_t droppedWebBytes() const {
        return _droppedWebBytes;
    }

    /**
     * Send a message to all connected WebSocket clients.
     *
     * This bypasses the debug-level filter, but still respects WebSocket
     * silencing.
     */
    void sendWebMessage(const String& message) {
        if (
            _webSocket == nullptr ||
            !hasWebClients() ||
            _webSilenced
        ) {
            return;
        }

        _webSocket->textAll(message);
    }

    /**
     * Send a message to one WebSocket client.
     *
     * This is intended for command acknowledgements and therefore does not
     * respect global WebSocket silencing.
     */
    void sendWebMessage(
        AsyncWebSocketClient* client,
        const String& message
    ) {
        if (client != nullptr) {
            client->text(message);
        }
    }

    /*
     * Common RemoteDebug API forwarding methods.
     */

    void setPassword(const String& password) {
        _remote.setPassword(password);
    }

    void setSerialEnabled(bool enabled) {
        _remote.setSerialEnabled(enabled);
    }

    void setResetCmdEnabled(bool enabled) {
        _remote.setResetCmdEnabled(enabled);
    }

    void setHelpProjectsCmds(const String& help) {
        _remote.setHelpProjectsCmds(help);
    }

    void setCallBackProjectCmds(void (*callback)()) {
        _remote.setCallBackProjectCmds(callback);
    }

    void setCallBackNewClient(void (*callback)()) {
        _remote.setCallBackNewClient(callback);
    }

    String getLastCommand() {
        return _remote.getLastCommand();
    }

    void clearLastCommand() {
        _remote.clearLastCommand();
    }

    void showTime(bool enabled) {
        _remote.showTime(enabled);
    }

    void showProfiler(
        bool enabled,
        uint32_t minimumTime = 0
    ) {
        _remote.showProfiler(enabled, minimumTime);
    }

    void showDebugLevel(bool enabled) {
        _remote.showDebugLevel(enabled);
    }

    void showColors(bool enabled) {
        _remote.showColors(enabled);
    }

    void showRaw(bool enabled) {
        _remote.showRaw(enabled);
    }

    void setFilter(const String& filter) {
        _remote.setFilter(filter);
    }

    void setNoFilter() {
        _remote.setNoFilter();
    }

    void silence(
        bool enabled,
        bool showMessage = true,
        bool fromBreak = false,
        uint32_t timeout = 0
    ) {
        _remote.silence(
            enabled,
            showMessage,
            fromBreak,
            timeout
        );

        setWebSilenced(enabled);
    }

    bool isSilence() {
        return _remote.isSilence() && _webSilenced;
    }

    bool isConnected() {
        return _remote.isConnected() || hasWebClients();
    }

    void disconnect(bool onlyTelnetClient = false) {
        _remote.disconnect(onlyTelnetClient);

        if (
            !onlyTelnetClient &&
            _webSocket != nullptr
        ) {
            _webSocket->closeAll();
            clearWebBuffers();
        }
    }

    void stop() {
        flushWebSocketLine();
        sendPendingWebBatch(true);

        if (_webSocket != nullptr) {
            _webSocket->closeAll();
        }

        clearWebBuffers();
        _remote.stop();
    }

    RemoteDebug& remote() {
        return _remote;
    }

    const RemoteDebug& remote() const {
        return _remote;
    }

    static int parseLevel(String level) {
        level.trim();
        level.toLowerCase();

        if (level == "profiler") {
            return RemoteDebug::PROFILER;
        }

        if (level == "verbose" || level == "v") {
            return RemoteDebug::VERBOSE;
        }

        if (level == "debug" || level == "d") {
            return RemoteDebug::DEBUG;
        }

        if (level == "info" || level == "i") {
            return RemoteDebug::INFO;
        }

        if (level == "warning" || level == "warn" || level == "w") {
            return RemoteDebug::WARNING;
        }

        if (level == "error" || level == "e") {
            return RemoteDebug::ERROR;
        }

        if (level == "any" || level == "a") {
            return RemoteDebug::ANY;
        }

        return -1;
    }

    static const char* levelName(uint8_t level) {
        switch (level) {
            case RemoteDebug::PROFILER:
                return "profiler";

            case RemoteDebug::VERBOSE:
                return "verbose";

            case RemoteDebug::DEBUG:
                return "debug";

            case RemoteDebug::INFO:
                return "info";

            case RemoteDebug::WARNING:
                return "warning";

            case RemoteDebug::ERROR:
                return "error";

            case RemoteDebug::ANY:
                return "any";

            default:
                return "unknown";
        }
    }

    void showWebDebugLevel(bool enabled) {
        _webShowDebugLevel = enabled;
    }

    void showWebProfiler(bool enabled) {
        _webShowProfiler = enabled;

        /*
        * Restart the elapsed-time measurement when enabling it.
        */
        if (enabled) {
            _lastWebLogTime = millis();
        }
    }

private:
    static constexpr size_t MAX_WEB_LINE_LENGTH = 1024;
    static constexpr size_t MAX_WEB_BATCH_LENGTH = 16384;

    static constexpr uint32_t WEB_BATCH_INTERVAL_MS = 25;
    static constexpr uint32_t CLEANUP_INTERVAL_MS = 1000;

    RemoteDebug _remote;
    AsyncWebSocket* _webSocket = nullptr;

    uint8_t _webLevel = RemoteDebug::DEBUG;

    bool _webSilenced = false;
    bool _levelCheckPending = false;
    bool _currentRemoteActive = false;
    bool _currentWebActive = false;

    String _webLine;
    String _webBatch;

    uint32_t _lastWebBatchSend = 0;
    uint32_t _lastCleanup = 0;

    uint32_t _droppedWebLines = 0;
    uint32_t _droppedWebBytes = 0;

    uint8_t _currentLevel = RemoteDebug::ANY;

    uint32_t _lastWebLogTime = 0;

    bool _webShowDebugLevel = true;
    bool _webShowProfiler = true;

    void appendWebSocketData(
        const uint8_t* buffer,
        size_t size
    ) {
        for (size_t i = 0; i < size; ++i) {
            const char character =
                static_cast<char>(buffer[i]);

            if (character == '\r') {
                continue;
            }

            if (character == '\n') {
                flushWebSocketLine();
                continue;
            }

            /*
             * Bound individual line growth. If one log line exceeds the
             * limit, flush it as a partial line and continue.
             */
            if (_webLine.length() >= MAX_WEB_LINE_LENGTH) {
                flushWebSocketLine();
            }

            _webLine += character;
        }
    }

    void flushWebSocketLine() {
        if (_webLine.isEmpty()) {
            return;
        }

        String formattedLine;
        formattedLine.reserve(
            _webLine.length() + 24
        );

        formattedLine = makeWebPrefix(_currentLevel);
        formattedLine += _webLine;

        const size_t lineLength =
            formattedLine.length() + 1;

        const size_t requiredLength =
            _webBatch.length() + lineLength;

        if (requiredLength <= MAX_WEB_BATCH_LENGTH) {
            _webBatch += formattedLine;
            _webBatch += '\n';
        } else {
            ++_droppedWebLines;
            _droppedWebBytes += lineLength;
        }

        _webLine = "";
    }

    void sendPendingWebBatch(bool force = false) {
        if (_webSocket == nullptr) {
            clearWebBuffers();
            return;
        }

        if (!hasWebClients() || _webSilenced) {
            clearWebBuffers();
            return;
        }

        const uint32_t now = millis();

        if (
            !force &&
            now - _lastWebBatchSend < WEB_BATCH_INTERVAL_MS
        ) {
            return;
        }

        if (_webBatch.isEmpty() && _droppedWebLines == 0) {
            return;
        }

        /*
         * Send the accumulated log batch first.
         */
        if (!_webBatch.isEmpty()) {
            _webSocket->textAll(_webBatch);
            _webBatch = "";
        }

        /*
         * Report dropped output separately so the warning cannot itself be
         * prevented by a full log batch.
         */
        if (_droppedWebLines > 0) {
            String droppedMessage;
            droppedMessage.reserve(112);

            droppedMessage = "[Web debug dropped ";
            droppedMessage += String(_droppedWebLines);
            droppedMessage += " lines / ";
            droppedMessage += String(_droppedWebBytes);
            droppedMessage +=
                " bytes because the output buffer was full]";

            _webSocket->textAll(droppedMessage);

            _droppedWebLines = 0;
            _droppedWebBytes = 0;
        }

        _lastWebBatchSend = now;
    }

    void clearWebBuffers() {
        _webLine = "";
        _webBatch = "";
        _droppedWebLines = 0;
        _droppedWebBytes = 0;
    }

    void resetRoutingState() {
        _levelCheckPending = false;
        _currentRemoteActive = false;
        _currentWebActive = false;
        _currentLevel = RemoteDebug::ANY;
    }

    static bool levelIsEnabled(
        uint8_t messageLevel,
        uint8_t configuredLevel
    ) {
        /*
         * RemoteDebug treats ANY and ERROR as always visible.
         */
        if (
            messageLevel == RemoteDebug::ANY ||
            messageLevel == RemoteDebug::ERROR
        ) {
            return true;
        }

        if (messageLevel == RemoteDebug::PROFILER) {
            return configuredLevel == RemoteDebug::PROFILER;
        }

        /*
         * RemoteDebug levels become less verbose as the numeric value rises:
         *
         * VERBOSE=1, DEBUG=2, INFO=3, WARNING=4, ERROR=5
         */
        return messageLevel >= configuredLevel;
    }

    static char levelCharacter(uint8_t level) {
        switch (level) {
            case RemoteDebug::PROFILER:
                return 'P';

            case RemoteDebug::VERBOSE:
                return 'V';

            case RemoteDebug::DEBUG:
                return 'D';

            case RemoteDebug::INFO:
                return 'I';

            case RemoteDebug::WARNING:
                return 'W';

            case RemoteDebug::ERROR:
                return 'E';

            case RemoteDebug::ANY:
            default:
                return 'A';
        }
    }

    String makeWebPrefix(uint8_t level) {
        if (!_webShowDebugLevel && !_webShowProfiler) {
            return "";
        }

        String prefix;
        prefix.reserve(24);

        prefix += '(';

        if (_webShowDebugLevel) {
            prefix += levelCharacter(level);
        }

        if (_webShowDebugLevel && _webShowProfiler) {
            prefix += ' ';
        }

        if (_webShowProfiler) {
            const uint32_t now = millis();

            uint32_t elapsed = 0;

            if (_lastWebLogTime != 0) {
                elapsed = now - _lastWebLogTime;
            }

            _lastWebLogTime = now;

            prefix += "p:^";

            /*
            * Match RemoteDebug's four-digit minimum formatting:
            *
            * 1    -> 0001
            * 52   -> 0052
            * 830  -> 0830
            * 1002 -> 1002
            */
            if (elapsed < 1000) {
                prefix += '0';
            }

            if (elapsed < 100) {
                prefix += '0';
            }

            if (elapsed < 10) {
                prefix += '0';
            }

            prefix += String(elapsed);
            prefix += "ms";
        }

        prefix += ") ";

        return prefix;
    }
};

#endif // WEB_REMOTE_DEBUG_H
