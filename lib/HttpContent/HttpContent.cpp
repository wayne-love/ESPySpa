#ifdef INCLUDE_UPDATES
#include "HttpContent.h"

HttpContent::HttpContent() {}

bool HttpContent::getHttpClient(const String url, HTTPClient& http) {
    int redirectCount = 0;
    String currentUrl = url;

    while (redirectCount < MAX_REDIRECTS) {
        debugD("Requesting URL: %s", currentUrl.c_str());
        http.begin(currentUrl);
        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            debugD("HTTP GET successful for URL: %s", currentUrl.c_str());
            return true;
        } else if (httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND) {
            String newUrl = http.getLocation();
            if (newUrl.isEmpty()) {
                debugE("Redirect response with empty location.");
                http.end();
                return false;
            }
            currentUrl = newUrl;
            debugD("Redirecting to: %s", currentUrl.c_str());
            redirectCount++;
            http.end();
        } else {
            debugE("HTTP GET failed for URL: %s, Code: %d", currentUrl.c_str(), httpCode);
            http.end();
            return false;
        }
    }

    debugE("Maximum redirects (%d) reached for URL: %s", MAX_REDIRECTS, url.c_str());
    return false;
}

bool HttpContent::fetchHttpContent(const String url, String& content) {
    HTTPClient http;
    if (getHttpClient(url, http)) {
        content = http.getString();
        debugD("Fetched content");
        debugV("%s", content.c_str());
        http.end();
        return true;
    }

    debugE("Failed to fetch content from URL: %s", url.c_str());
    return false;
}

bool HttpContent::flashFirmware(const String firmwareUrl, const String type, Config &config, const int updateNum, const int numUpdates) {
    HTTPClient http;

    static int updateType = U_FLASH; // Default to firmware update

    if (type) {
        if (type == "filesystem") {
            updateType = U_SPIFFS;
            debugD("Filesystem update selected.");
            config.updateStatus.setValue("Updating SPIFFS...");
        } else if (type == "application") {
            updateType = U_FLASH;
            debugD("Application (firmware) update selected.");
            config.updateStatus.setValue("Updating firmware...");
        } else {
            debugD("Unknown update type: %s", type.c_str());
            return false;
        }
    } else {
        debugD("No update type specified. Defaulting to application update.");
        config.updateStatus.setValue(String("Updating firmware... ") + updateNum + " of " + numUpdates);
    }

    if (getHttpClient(firmwareUrl, http)) {
        WiFiClient* stream = http.getStreamPtr();
        size_t contentLength = http.getSize();

        if (!Update.begin(contentLength, updateType)) {
            debugE("Update.begin() failed: Not enough space for update.");
            http.end();
            config.updateStatus.setValue("Update failed: Not enough space.");
            return false;
        }

        debugD("Writing firmware to flash...");
        size_t written = 0;
        uint8_t buff[128] = { 0 };
        uint16_t startUpdate = millis();
        while (written < contentLength || millis() - startUpdate < 60000) {
            if (stream->available()) {
                size_t len = stream->readBytes(buff, sizeof(buff));
                written += Update.write(buff, len);

                // Calculate and update the percentage complete
                int progress = ((written / (float)contentLength) * 100 / numUpdates) + ((updateNum - 1) * 100 / numUpdates);
                debugV("Update progress: %d%%", progress);
                config.updatePercentage.setValue(progress);
            } else {
                delay(50);
            }
        }

        if (written == contentLength) {
            if (Update.end() && Update.isFinished()) {
                debugD("Success: Firmware update complete.");
                http.end();
                config.updateStatus.setValue("Update successful.");
                return true;
            } else {
                debugE("Firmware update failed to complete.");
            }
        } else {
            debugE("Firmware write failed. Written: %zu, Expected: %zu", written, contentLength);
        }
        http.end();
    } else {
        debugE("Failed to initialize HTTPClient for firmware URL: %s", firmwareUrl.c_str());
    }

    config.updateStatus.setValue("Update failed.");
    return false;
}
#endif // INCLUDE_UPDATES
