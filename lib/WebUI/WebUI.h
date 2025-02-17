#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <Update.h>

#include "SpaInterface.h"
#include "SpaUtils.h"
#include "Config.h"
#include "MQTTClientWrapper.h"

//define stringify function
#define xstr(a) str(a)
#define str(a) #a

extern RemoteDebug Debug;

class WebUI {
    public:
        std::unique_ptr<WebServer> server;
        WebUI(SpaInterface *spa, Config *config, MQTTClientWrapper *mqttClient);

        /// @brief Set the function to be called when properties have been updated.
        /// @param f
        void setWifiManagerCallback(void (*f)());
        void begin();
        bool initialised = false;

    private:
        SpaInterface *_spa;
        Config *_config;
        MQTTClientWrapper *_mqttClient;
        void (*_wifiManagerCallback)() = nullptr;

        const char* getError();

static constexpr const char *indexPageTemplate PROGMEM =
R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<link rel="stylesheet" href="/styles.css">
<script>
function confirmAction(url) {
    if (confirm('Are you sure?')) {
    window.location.href = url;
    }
}
function fetchStatus() {
  fetch('/json')
    .then(response => response.json())
    .then(value_json => {
      document.getElementById('temperatures_water').innerText = value_json.temperatures.water + "\u00B0C";
      document.getElementById('temperatures_setPoint').value = value_json.temperatures.setPoint;
      document.getElementById('status_state').innerText = value_json.status.state;
      document.getElementById('status_controller').innerText = value_json.status.controller;
      document.getElementById('status_serial').innerText = value_json.status.serial;
      document.getElementById('status_siInitialised').innerText = value_json.status.siInitialised;
      document.getElementById('status_mqtt').innerText = value_json.status.mqtt;
    })
    .catch(error => console.error('Error fetching status:', error));
}
function updateTempSetPoint() {
  const temperatures_setPoint = document.getElementById('temperatures_setPoint').value;
  fetch('/set', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: 'temperatures_setPoint=' + temperatures_setPoint
  })
  .then(response => response.text())
  .then(result => console.log(result))
  .catch(error => console.error('Error setting temperature:', error));
}
function sendCurrentTime() {
  const status_datetime = new Date(Date() + " UTC").toISOString().slice(0, 19).replace("T", " ");
  fetch('/set', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: 'status_datetime=' + status_datetime
  })
  .then(response => response.text())
  .then(result => console.log(result))
  .catch(error => console.error('Error setting temperature:', error));
}
window.onload = function() {
    fetchStatus();
    setInterval(fetchStatus, 10000);
}
</script>
</head>
<body>
<h1>ESP32 Spa Controller</h1>
<table>
<tr><td>Spa status:</td><td><span id="status_state">Loading...</span></td></tr>
<tr><td>Spa temperature:</td><td><span id="temperatures_water">Loading...</span></td></tr>
<tr><td>Set Temperature:</td><td><input type="number" id="temperatures_setPoint" step="0.2" min="10" max="41">
<button onclick="updateTempSetPoint();">Set</button></td></tr>
<tr><td>Spa controller:</td><td><span id="status_controller">Loading...</span></td></tr>
<tr><td>Spa serial number:</td><td><span id="status_serial">Loading...</span></td></tr>
<tr><td>Spa interface initialised:</td><td><span id="status_siInitialised">Loading...</span></td></tr>
<tr><td>MQTT status:</td><td><span id="status_mqtt">Loading...</span></td></tr>
<tr><td>Build:</td><td>)" xstr(BUILD_INFO) R"(</td></tr>
</table>
<p><a href="/json.html">Spa JSON HTML</a></p>
<p><a href="/json">Spa JSON</a></p>
<p><a href="/status">Spa Response</a></p>
<p><a href="#" onclick="sendCurrentTime();">Send Current Time to Spa</a></p>
<p><a href="/config">Configuration</a></p>
<p><a href="/fota">Firmware Update</a></p>
<p><a href="#" onclick="confirmAction('/wifi-manager'); return false;">Wi-Fi Manager</a></p>
<p><a href="#" onclick="confirmAction('/reboot'); return false;">Reboot ESP</a></p>
</body>
</html>)";

static constexpr const char *fotaPage PROGMEM =
R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<link rel="stylesheet" href="/styles.css">
<title>Firmware Update</title>
</head>
<body>
<h1>Firmware Update</h1>
<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
<form method='POST' action='' enctype='multipart/form-data' id='upload_form'>
<label for="updateType">Update Type:</label>
<select name="updateType" id="updateType">
<option value="application">Application (Firmware)</option>
<option value="filesystem">Filesystem</option>
</select>
<br><br>
<div id="fileInputs">
<label for="appFile">Application Update File:</label>
<input type='file' accept='.bin,.bin.gz' name='appFile' id='appFile'>
<br><br>
<label for="fsFile" id="fsFileLabel" style="display: none;">Filesystem Update File:</label>
<input type='file' accept='.bin,.bin.gz' name='fsFile' id='fsFile' style="display: none;">
</div>
<input type='submit' value='Update'>
</form>
<div id='prg'>progress: 0%</div>
<div id='msg'></div>
<script>
$(document).ready(function() {
  $('#updateType').change(function() {
    if ($(this).val() === 'filesystem') {
      $('#fsFileLabel').show();
      $('#fsFile').show();
    } else {
      $('#fsFileLabel').hide();
      $('#fsFile').hide();
      $('#fsFile').val('');
    }
  });
});
$('form').submit(function(e){
  e.preventDefault();

  const updateType = $('#updateType').val();
  const appFile = $('#appFile')[0].files[0];
  const fsFile = $('#fsFile')[0].files[0];

  if (!appFile) {
    $('#msg').html('<p style="color:red;">Please select an application update file.</p>');
    return;
  }

  if (updateType === 'filesystem' && !fsFile) {
    $('#msg').html('<p style="color:red;">Please select a filesystem update file.</p>');
    return;
  }

  var appData = new FormData();
  appData.append('updateType', 'application');
  appData.append('update', appFile);
  uploadFile(appData, '/fota', function() {
    $('#msg').html('<p style="color:blue;">Application uploaded successfully. Now uploading filesystem...</p>');

    if (updateType === 'filesystem') {
      // Upload the filesystem file
      const fsData = new FormData();
      fsData.append('updateType', 'filesystem');
      fsData.append('update', fsFile);

      uploadFile(fsData, '/fota', function() {
        $('#msg').html('<p style="color:green;">Filesystem uploaded successfully! Rebooting...</p>');
        initiateReboot();
      });
    } else {
      $('#msg').html('<p style="color:green;">Update successful! Rebooting...</p>');
      initiateReboot();
    }
  });
});
function initiateReboot() {
  $.ajax({
    url: '/reboot',
    type: 'GET',
    success: function() {
      $('#msg').append('<p>Reboot initiated.</p>');
    },
    error: function() {
      $('#msg').append('<p style="color:red;">Failed to initiate reboot.</p>');
    },
    complete: function() {
      setTimeout(function() { window.location.href = '/'; }, 2000);
    }
  });
}
function uploadFile(data, url, successCallback) {
  $.ajax({
    url: '/fota',
    type: 'POST',
    data: data,
    contentType: false,
    processData: false,
    xhr: function() {
      var xhr = new window.XMLHttpRequest();
        xhr.upload.addEventListener('progress', function(evt) {
        if (evt.lengthComputable) {
            var per = evt.loaded / evt.total;
            $('#prg').html('progress: ' + Math.round(per*100) + '%');
          if (per < 1) {
            $('#msg').html('<p>Uploading...</p>');
          } else {
            $('#msg').html('<p style="color:blue;">Flashing...</p>');
          }
        }
      }, false);
      return xhr;
    },
    success: function() {
      $('#msg').html('<p style="color:green;">Update successful!</p>');
      if (successCallback) successCallback();
    },
    error: function() {
      $('#msg').html('<p style="color:red;">Update failed! Please try again.</p>');
    }
  });
}
</script>
</body>
</html>)";

static constexpr const char *styleSheet PROGMEM =
R"(
input[type=file]::file-selector-button, input[type="submit"], a, button {
    padding: 7px 15px;
    border: none;
    background: #007BFF;
    color: white;
    text-decoration: none;
    border-radius: 5px;
    margin-top: 5px;
    display: inline-block;
    font-size: 16px;
    font-family: Arial, sans-serif;
    cursor: pointer;
    text-align: center;
}
input[type=file]::file-selector-button:hover, input[type="submit"]:hover, a:hover, button:hover {
    background-color: #0056b3;
}
table, td, th {
  border: 1px solid;
  padding: 5px;
}
table {
  border-collapse: collapse;
})";

static constexpr const char *rebootPage PROGMEM =
R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<META http-equiv="refresh" content="5;URL=/">
<link rel="stylesheet" href="/styles.css">
<title>Rebooting</title>
</head>
<body>
<h1>Rebooting ESP...</h1>
</body>
</html>)";

static constexpr const char *configPageTemplate PROGMEM =
R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<link rel="stylesheet" href="/styles.css">
<title>Configuration</title>
</head>
<body>
<h1>Configuration</h1>
<form id='config_form' action='#' method='POST'>
<table>
<tr><td>Spa Name:</td><td><input type='text' name='spaName' id='spaName'></td></tr>
<tr><td>MQTT Server:</td><td><input type='text' name='mqttServer' id='mqttServer'></td></tr>
<tr><td>MQTT Port:</td><td><input type='number' name='mqttPort' id='mqttPort'></td></tr>
<tr><td>MQTT Username:</td><td><input type='text' name='mqttUsername' id='mqttUsername'></td></tr>
<tr><td>MQTT Password:</td><td><input type='text' name='mqttPassword' id='mqttPassword'></td></tr>
<tr><td>Poll Frequency (seconds):</td><td><input type='number' name='updateFrequency' id='updateFrequency' step="1" min="10" max="300"></td></tr>
</table>
<input type='submit' value='Save'>
</form>
<div id='msg'></div>
<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
<script>
function loadConfig() {
  fetch('/json/config')
    .then(response => response.json())
    .then(data => {
      document.getElementById('spaName').value = data.spaName;
      document.getElementById('mqttServer').value = data.mqttServer;
      document.getElementById('mqttPort').value = data.mqttPort;
      document.getElementById('mqttUsername').value = data.mqttUsername;
      document.getElementById('mqttPassword').value = data.mqttPassword;
      document.getElementById('updateFrequency').value = data.updateFrequency;
    })
  .catch(error => console.error('Error loading config:', error));
}
document.addEventListener("DOMContentLoaded", loadConfig);
$('#config_form').submit(function(e) {
  e.preventDefault();
  $.ajax({
    url: '/config',
    type: 'POST',
    data: $('#config_form').serialize(),
    success: function() {
      $('#msg').html('<p style="color:green;">Configuration updated successfully!</p>');
      loadConfig();
      setTimeout(function() { $('#msg').html(''); }, 3000);
    },
    error: function() {
      $('#msg').html('<p style="color:red;">Error updating configuration.</p>');
    }
  });
});
</script>
</body>
</html>)";

static constexpr const char *jsonHTMLTemplate PROGMEM =
R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<link rel="stylesheet" href="/styles.css">
<style>
  table {
    width: 80%;
    border-collapse: collapse;
    margin: auto;
  }
  th, td {
    padding: 8px;
    border: 1px solid #ddd;
    text-align: left;
  }
  th {
    background-color: #f4f4f4;
  }
  @media (prefers-color-scheme: dark) {
    th {
      background-color: #333;
    }
</style>
<title>JSON Data Table</title>
</head>
<body>
<h2>JSON Data Table</h2>
<table id="dataTable">
  <thead>
    <tr id="headerRow"></tr>
  </thead>
  <tbody>
  </tbody>
</table>
<script>
// Utility to calculate leaf row counts while treating arrays as single entries
function calculateRowCounts(obj) {
  if (typeof obj !== "object" || obj === null || Array.isArray(obj) && !obj.some(item => typeof item === "object")) {
    return 1; // Count as 1 if primitive or a simple array
  }
  let totalRows = 0;
  for (const value of Object.values(obj)) {
    totalRows += calculateRowCounts(value);
  }
  return totalRows;
}

// Recursive function to create rows for leaf nodes with accurate rowspan for parents
function createRows(data, tableBody, level = 0, parentCells = [], maxDepth) {
  Object.entries(data).forEach(([key, value]) => {
    if (typeof value === "object" && value !== null && !Array.isArray(value)) {
      const rowCount = calculateRowCounts(value);
      createRows(value, tableBody, level + 1, [...parentCells, { name: key, count: rowCount }], maxDepth);
    } else {
      const row = document.createElement("tr");

      // Apply row spans for parent cells accurately
      parentCells.forEach((parent, index) => {
        if (index >= level) return;
        if (parent.count > 0) {
          const cell = document.createElement("td");
          cell.rowSpan = parent.count;
          cell.textContent = parent.name;
          row.appendChild(cell);
          parent.count = 0; // Clear count for other rows
        }
      });

      // Add current level cell for the key
      const keyCell = document.createElement("td");
      keyCell.textContent = key;
      row.appendChild(keyCell);

      const valueCell = document.createElement("td");
      valueCell.textContent = Array.isArray(value) ? value.join(", ") : value;
      row.appendChild(valueCell);

      // Add empty cells to align with maxDepth
      for (let i = level + 2; i < maxDepth; i++) {
        const emptyCell = document.createElement("td");
        row.appendChild(emptyCell);
      }

      tableBody.appendChild(row);
    }
  });
}

// Populate header row based on calculated depth
function populateHeaderRow(depth) {
  const headerRow = document.getElementById("headerRow");
  headerRow.innerHTML = ""; // Clear existing headers
  for (let i = 1; i <= depth; i++) {
    const headerCell = document.createElement("th");
    headerCell.textContent = `Level ${i}`;
    headerRow.appendChild(headerCell);
  }
}

// Fetch JSON data and populate the table
async function fetchDataAndPopulateTable() {
  try {
    const response = await fetch('/json');
    if (!response.ok) throw new Error(`Error fetching data: ${response.statusText}`);

    const jsonData = await response.json();
    const depth = calculateDepth(jsonData); // Total column depth
    populateHeaderRow(depth);

    const tableBody = document.getElementById("dataTable").querySelector("tbody");
    tableBody.innerHTML = ""; // Clear previous rows
    createRows(jsonData, tableBody, 0, [], depth);
  } catch (error) {
    console.error(error);
  }
}

// Calculate maximum depth of JSON structure
function calculateDepth(obj, depth = 0) {
  if (typeof obj !== "object" || obj === null) return depth;
  let maxDepth = depth;
  for (const value of Object.values(obj)) {
    maxDepth = Math.max(maxDepth, calculateDepth(value, depth + 1));
  }
  return maxDepth;
}

// Fetch data on page load
window.onload = fetchDataAndPopulateTable;
</script>
</body>
</html>)";

};

#endif // WEBUI_H
