#ifndef SPAUTILS_H
#define SPAUTILS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <RemoteDebug.h>
#include <time.h>
#include <TimeLib.h>
#include "SpaInterface.h"
#include "Config.h"
#include <PubSubClient.h>
#include "MQTTClientWrapper.h"
#include "HttpContent.h"

//define stringify function
#define xstr(a) str(a)
#define str(a) #a

#define REPO_OWNER "wayne-love"
#define REPO "ESPySpa"
#define RELEASES_URL "https://api.github.com/repos/" REPO_OWNER "/" REPO "/releases/latest"

extern RemoteDebug Debug;

String convertToTime(int data);
int convertToInteger(String &timeStr);
bool getPumpModesJson(SpaInterface &si, int pumpNumber, JsonObject pumps);

bool getPumpInstalledState(String pumpState);
String getPumpSpeedType(String pumpState);
String getPumpPossibleStates(String pumpState);
int getPumpSpeedMax(String pumpState);
int getPumpSpeedMin(String pumpState);

bool generateStatusJson(SpaInterface &si, MQTTClientWrapper &mqttClient, Config &config, String &output, bool prettyJson=false);

String fetchLatestVersion(const String& url);
bool parseVersion(const String version, int parsedVersion[3]);
int compareVersions(const int current[3], const int latest[3]);
#ifdef INCLUDE_UPDATES
void firmwareCheckUpdates(Config &config);
void updateFirmware(const String firmwareUrl, const String spiffsUrl, Config &config, bool reboot);
#endif // INCLUDE_UPDATES

#endif // SPAUTILS_H
