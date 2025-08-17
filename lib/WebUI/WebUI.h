#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <LittleFS.h>
#include <Update.h>

#include "SpaInterface.h"
#include "SpaUtils.h"
#include "Config.h"
#include "MQTTClientWrapper.h"
#include "ESPAsyncWebServer.h"
#include "WiFiTools.h"

extern RemoteDebug Debug;

class WebUI {
    public:
        WebUI(SpaInterface *spa, Config *config, MQTTClientWrapper *mqttClient, WiFiTools *wifiTools) :
            _spa(spa), _config(config), _mqttClient(mqttClient), _wifiTools(wifiTools) {
            if (!LittleFS.begin()) {
                debugE("Failed to mount file system");
            }
        }

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
        bool initialised = false;

    private:
        AsyncWebServer server{80};
        SpaInterface *_spa;
        Config *_config;
        MQTTClientWrapper *_mqttClient;
        WiFiTools *_wifiTools;
        void (*_wifiManagerCallback)() = nullptr;
        void (*_setSpaCallback)(const String, const String) = nullptr;

        const char* getError();

      // hard-coded FOTA page in case file system gets wiped
      static constexpr const char *fotaPage PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<title>Firmware Update</title>
</head>
<body>
<h1>Firmware Update</h1>
<form method="POST" action="" enctype="multipart/form-data" id="upload_form">
<table>
<tr>
<td><label for="appFile">Firmware Update File:</label></td>
<td><input type="file" accept=".bin,.bin.gz" name="appFile" id="appFile"></td>
</tr>
<tr>
<td><label for="fsFile">Filesystem Update File:</label></td>
<td><input type="file" accept=".bin,.bin.gz" name="fsFile" id="fsFile"></td>
</tr>
<tr><td><input type="submit" value="Update"></td><tr>
</table>
</form>
<div id="prg">progress: 0%</div>
<div id="msg"></div>
<script>
document.addEventListener("DOMContentLoaded", () => {
  const form = document.getElementById("upload_form");
  const appFileInput = document.getElementById("appFile");
  const fsFileInput = document.getElementById("fsFile");
  const prg = document.getElementById("prg");
  const msgDiv = document.getElementById("msg");

  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    const appFile = appFileInput.files[0];
    const fsFile = fsFileInput.files[0];
    let appSuccess = false, fsSuccess = false;

    if (!appFile && !fsFile) {
      msg("Error: Please select either an firmware or filesystem update file.", "red");
      console.error("No files selected for upload.");
      return;
    }

    // Upload firmware file if provided
    if (appFile) {
      const appData = new FormData();
      appData.append("updateType", "application");
      appData.append("update", appFile);
      appSuccess = await uploadFileAsync(appData, "/fota");
    }

    // Upload filesystem file if provided
    if (fsFile) {
      const fsData = new FormData();
      fsData.append("updateType", "filesystem");
      fsData.append("update", fsFile);
      fsSuccess = await uploadFileAsync(fsData, "/fota");
    }

    // Trigger reboot only if all provided uploads were successful
    if ((!appFile || appSuccess) && (!fsFile || fsSuccess)) {
      reboot();
    } else {
      msg("One or more uploads failed. Reboot canceled.", "red");
    }
  });

  function uploadFileAsync(data, url) {
    return new Promise((resolve) => {
      const xhr = new XMLHttpRequest();
      xhr.open("POST", url, true);

      xhr.upload.addEventListener("progress", (e) => {
        if (e.lengthComputable) {
          const progress = Math.round((e.loaded / e.total) * 100);
          prg.textContent = "progress: " + progress + "%";
          msg(progress < 100 ? "Uploading..." : "Flashing...", "blue");
        }
      });

      xhr.onload = () => {
        if (xhr.status >= 200 && xhr.status < 300) {
          msg("Update successful!", "green");
          resolve(true);
        } else {
          msg("Update failed! Please try again.", "red");
          resolve(false);
        }
      };

      xhr.onerror = () => {
        msg("Update failed! Please try again.", "red");
        resolve(false);
      };

      xhr.send(data);
    });
  }

  function reboot() {
    fetch("/reboot")
      .then(() => msg("Reboot initiated.", "blue"))
      .catch(() => msg("Failed to initiate reboot.", "red"))
      .finally(() => setTimeout(() => location.href = "/", 2000));
  }

  function msg(message, color) {
    msgDiv.innerHTML = `<p style="color:${color};">${message}</p>`;
  }
});
</script>
</body>
</html>
)rawliteral";
};

#endif // WEBUI_H
