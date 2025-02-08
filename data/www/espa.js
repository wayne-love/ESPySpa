/************************************************************************************************
 * 
 * Utility Methods
 * 
 ***********************************************************************************************/
function confirmAction(url) {
    if (confirm('Are you sure?')) {
        window.location.href = url;
    }
}

function confirmFunction(func) {
    if (confirm('Are you sure?')) {
        func();
    }
}

function parseVersion(version) {
    const match = version.match(/^v([\d.]+)/);
    if (!match) return null;
    return match[1].split('.').map(Number);
}

function compareVersions(current, latest) {
    if (!current) return -1;
    if (!latest) return 1;
    for (let i = 0; i < Math.max(current.length, latest.length); i++) {
        const a = current[i] || 0;
        const b = latest[i] || 0;
        if (a < b) return -1;
        if (a > b) return 1;
    }
    return 0;
}

// Copy to clipboard functionality
$('#copyToClipboardButton').click(function () {
    copyToClipboard('#infoModelPre');
});

function copyToClipboard(element) {
    const text = $(element).text();
    navigator.clipboard.writeText(text).then(() => {
        //alert('Copied to clipboard: ' + text);
    }).catch(err => {
        console.error('Failed to copy: ', err);
    });
}

function reboot(message) {
    $.ajax({
      url: '/reboot',
      type: 'GET',
      success: () => showAlert(message, 'alert-success', 'Reboot'),
      error: () => showAlert('Failed to initiate reboot.', 'alert-danger', 'Error'),
      complete: () => setTimeout(() => location.href = '/', 2000)
    });
}


/************************************************************************************************
 * 
 * Status 
 * 
 ***********************************************************************************************/

let fetchStatusFailed = false;

function fetchStatus() {
    fetch('/json')
        .then(response => response.json())
        .then(value_json => {
            if (fetchStatusFailed) {
                clearAlert();
                fetchStatusFailed = false;
            }
            updateStatusElement('status_state', value_json.status.state);
            updateStatusElement('temperatures_water', value_json.temperatures.water + "\u00B0C");
            updateStatusElement('temperatures_setPoint', value_json.temperatures.setPoint);
            updateStatusElement('status_controller', value_json.status.controller);
            updateStatusElement('status_firmware', value_json.status.firmware);
            updateStatusElement('status_serial', value_json.status.serial);
            updateStatusElement('status_siInitialised', value_json.status.siInitialised);
            updateStatusElement('status_mqtt', value_json.status.mqtt);
            updateStatusElement('espa_model', value_json.eSpa.model);
            updateStatusElement('espa_build', value_json.eSpa.update.installed_version);
        })
        .catch(error => {
            console.error('Error fetching status:', error);
            showAlert('Error connecting to the spa. If this persists, take a look at our <a class="alert-link" href="https://espa.diy/troubleshooting.html">troubleshooting docs</a>.', 'alert-danger', "Error");
            fetchStatusFailed = true;
            handleStatusError('status_state');
            handleStatusError('temperatures_water');
            handleStatusError('temperatures_setPoint');
            handleStatusError('status_controller');
            handleStatusError('status_firmware');
            handleStatusError('status_serial');
            handleStatusError('status_siInitialised');
            handleStatusError('status_mqtt');
            handleStatusError('espa_model');
            handleStatusError('espa_build');
        });
}

function updateStatusElement(elementId, value) {
    const element = document.getElementById(elementId);
    element.classList.remove('badge', 'text-bg-warning', 'text-bg-danger');
    if (element instanceof HTMLInputElement) {
        element.value = value;
    } else {
        element.textContent = value;
    }
}

function handleStatusError(elementId) {
    const element = document.getElementById(elementId);
    element.classList.remove('text-bg-warning');
    element.classList.add('text-bg-danger');
    if (element instanceof HTMLInputElement) {
        element.value = '';
    } else {
        element.textContent = 'Failed to load';
    }
}

function clearAlert() {
    const pageAlert = $('#page-alert');
    const pageAlertParent = $('.page-alert-parent');
    pageAlert.removeClass(function (index, className) {
        return (className.match(/(^|\s)alert-\S+/g) || []).join(' ');
    }).text('');
    pageAlertParent.hide();
}

window.onload = function () {
    fetchStatus();
    loadFotaData();
    setInterval(fetchStatus, 10000);
}


/************************************************************************************************
 * 
 * Updating eSpa configuration
 * 
 ***********************************************************************************************/

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
        .catch(error => console.error('Error setting datetime:', error));
}

// Retrieving and updating the configured settings, so they can be displayed in the modal popup
function loadConfig() {
    $('#configErrorAlert').hide();
    fetch('/json/config')
        .then(response => response.json())
        .then(data => {
            document.getElementById('spaName').value = data.spaName;
            document.getElementById('mqttServer').value = data.mqttServer;
            document.getElementById('mqttPort').value = data.mqttPort;
            document.getElementById('mqttUsername').value = data.mqttUsername;
            document.getElementById('mqttPassword').value = data.mqttPassword;
            document.getElementById('updateFrequency').value = data.updateFrequency;

            // Enable form fields and save button
            $('#config_form input').prop('disabled', false);
            $('#saveConfigButton').prop('disabled', false);
        })
        .catch(error => {
            console.error('Error loading config:', error);
            $('#configErrorAlert').text('Error loading configuration. Please try again.').show();

            // Make form fields read-only and disable save button
            $('#config_form input').prop('disabled', true);
            $('#saveConfigButton').prop('disabled', true);
        });
}

// Configuration modal 
$(document).ready(function () {
    // configuration settings modal
    $('#configLink').click(function (event) {
        event.preventDefault();
        $('#configModal').modal('show');
    });

    // Load configuration when the config modal is shown
    $('#configModal').on('shown.bs.modal', function () {
        loadConfig();
    });

    // Handle form submission when the save button is clicked
    $('#saveConfigButton').click(function () {
        submitConfigForm();
    });

    function submitConfigForm() {
        $.ajax({
            url: '/config',
            type: 'POST',
            data: $('#config_form').serialize(),
            success: function () {
                showAlert('Configuration updated successfully!', 'alert-success', 'Success');
                loadConfig();
                $('#configModal').modal('hide');
            },
            error: function () {
                $('#configErrorAlert').text('Error updating configuration. Please try again.').show();
            }
        });
    }

    $('#config_form').submit(function (e) {
        e.preventDefault();
        submitConfigForm();
    });
});


/************************************************************************************************
 * 
 * OTA Update Support
 * 
 ***********************************************************************************************/

$(document).ready(function () {
    $('#progressDiv').hide();
    $('#localInstallButton').prop('disabled', true);
    $('#localUpdate').show();
    document.getElementById('updateForm').reset();

    // Delegate event listener for dynamically added #fotaLink
    $(document).on('click', '#fotaLink', function (event) {
        event.preventDefault();
        $('#fotaModal').modal('show');
        // loadFotaData();
    });

    // Enable the local install button when a file is selected
    $('#fsFile').change(updateLocalInstallButton);
    $('#appFile').change(updateLocalInstallButton);
    function updateLocalInstallButton () {
        if ($('#fsFile').val() || $('#appFile').val()) {
            $('#localInstallButton').prop('disabled', false);
        } else {
            $('#localInstallButton').prop('disabled', true);
        }
    };

    // Handle local install button click
    $('#localInstallButton').click(async function () {
        const appFile = $('#appFile')[0].files[0];
        const fsFile = $('#fsFile')[0].files[0];
        let appSuccess = false, fsSuccess = false;

        if (!appFile && !fsFile) {
          showAlert('Please select either an application or filesystem update file.', 'alert-danger', 'Error');
          console.error('No files selected for upload.');
          return;
        }

        let totalFiles = 1;
        if (appFile && fsFile) totalFiles = 2;
        let fileNum = 0;
        // Upload application file if provided
        if (appFile) {
            const appData = new FormData();
            appData.append('updateType', 'application');
            appData.append('update', appFile);
            fileNum++;
            $('#msg').html(`<p style="color:blue;">Uploading file ${fileNum} of ${totalFiles} - Application update.</p>`);
            appSuccess = await uploadFileAsync(appData, '/fota', fileNum, totalFiles);
        }

        // Upload filesystem file if provided
        if (fsFile) {
            const fsData = new FormData();
            fsData.append('updateType', 'filesystem');
            fsData.append('update', fsFile);
            fileNum++;
            $('#msg').html(`<p style="color:blue;">Uploading file ${fileNum} of ${totalFiles} - File system update.</p>`);
            fsSuccess = await uploadFileAsync(fsData, '/fota', fileNum, totalFiles);
        }

        // Trigger reboot only if all provided uploads were successful
        if ((!appFile || appSuccess) && (!fsFile || fsSuccess)) {
            $('#fotaModal').modal('hide');
            setTimeout(() => reboot('The firmware has been updated successfully. The spa will now restart to apply the changes.'), 500);
        } else {
            showAlert('One or more uploads failed.', 'alert-danger', 'Error');
        }

        document.getElementById('updateForm').reset();
      });

    async function uploadFileAsync(data, url, fileNum, totalFiles) {
        let percentMultipler = 1 / totalFiles;
        let startPercent = percentMultipler * 100 * (fileNum - 1);
        return new Promise((resolve) => {
            $.ajax({
            url,
            type: 'POST',
            data,
            contentType: false,
            processData: false,
            xhr: function () {
                $('#progressDiv').show();
                const xhr = new XMLHttpRequest();
                xhr.upload.addEventListener('progress', function(evt) {
                    if (evt.lengthComputable) {
                        var percentComplete = evt.loaded / evt.total;
                        percentComplete = parseInt((percentComplete * 100 * percentMultipler) + startPercent);
                        $('#progressBar').css('width', percentComplete + '%').attr('aria-valuenow', percentComplete).text(percentComplete + '%');
                    }
                }, false);
                return xhr;
            },
            success: function (data) {
                showAlert('The firmware has been uploaded.', 'alert-success', 'Firmware uploaded');
                resolve(true);
            },
            error: function () {
                showAlert('The firmware update failed. Please try again.', 'alert-danger', 'Error');
                resolve(false);
            }
            });
        });
    }
}

    // Handle remote update installation
    /*
    $('#remoteInstallButton').click(function (event) {
        event.preventDefault();
        var selectedVersion = $('#firmware-select').val();
        if (selectedVersion) {
            $.ajax({
                url: '/install',
                type: 'POST',
                data: { version: selectedVersion },
                success: function (data) {
                    showAlert('The firmware has been updated successfully. The spa will now restart to apply the changes.', 'alert-success', 'Firmware updated');
                    $('#fotaModal').modal('hide');
                },
                error: function () {
                    showAlert('The firmware update failed. Please try again.', 'alert-danger', 'Error');
                }
            });
        }
    });

    // Show/hide update sections based on selected update method
    $('#updateMethod').change(function () {
        var selectedMethod = $(this).val();
        if (selectedMethod === 'remote') {
            $('#remoteUpdate').show();
            $('#localUpdate').hide();
        } else if (selectedMethod === 'local') {
            $('#remoteUpdate').hide();
            $('#localUpdate').show();
        } else {
            $('#remoteUpdate').hide();
            $('#localUpdate').hide();
        }
    });
    */
);

function loadFotaData() {
    fetch('/json')
        .then(response => response.json())
        .then(value_json => {
            document.getElementById('espa_model').innerText = value_json.eSpa.model;
            document.getElementById('installedVersion').innerText = value_json.eSpa.update.installed_version;
        })
        .catch(error => console.error('Error fetching FOTA data:', error));

    $.ajax({
        url: 'https://api.github.com/repos/wayne-love/ESPySpa/releases',
        type: 'GET',
        success: function (data) {
            document.getElementById('lastestRelease').innerText = data[0].tag_name;

            // Populate the select dropdown with all releases
            const firmwareSelect = document.getElementById('firmware-select');
            firmwareSelect.innerHTML = ''; // Clear existing options

            // Add default disabled option
            const defaultOption = document.createElement('option');
            defaultOption.value = '';
            defaultOption.text = 'Select a version';
            defaultOption.disabled = true;
            defaultOption.selected = true;
            firmwareSelect.appendChild(defaultOption);

            // Add release options
            data.forEach(release => {
                const option = document.createElement('option');
                option.value = release.tag_name;
                option.text = release.tag_name;
                firmwareSelect.appendChild(option);
            });

            // Enable the install button when a valid choice is selected
            firmwareSelect.addEventListener('change', function () {
                if (firmwareSelect.value) {
                    $('#remoteInstallButton').prop('disabled', false);
                } else {
                    $('#remoteInstallButton').prop('disabled', true);
                }
            });

            // Check for new version
            const latestVersion = parseVersion(data[0].tag_name);
            const currentVersion = parseVersion(document.getElementById('installedVersion').innerText);
            const comparison = compareVersions(currentVersion, latestVersion);
            if (comparison < 0) {
                showAlert(`There is a new eSpa release available - it's version ${data[0].tag_name}. You can <a href="#" id="fotaLink" class="alert-link">update now</a>.`, 'alert-primary', "New eSpa release!");
            }
        },
        error: function () {
            showAlert('Failed to fetch eSpa release information. If this persists, take a look at our <a class="alert-link" href="https://espa.diy/troubleshooting.html">troubleshooting docs</a>.', 'alert-danger', "Error");
        }
    });
}


/************************************************************************************************
 * 
 * Status models (for JSON and Spa response)
 * 
 ***********************************************************************************************/

$(document).ready(function () {
    // JSON dump modal
    $('#jsonLink').click(function (event) {
        event.preventDefault();
        fetch('/json').then(response => response.json()).then(data => {
            $('#infoModalTitle').html("Spa JSON");
            $('#infoModalBody').html('<pre id="infoModelPre">' + JSON.stringify(data, null, 2) + '</pre>');
            $('#infoModal').modal('show');
        })
        .catch(error => {
            console.error('Error fetching JSON:', error);
            showAlert('Error connecting to the spa.  If this persists, take a look at our <a class="alert-link" href="https://espa.diy/troubleshooting.html">troubleshooting docs</a>.', 'alert-danger', "Error");
        });
    });

    // spa status modal
    $('#statusLink').click(function (event) {
        event.preventDefault();
        fetch('/status').then(response => response.text()).then(data => {
            $('#infoModalTitle').html("Spa Status");
            $('#infoModalBody').html('<pre>' + data + '</pre>');
            $('#infoModal').modal('show');
        })
        .catch(error => {
            console.error('Error fetching status:', error);
            showAlert('Error connecting to the spa.  If this persists, take a look at our <a class="alert-link" href="https://espa.diy/troubleshooting.html">troubleshooting docs</a>.', 'alert-danger', "Error");
        });
    });
});


/************************************************************************************************
 * 
 * Front page alerts
 * 
 ***********************************************************************************************/

function showAlert(message, alertClass, title = '') {
    const pageAlert = $('#page-alert');
    const pageAlertParent = $('.page-alert-parent');

    // Clear existing alert classes and set the new class
    pageAlert.removeClass(function (index, className) {
        return (className.match(/(^|\s)alert-\S+/g) || []).join(' ');
    }).addClass(alertClass);

    // Construct the alert content
    let alertContent = '';
    if (title) {
        alertContent += `<h4 class="alert-heading">${title}</h4>`;
    }
    alertContent += message;

    // Set the alert content and show the alert
    pageAlert.html(alertContent);
    pageAlertParent.show();
}


/************************************************************************************************
 * 
 * Light / Dark Mode Switch
 * 
 ***********************************************************************************************/

document.addEventListener('DOMContentLoaded', (event) => {
    const htmlElement = document.documentElement;
    const switchElement = document.getElementById('darkModeSwitch');

    // Set the default theme to dark if no setting is found in local storage
    const currentTheme = localStorage.getItem('bsTheme') || 'dark';
    htmlElement.setAttribute('data-bs-theme', currentTheme);
    switchElement.checked = currentTheme === 'dark';

    switchElement.addEventListener('change', function () {
        if (this.checked) {
            htmlElement.setAttribute('data-bs-theme', 'dark');
            localStorage.setItem('bsTheme', 'dark');
        } else {
            htmlElement.setAttribute('data-bs-theme', 'light');
            localStorage.setItem('bsTheme', 'light');
        }
    });
});