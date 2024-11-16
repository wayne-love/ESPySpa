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

bool getPumpModes(SpaInterface &si, int pumpNumber, JsonObject pumps) {
  // Validate the pump number
  if (pumpNumber < 1 || pumpNumber > 5) {
    return false;
  }

  // Retrieve the pump install state dynamically
  String pumpState = si.pumpStates[pumpNumber - 1]->getValue();

  // Split pumpState by "-"
  int firstDash = pumpState.indexOf("-");
  int secondDash = pumpState.indexOf("-", firstDash + 1);

  char pumpKey[6] = "pump";  // Start with "pump"
  pumpKey[4] = '0' + pumpNumber;  // Append the pump number as a character
  pumpKey[5] = '\0';  // Null-terminate the string

  // Convert installed to a boolean (true for "1", false for "0")
  pumps[pumpKey]["installed"] = (pumpState.substring(0, firstDash) == "1");
  pumps[pumpKey]["speedType"] = pumpState.substring(firstDash + 1, secondDash);

  String possibleStates = pumpState.substring(secondDash + 1);
  // Convert possibleStates into words and store them in a JSON array
  for (uint i = 0; i < possibleStates.length(); i++) {
    char stateChar = possibleStates.charAt(i);
    if (stateChar == '0') {
      pumps[pumpKey]["possibleStates"].add("OFF");
    } else if (stateChar == '1') {
      pumps[pumpKey]["possibleStates"].add("ON");
    } else if (stateChar == '4') {
      pumps[pumpKey]["possibleStates"].add("AUTO");
    }
  }

  return true;
}

bool generateStatusJson(SpaInterface &si, String &output, bool prettyJson) {
  JsonDocument json;

  json["temperatures"]["setPoint"] = si.STMP.getValue() / 10.0;
  json["temperatures"]["water"] = si.WTMP.getValue() / 10.0;
  json["temperatures"]["heater"] = si.HeaterTemperature.getValue() / 10.0;
  json["temperatures"]["case"] = si.CaseTemperature.getValue();
  json["temperatures"]["heatpumpAmbient"] = si.HP_Ambient.getValue();
  json["temperatures"]["heatpumpCondensor"] = si.HP_Condensor.getValue();

  json["power"]["voltage"] = si.MainsVoltage.getValue();
  json["power"]["current"]= si.MainsCurrent.getValue() / 10.0;
  json["power"]["power"] = si.Power.getValue() / 10.0;
  json["power"]["totalenergy"]= si.Power_kWh.getValue() / 100.0; // convert to kWh to Wh.

  json["status"]["heatingActive"] = si.RB_TP_Heater.getValue()? "ON": "OFF";
  json["status"]["ozoneActive"] = si.RB_TP_Ozone.getValue()? "ON": "OFF";
  json["status"]["state"] = si.Status.getValue();
  json["status"]["spaMode"] = si.Mode.getValue();

  json["heatpump"]["mode"] = si.HPMPStrings[si.HPMP.getValue()];
  json["heatpump"]["auxheat"] = si.HELE.getValue()==0? "OFF" : "ON";

  JsonObject pumps = json["pumps"].to<JsonObject>();
  // Add pump data by calling the function for each pump
  for (int i = 1; i <= 5; i++) {
    if (!getPumpModes(si, i, pumps)) {
      debugD("Invalid pump number: %i", i);
    }
  }

  json["pumps"]["pump1"]["state"] = si.RB_TP_Pump1.getValue()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pumps"]["pump2"]["state"] = si.RB_TP_Pump2.getValue()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pumps"]["pump3"]["state"] = si.RB_TP_Pump3.getValue()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pumps"]["pump4"]["state"] = si.RB_TP_Pump4.getValue()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pumps"]["pump5"]["state"] = si.RB_TP_Pump5.getValue()==0? "OFF" : "ON"; // we're ignoring auto here

  String y=String(year(si.SpaTime.getValue()));
  String m=String(month(si.SpaTime.getValue()));
  if (month(si.SpaTime.getValue())<10) m = "0"+m;
  String d=String(day(si.SpaTime.getValue()));
  if (day(si.SpaTime.getValue())<10) d = "0"+d;
  String h=String(hour(si.SpaTime.getValue()));
  if (hour(si.SpaTime.getValue())<10) h = "0"+h;
  String min=String(minute(si.SpaTime.getValue()));
  if (minute(si.SpaTime.getValue())<10) min = "0"+min;
  String s=String(second(si.SpaTime.getValue()));
  if (second(si.SpaTime.getValue())<10) s = "0"+s;

  json["status"]["datetime"]=y+"-"+m+"-"+d+" "+h+":"+min+":"+s;

  json["blower"]["state"] = si.Outlet_Blower.getValue()==2? "OFF" : "ON";
  json["blower"]["mode"] = si.Outlet_Blower.getValue()==1? "Ramp" : "Variable";
  json["blower"]["speed"] = si.Outlet_Blower.getValue() ==2? "0" : String(si.VARIValue.getValue());

  int member = 0;
  for (const auto& pair : si.sleepBitmap) {
      if (pair == si.L_1SNZ_DAY.getValue()) {
        json["sleepTimers"]["timer1"]["state"]=si.sleepSelection[member];
        debugD("SleepTimer1: %s", si.sleepSelection[member].c_str());
      }
      if (pair == si.L_2SNZ_DAY.getValue()) {
        json["sleepTimers"]["timer2"]["state"]=si.sleepSelection[member];
        debugD("SleepTimer2: %s", si.sleepSelection[member].c_str());
      }
      member++;
  }
  json["sleepTimers"]["timer1"]["begin"]=convertToTime(si.L_1SNZ_BGN.getValue());
  json["sleepTimers"]["timer1"]["end"]=convertToTime(si.L_1SNZ_END.getValue());
  json["sleepTimers"]["timer2"]["begin"]=convertToTime(si.L_2SNZ_BGN.getValue());
  json["sleepTimers"]["timer2"]["end"]=convertToTime(si.L_2SNZ_END.getValue());

  json["lights"]["speed"] = si.LSPDValue.getValue();
  json["lights"]["state"] = si.RB_TP_Light.getValue()? "ON": "OFF";
  json["lights"]["effect"] = si.colorModeStrings[si.ColorMode.getValue()];
  json["lights"]["brightness"] = si.LBRTValue.getValue();

  // 0 = white, if white, then set the hue and saturation to white so the light displays correctly in HA.
  if (si.ColorMode.getValue() == 0) {
    json["lights"]["color"]["h"] = 0;
    json["lights"]["color"]["s"] = 0;
  } else {
    int hue = 4;
    for (uint count = 0; count < sizeof(si.colorMap); count++){
      if (si.colorMap[count] == si.CurrClr.getValue()) {
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

