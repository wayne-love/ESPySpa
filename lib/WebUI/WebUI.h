#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <Update.h>
#include <ESPAsyncWebServer.h>

#include "SpaInterface.h"
#include "SpaUtils.h"
#include "Config.h"
#include "MQTTClientWrapper.h"

extern WebRemoteDebug Debug;

class WebUI {
    public:
        WebUI(SpaInterface *spa, Config *config, MQTTClientWrapper *mqttClient);

        /// @brief Set the function to be called to start Wi-Fi Manager.
        /// @param f
        void setWifiManagerCallback(void (*f)()) {
          _wifiManagerCallback = f;
        }
        /// @brief Set the function to be called when properties have been updated.
        /// @param f
        void setSpaCallback(void (*f)(const String, const String)) {
          _setSpaCallback = f;
        }
        void begin();

        /// @brief Push the latest spa status to all connected SPA WebSocket clients.
        /// Call this from the main application whenever spa state changes.
        void notifySpaUpdated();

        bool initialised = false;

    private:
        AsyncWebServer server{80};
        SpaInterface *_spa;
        Config *_config;
        MQTTClientWrapper *_mqttClient;
        AsyncWebSocket _appSocket{"/ws"};
        AsyncWebSocket _debugSocket{"/debug/ws"};

        void (*_wifiManagerCallback)() = nullptr;
        void (*_setSpaCallback)(const String, const String) = nullptr;

        const char* getError();

        void configureAppWebSocket();
        void configureDebugWebSocket();

        void handleAppWebSocketEvent(
            AsyncWebSocket* server,
            AsyncWebSocketClient* client,
            AwsEventType type,
            void* arg,
            uint8_t* data,
            size_t len
        );

        void handleAppWebSocketData(
            AsyncWebSocketClient* client,
            AwsFrameInfo* info,
            uint8_t* data,
            size_t len
        );

        void processAppCommand(
            AsyncWebSocketClient* client,
            const String& command
        );

        void sendMessage(
            const String& type,
            const String& dataJson = "null",
            const String& message = "",
            AsyncWebSocketClient* client = nullptr
        );
        bool sendStatus(AsyncWebSocketClient* client = nullptr);
        void sendConfig(AsyncWebSocketClient* client);
        String buildConfigJson() const;
        String jsonEscape(const String& value) const;
        String urlDecode(const String& value) const;
        bool applyFormEncodedConfig(const String& formData);

        void handleDebugWebSocketEvent(
            AsyncWebSocket* server,
            AsyncWebSocketClient* client,
            AwsEventType type,
            void* arg,
            uint8_t* data,
            size_t len
        );

        void handleDebugWebSocketData(
            AsyncWebSocketClient* client,
            AwsFrameInfo* info,
            uint8_t* data,
            size_t len
        );

        bool processDebugCommand(
            const String& command,
            AsyncWebSocketClient* client
        );

        // hard-coded FOTA page in case file system gets wiped
        static constexpr const char *fotaPage PROGMEM = R"(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<title>Firmware Update</title>
<style>
body{font-family:system-ui,sans-serif;max-width:760px;margin:2rem auto;padding:0 1rem}
fieldset{margin:1rem 0;padding:1rem}label{display:block;margin:.75rem 0 .25rem}
button{padding:.6rem 1rem}.status{margin-top:1rem;white-space:pre-wrap}
progress{width:100%;height:1.25rem;margin-top:1rem}
.error{color:#c62828}.success{color:#2e7d32}
</style>
</head>
<body>
<h1>Firmware Update</h1>
<p>Firmware Over The Air - Recovery page.</p>
<form id="uploadForm">
<fieldset>
<legend>Select update images</legend>
<label for="appFile">Application firmware</label>
<input type="file" accept=".bin,.bin.gz" id="appFile">
<label for="fsFile">Filesystem image</label>
<input type="file" accept=".bin,.bin.gz" id="fsFile">
</fieldset>
<button type="submit" id="uploadButton">Upload and reboot</button>
</form>
<progress id="progress" max="100" value="0" hidden></progress>
<div id="status" class="status" role="status" aria-live="polite"></div>
<script>
(() => {
  const form = document.getElementById('uploadForm');
  const button = document.getElementById('uploadButton');
  const progress = document.getElementById('progress');
  const status = document.getElementById('status');

  function setStatus(message, className = '') {
    status.textContent = message;
    status.className = `status ${className}`;
  }

  function upload(file, updateType, fileNumber, fileCount) {
    return new Promise((resolve, reject) => {
      const data = new FormData();
      data.append('updateType', updateType);
      data.append('update', file);

      const request = new XMLHttpRequest();
      request.open('POST', '/fota');
      request.upload.addEventListener('progress', event => {
        if (!event.lengthComputable) return;
        const fileProgress = event.loaded / event.total;
        progress.value = Math.round(((fileNumber - 1 + fileProgress) / fileCount) * 100);
        setStatus(`Uploading ${fileNumber} of ${fileCount}: ${file.name}`);
      });
      request.addEventListener('load', () => {
        if (request.status >= 200 && request.status < 300) resolve();
        else reject(new Error(request.responseText || `HTTP ${request.status}`));
      });
      request.addEventListener('error', () => reject(new Error('Network error during upload')));
      request.send(data);
    });
  }

  form.addEventListener('submit', async event => {
    event.preventDefault();
    const updates = [];
    const appFile = document.getElementById('appFile').files[0];
    const fsFile = document.getElementById('fsFile').files[0];
    if (appFile) updates.push({file: appFile, type: 'application'});
    if (fsFile) updates.push({file: fsFile, type: 'filesystem'});

    if (updates.length === 0) {
      setStatus('Select at least one update image.', 'error');
      return;
    }

    button.disabled = true;
    progress.hidden = false;
    progress.value = 0;
    try {
      for (let index = 0; index < updates.length; index++) {
        await upload(updates[index].file, updates[index].type, index + 1, updates.length);
      }
      progress.value = 100;
    } catch (error) {
      setStatus(`Update failed: ${error.message}`, 'error');
      button.disabled = false;
    }
    setStatus('Update completed. Rebooting...', 'success');
    fetch('/reboot');
    setTimeout(() => location.href = '/', 2000);
  });
})();
</script>
</body>
</html>
)";
};

#endif // WEBUI_H
