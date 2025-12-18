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

//define stringify function
#define xstr(a) str(a)
#define str(a) #a

extern RemoteDebug Debug;

String convertToTime(int data);
int convertToInteger(String &timeStr);
bool getPumpModesJson(SpaInterface &si, int pumpNumber, JsonObject pumps);

bool getPumpInstalledState(String pumpState);
String getPumpSpeedType(String pumpState);
String getPumpPossibleStates(String pumpState);
int getPumpSpeedMax(String pumpState);
int getPumpSpeedMin(String pumpState);

#ifdef ENABLE_ESPA_CONTROL
  class EspaControl; // Forward declaration
#endif

bool generateStatusJson(SpaInterface &si, MQTTClientWrapper &mqttClient, String &output, bool prettyJson=false, EspaControl *espaControl=nullptr);

#endif // SPAUTILS_H
