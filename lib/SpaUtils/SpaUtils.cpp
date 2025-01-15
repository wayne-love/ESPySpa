#include "SpaUtils.h"

// Function to convert integer to time in HH:mm format
String convertToTime(int data) {
  // Extract hours and minutes from data
  int hours = (data >> 8) & 0x3F; // High byte for hours
  int minutes = data & 0x1F;      // Low byte for minutes

  String timeStr = String(hours / 10) + String(hours % 10) + ":" +
                   String(minutes / 10) + String(minutes % 10);

  // Print debug information
  debugV("data: %i, timeStr: %s", data, timeStr.c_str());

  return timeStr;
}

int convertToInteger(String &timeStr) {
  int data = -1;

  // Check for an empty string
  if (timeStr.length() == 0) {
    return data;
  }

  // Find the position of the colon
  int colonIndex = timeStr.indexOf(':');
  if (colonIndex == -1) {
    return data; // Invalid format
  }

  // Extract hours and minutes as substrings
  int hours = timeStr.substring(0, colonIndex).toInt();
  int minutes = timeStr.substring(colonIndex + 1).toInt();

  // Validate hours and minutes ranges
  if (hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60) {
    data = (hours * 256) + minutes;
  }

  // Print debug information
  debugV("data: %i, timeStr: %s", data, timeStr.c_str());

  return data;
}

bool getPumpModesJson(SpaInterface &si, int pumpNumber, JsonObject pumps) {
  // Validate the pump number
  if (pumpNumber < 1 || pumpNumber > 5) {
    return false;
  }

  // Retrieve the pump install state dynamically
  String pumpInstallState = (si.*(pumpInstallStateFunctions[pumpNumber - 1]))();

  char pumpKey[6] = "pump";  // Start with "pump"
  pumpKey[4] = '0' + pumpNumber;  // Append the pump number as a character
  pumpKey[5] = '\0';  // Null-terminate the string

  pumps[pumpKey]["installed"] = getPumpInstalledState(pumpInstallState);
  String speedType = getPumpSpeedType(pumpInstallState);
  pumps[pumpKey]["speedType"] = speedType;

  String possibleStates = getPumpPossibleStates(pumpInstallState);
  // Convert possibleStates into words and store them in a JSON array
  for (uint i = 0; i < possibleStates.length(); i++) {
    char stateChar = possibleStates.charAt(i);
    if (stateChar == '0') {
      pumps[pumpKey]["possibleStates"].add("OFF");
    } else if (stateChar == '1') {
      pumps[pumpKey]["possibleStates"].add("ON");
    } else if (stateChar == '2') {
      pumps[pumpKey]["possibleStates"].add("LOW");
    } else if (stateChar == '3') {
      pumps[pumpKey]["possibleStates"].add("HIGH");
    } else if (stateChar == '4') {
      pumps[pumpKey]["possibleStates"].add("AUTO");
    }
  }

  int pumpState = (si.*(pumpStateFunctions[pumpNumber - 1]))();
  if (pumpInstallState.endsWith("4") && possibleStates.length() > 1) {
    if (pumpState == 4) pumps[pumpKey]["mode"] = "Auto";
    else pumps[pumpKey]["mode"] = "Manual";
  }
  pumps[pumpKey]["state"] = pumpState==0?"OFF":"ON";
  if (pumpState == 4) pumpState = 2;
  pumps[pumpKey]["speed"] = pumpState;

  return true;
}

bool getPumpInstalledState(String pumpInstallState) {
  return pumpInstallState.startsWith("1");
}

String getPumpSpeedType(String pumpInstallState) {
  int firstDash = pumpInstallState.indexOf("-");
  int secondDash = pumpInstallState.lastIndexOf("-");
  return pumpInstallState.substring(firstDash + 1, secondDash);
}

String getPumpPossibleStates(String pumpInstallState) {
  int secondDash = pumpInstallState.lastIndexOf("-");
  return pumpInstallState.substring(secondDash + 1);
}

int getPumpSpeedMax(String pumpInstallState) {
  String possibleStates = getPumpPossibleStates(pumpInstallState);
  uint max = 0;
  for (uint i = 0; i < possibleStates.length(); i++) {
    int pumpMode = possibleStates.charAt(i)  - '0';
    if (pumpMode > 0 && pumpMode < 4 && pumpMode > max) max = pumpMode;
  }
  return max;
}

int getPumpSpeedMin(String pumpInstallState) {
  String possibleStates = getPumpPossibleStates(pumpInstallState);
  uint min = UINT_MAX;
  for (uint i = 0; i < possibleStates.length(); i++) {
    int pumpMode = possibleStates.charAt(i)  - '0';
    if (pumpMode > 0 && pumpMode < 4 && pumpMode < min) min = pumpMode;
  }
  if (min == UINT_MAX) min = 0;
  return min;
}

bool generateStatusJson(SpaInterface &si, MQTTClientWrapper &mqttClient, Config &config, String &output, bool prettyJson) {
  JsonDocument json;

  json["temperatures"]["setPoint"] = si.getSTMP() / 10.0;
  json["temperatures"]["water"] = si.getWTMP() / 10.0;
  json["temperatures"]["heater"] = si.getHeaterTemperature() / 10.0;
  json["temperatures"]["case"] = si.getCaseTemperature(); 
  json["temperatures"]["heatpumpAmbient"] = si.getHP_Ambient();
  json["temperatures"]["heatpumpCondensor"] = si.getHP_Condensor();

  json["power"]["voltage"] = si.getMainsVoltage();
  json["power"]["current"]= si.getMainsCurrent() / 10.0; // convert value to A
  json["power"]["power"] = si.getPower() / 10.0; // convert value to W
  json["power"]["totalenergy"]= si.getPower_kWh() / 100.0; // convert value to kWh.

  json["status"]["heatingActive"] = si.getRB_TP_Heater()? "ON": "OFF";
  json["status"]["ozoneActive"] = si.getRB_TP_Ozone()? "ON": "OFF";
  json["status"]["state"] = si.getStatus();
  json["status"]["spaMode"] = si.getMode();
  json["status"]["controller"] = si.getModel();
  String firmware = si.getSVER().substring(3);
  firmware.replace(' ', '.');
  json["status"]["firmware"] = firmware;
  json["status"]["serial"] = si.getSerialNo1() + "-" + si.getSerialNo2();
  json["status"]["siInitialised"] = si.isInitialised()?"true":"false";
  json["status"]["mqtt"] = mqttClient.connected()?"connected":"disconnected";

  json["eSpa"]["model"] = xstr(PIOENV);
  json["eSpa"]["updateAvailable"] = config.updateAvailable.getValue() == 1;
  json["eSpa"]["update"]["installed_version"] = xstr(BUILD_INFO);
  json["eSpa"]["update"]["latest_version"] = config.latestVersion.getValue();
  String releaseNotes = config.releaseNotes.getValue();
  int newLineIndex = releaseNotes.indexOf("\n");
  if (newLineIndex > 0) {
    releaseNotes = releaseNotes.substring(0, newLineIndex);
  }
  json["eSpa"]["update"]["release_summary"] = releaseNotes;
  json["eSpa"]["update"]["release_url"] = config.releaseUrl.getValue();
  json["eSpa"]["update"]["in_progress"] = false; //TODO
  if (false) { //if updating...
    json["eSpa"]["update"]["update_percentage"] = 75; //TODO
  }

  json["heatpump"]["mode"] = si.HPMPStrings[si.getHPMP()];
  json["heatpump"]["auxheat"] = si.getHELE()==0? "OFF" : "ON";

  JsonObject pumps = json["pumps"].to<JsonObject>();
  // Add pump data by calling the function for each pump
  for (int i = 1; i <= 5; i++) {
    if (!getPumpModesJson(si, i, pumps)) {
      debugD("Invalid pump number: %i", i);
    }
  }

  String y=String(year(si.getSpaTime()));
  String m=String(month(si.getSpaTime()));
  if (month(si.getSpaTime())<10) m = "0"+m;
  String d=String(day(si.getSpaTime()));
  if (day(si.getSpaTime())<10) d = "0"+d;
  String h=String(hour(si.getSpaTime()));
  if (hour(si.getSpaTime())<10) h = "0"+h;
  String min=String(minute(si.getSpaTime()));
  if (minute(si.getSpaTime())<10) min = "0"+min;
  String s=String(second(si.getSpaTime()));
  if (second(si.getSpaTime())<10) s = "0"+s;

  json["status"]["datetime"]=y+"-"+m+"-"+d+" "+h+":"+min+":"+s;

  json["blower"]["state"] = si.getOutlet_Blower()==2? "OFF" : "ON";
  json["blower"]["mode"] = si.getOutlet_Blower()==1? "Ramp" : "Variable";
  json["blower"]["speed"] = si.getOutlet_Blower() ==2? "0" : String(si.getVARIValue());

  int member = 0;
  for (const auto& pair : si.sleepBitmap) {
      if (pair == si.getL_1SNZ_DAY()) {
        json["sleepTimers"]["timer1"]["state"]=si.sleepSelection[member];
        debugD("SleepTimer1: %s", si.sleepSelection[member].c_str());
      }
      if (pair == si.getL_2SNZ_DAY()) {
        json["sleepTimers"]["timer2"]["state"]=si.sleepSelection[member];
        debugD("SleepTimer2: %s", si.sleepSelection[member].c_str());
      }
      member++;
  }
  json["sleepTimers"]["timer1"]["begin"]=convertToTime(si.getL_1SNZ_BGN());
  json["sleepTimers"]["timer1"]["end"]=convertToTime(si.getL_1SNZ_END());
  json["sleepTimers"]["timer2"]["begin"]=convertToTime(si.getL_2SNZ_BGN());
  json["sleepTimers"]["timer2"]["end"]=convertToTime(si.getL_2SNZ_END());

  json["lights"]["speed"] = si.getLSPDValue();
  json["lights"]["state"] = si.getRB_TP_Light()? "ON": "OFF";
  json["lights"]["effect"] = si.colorModeStrings[si.getColorMode()];
  json["lights"]["brightness"] = si.getLBRTValue();

  // 0 = white, if white, then set the hue and saturation to white so the light displays correctly in HA.
  if (si.getColorMode() == 0) {
    json["lights"]["color"]["h"] = 0;
    json["lights"]["color"]["s"] = 0;
  } else {
    int hue = 4;
    for (uint count = 0; count < sizeof(si.colorMap); count++){
      if (si.colorMap[count] == si.getCurrClr()) {
        hue = count * 15;
      }
    }
    json["lights"]["color"]["h"] = hue;
    json["lights"]["color"]["s"] = 100;
  }
  json["lights"]["color_mode"] = "hs";

  int jsonSize;
  if (prettyJson) {
    jsonSize = serializeJsonPretty(json, output);
  } else {
    jsonSize = serializeJson(json, output);
  }
  // serializeJson returns the size of the json output. If this is greater than zero we consider this successful
  return (jsonSize > 0);
}

bool parseVersion(const String version, int parsedVersion[3]) {
  if (!version.startsWith("v")) {
    return false;
  }

  String numPart = version.substring(1); // Remove 'v'
  int start = 0, end = 0, index = 0;

  while (index < 3 && (end = numPart.indexOf('.', start)) != -1) {
    parsedVersion[index++] = numPart.substring(start, end).toInt();
    start = end + 1;
  }

  if (index < 3) {
    parsedVersion[index++] = numPart.substring(start).toInt();
  }

  // Fill remaining parts with 0
  while (index < 3) {
    parsedVersion[index++] = 0;
  }

  return true;
}

int compareVersions(const int current[3], const int latest[3]) {
  for (int i = 0; i < 3; i++) {
    if (current[i] < latest[i]) return -1;
    if (current[i] > latest[i]) return 1;
  }
  return 0;
}

void firmwareCheckUpdates(Config &config) {
  HttpContent httpContent;
  String content;
  if (httpContent.fetchHttpContent(RELEASES_URL, content)) {
    JsonDocument doc;
    deserializeJson(doc, content);
    String latestRelease = doc["tag_name"].as<String>();

    if (latestRelease.isEmpty()) {
      debugE("Failed to fetch the latest release.");
      return;
    }

    debugD("Latest release: %s\n", latestRelease.c_str());
    config.latestVersion.setValue(latestRelease);
    config.releaseNotes.setValue(doc["body"].as<String>());
    config.releaseUrl.setValue(doc["html_url"].as<String>());

    String firmwareFilename = "firmware_" xstr(PIOENV) "_ota.bin";

    // Search for the correct asset
    JsonArray assets = doc["assets"].as<JsonArray>();
    for (JsonObject asset : assets) {
      String name = asset["name"].as<String>();
      if (name == firmwareFilename) {
        String downloadUrl = asset["browser_download_url"].as<String>();
        config.firmwareUrl.setValue(downloadUrl); // Store the firmware URL
        debugD("Firmware URL found: %s\n", downloadUrl.c_str());
      } else if (name == spiffsFilename) {
        String downloadUrl = asset["browser_download_url"].as<String>();
        config.spiffsUrl.setValue(downloadUrl); // Store the SPIFFS URL
        debugD("SPIFFS URL found: %s\n", downloadUrl.c_str());
      }
    }

    int latestVersion[3] = {0};
    int currentVersion[3] = {0};

    if (!parseVersion(latestRelease, latestVersion)) {
      debugE("Failed to parse latest release version.");
      return;
    }

    if (!parseVersion(xstr(BUILD_INFO), currentVersion)) {
      debugE("Failed to parse current build version.");
      return;
    }

    int comparison = compareVersions(currentVersion, latestVersion);
    config.updateAvailable.setValue((comparison < 0?1:0));
    if (comparison < 0) {
      debugD("New version available: %s\n", latestRelease.c_str());
    } else if (comparison == 0) {
      debugD("You are using the latest version.");
    } else {
      debugD("You are ahead of the latest release!");
    }
  }
}
