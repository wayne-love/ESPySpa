#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <Update.h>

#include "SpaInterface.h"
#include "SpaUtils.h"
#include "Config.h"
#include "MQTTClientWrapper.h"
#include "ESPAsyncWebServer.h"

extern RemoteDebug Debug;

class WebUI {
    public:
        WebUI(SpaInterface *spa, Config *config, MQTTClientWrapper *mqttClient);

        /// @brief Set the function to be called when properties have been updated.
        /// @param f
        void setWifiManagerCallback(void (*f)());
        void begin();
        bool initialised = false;

    private:
        AsyncWebServer server{80};
        SpaInterface *_spa;
        Config *_config;
        MQTTClientWrapper *_mqttClient;
        void (*_wifiManagerCallback)() = nullptr;

        const char* getError();

        // hard-coded FOTA page in case file system gets wiped
        static constexpr const char *fotaPage PROGMEM = R"(
<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<title>Firmware Update</title>
</head>
<body>
<h1>Firmware Update</h1>
<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js"></script>
<form method="POST" action="" enctype="multipart/form-data" id="upload_form">
<table>
<tr>
<td><label for="appFile">Application Update File:</label></td>
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
$(function() {
  $('form').submit(async function(e) {
    e.preventDefault();
    const appFile = $('#appFile')[0].files[0];
    const fsFile = $('#fsFile')[0].files[0];
    let appSuccess = false, fsSuccess = false;

    if (!appFile && !fsFile) {
      msg('Error: Please select either an application or filesystem update file.', 'red');
      console.error('No files selected for upload.');
      return;
    }

    // Upload application file if provided
    if (appFile) {
      const appData = new FormData();
      appData.append('updateType', 'application');
      appData.append('update', appFile);
      appSuccess = await uploadFileAsync(appData, '/fota');
    }

    // Upload filesystem file if provided
    if (fsFile) {
      const fsData = new FormData();
      fsData.append('updateType', 'filesystem');
      fsData.append('update', fsFile);
      fsSuccess = await uploadFileAsync(fsData, '/fota');
    }

    // Trigger reboot only if all provided uploads were successful
    if ((!appFile || appSuccess) && (!fsFile || fsSuccess)) {
      reboot();
    } else {
      msg('One or more uploads failed. Reboot canceled.', 'red');
    }
  });

  async function uploadFileAsync(data, url) {
    return new Promise((resolve) => {
      $.ajax({
        url,
        type: 'POST',
        data,
        contentType: false,
        processData: false,
        xhr: function() {
          const xhr = new XMLHttpRequest();
          xhr.upload.addEventListener('progress', function(e) {
            if (e.lengthComputable) {
              const progress = Math.round((e.loaded / e.total) * 100);
              $('#prg').text('progress: ' + progress + '%');
              msg(progress < 100 ? 'Uploading...' : 'Flashing...', 'blue');
            }
          });
          return xhr;
        },
        success: () => {
          msg('Update successful!', 'green');
          resolve(true);
        },
        error: () => {
          msg('Update failed! Please try again.', 'red');
          resolve(false);
        }
      });
    });
  }

  function reboot() {
    $.ajax({
      url: '/reboot',
      type: 'GET',
      success: () => msg('Reboot initiated.', 'blue'),
      error: () => msg('Failed to initiate reboot.', 'red'),
      complete: () => setTimeout(() => location.href = '/', 2000)
    });
  }

  function msg(message, color) {
    $('#msg').html(`<p style="color:${color};">${message}</p>`);
  }
});
</script>
</body>
</html>
)";
};

#endif // WEBUI_H
