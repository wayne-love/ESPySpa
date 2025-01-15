#ifndef HTTP_CONTENT_H
#define HTTP_CONTENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>
#include <RemoteDebug.h>

#include "Config.h"

extern RemoteDebug Debug;

#define MAX_REDIRECTS 5

class HttpContent {
private:
    /**
     * Attempts to initialize an HTTPClient for the given URL, following redirects if necessary.
     * @param url The URL to fetch.
     * @param http A reference to the HTTPClient object.
     * @return True if the HTTPClient was successfully initialized; false otherwise.
     */
    bool getHttpClient(const String url, HTTPClient& http);

public:
    HttpContent();

    /**
     * Fetches the content from the specified URL as a String.
     * @param url The URL to fetch.
     * @param content A reference to the String where the fetched content will be stored.
     * @return True if the content was successfully fetched; false otherwise.
     */
    bool fetchHttpContent(const String url, String& content);

    /**
     * Flashes the firmware from the given URL.
     * @param firmwareUrl The URL of the firmware file.
     * @param updateType Integer defining the type of update (firmware update or filesystem)
     * @return True if the firmware update was successful; false otherwise.
     */
    bool flashFirmware(const String firmwareUrl, const String updateType, Config &config, const int updateNum, const int numUpdates);
};

#endif // HTTP_CONTENT_H
