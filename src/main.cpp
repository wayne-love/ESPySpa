#include <WiFi.h>
#include <WebServer.h>

#include <WiFiClient.h>
#include <RemoteDebug.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#if defined(LED_PIN)
  #include "Blinker.h"
#endif

#include "WebUI.h"
#include "Config.h"
#include "SpaInterface.h"
#include "SpaUtils.h"
#include "HAAutoDiscovery.h"

//define stringify function
#define xstr(a) str(a)
#define str(a) #a

unsigned long bootStartMillis;  // To track when the device started
RemoteDebug Debug;

SpaInterface si;
Config config;

#if defined(LED_PIN)
  Blinker led(LED_PIN);
#endif

WiFiClient wifi;
PubSubClient mqttClient(wifi);

WebUI ui(&si, &config);



bool WMsaveConfig = false;
ulong mqttLastConnect = 0;
ulong wifiLastConnect = millis();
ulong bootTime = millis();
ulong statusLastPublish = millis();
bool delayedStart = true; // Delay spa connection for 10sec after boot to allow for external debugging if required.
bool autoDiscoveryPublished = false;

String mqttBase = "";
String mqttStatusTopic = "";
String mqttSet = "";
String mqttAvailability = "";
String mqttServer = "";
String mqttUsername = "";
String mqttPassword = "";

String spaSerialNumber = "";

bool updateMqtt = false;

void WMsaveConfigCallback(){
  WMsaveConfig = true;
}

void startWiFiManager(){
  if (ui.initialised) {
    ui.server->stop();
  }

  WiFiManager wm;
  WiFiManagerParameter custom_spa_name("spa_name", "Spa Name", config.SpaName.getValue().c_str(), 40);
  WiFiManagerParameter custom_mqtt_server("server", "MQTT server", config.MqttServer.getValue().c_str(), 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT port", String(config.MqttPort.getValue()).c_str(), 6);
  WiFiManagerParameter custom_mqtt_username("username", "MQTT Username", config.MqttUsername.getValue().c_str(), 20 );
  WiFiManagerParameter custom_mqtt_password("password", "MQTT Password", config.MqttPassword.getValue().c_str(), 40 );
  wm.addParameter(&custom_spa_name);
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_username);
  wm.addParameter(&custom_mqtt_password);
  wm.setBreakAfterConfig(true);
  wm.setSaveConfigCallback(WMsaveConfigCallback);
  wm.setConnectTimeout(300); //close the WiFiManager after 300 seconds of inactivity


  wm.startConfigPortal();
  debugI("Exiting Portal");

  if (WMsaveConfig) {
    config.SpaName.setValue(String(custom_spa_name.getValue()));
    config.MqttServer.setValue(String(custom_mqtt_server.getValue()));
    config.MqttPort.setValue(String(custom_mqtt_port.getValue()).toInt());
    config.MqttUsername.setValue(String(custom_mqtt_username.getValue()));
    config.MqttPassword.setValue(String(custom_mqtt_password.getValue()));

    config.writeConfigFile();
  }
}

// We check the EN_PIN every loop, to allow people to configure the system
void checkButton(){
#if defined(EN_PIN)
  if(digitalRead(EN_PIN) == LOW) {
    debugI("Initial buttong press detected");
    delay(100); // wait and then test again to ensure that it is a held button not a press
    if(digitalRead(EN_PIN) == LOW) {
      debugI("Button press detected. Starting Portal");
      startWiFiManager();

      ESP.restart();  // restart, dirty but easier than trying to restart services one by one
    }
  }
#endif
}
void startWifiManagerCallback() {
  debugD("Starting Wi-Fi Manager...");
  startWiFiManager();
  ESP.restart(); //do we need to reboot here??
}

void configChangeCallbackString(const char* name, String value) {
  debugD("%s: %s", name, value);
  if (strcmp(name, "MqttServer") == 0) {
    mqttServer = value;
    updateMqtt = true;
  }
  else if (strcmp(name, "MqttPort") == 0) {
    updateMqtt = true;
  }
  else if (strcmp(name, "MqttUsername") == 0) {
    mqttUsername = value;
    updateMqtt = true;
  }
  else if (strcmp(name, "MqttPassword") == 0) {
    mqttPassword = value;
    updateMqtt = true;
  }
  else if (strcmp(name, "SpaName") == 0) { } //TODO - Changing the SpaName currently requires the user to:
                                  // delete the entities in MQTT then reboot the ESP
}

void configChangeCallbackInt(const char* name, int value) {
  debugD("%s: %i", name, value);
  if (strcmp(name, "UpdateFrequency") == 0) si.setUpdateFrequency(value);
}

void mqttHaAutoDiscovery() {
  debugI("Publishing Home Assistant auto discovery");

  String output;
  String discoveryTopic;

  SpaADInformationTemplate spa;
  spa.spaName = config.SpaName.getValue();
  spa.spaSerialNumber = spaSerialNumber;
  spa.stateTopic = mqttStatusTopic;
  spa.availabilityTopic = mqttAvailability;
  spa.manufacturer = "sn_esp32";
  spa.model = xstr(PIOENV);
  spa.sw_version = xstr(BUILD_INFO);
  spa.configuration_url = "http://" + wifi.localIP().toString();

  //sensorADPublish("Water Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.water }}","measurement","WaterTemperature", spaName, spaSerialNumber);
  //sensorADPublish("Heater Temperature","diagnostic","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.heater }}","measurement","HeaterTemperature", spaName, spaSerialNumber);
  //sensorADPublish("Case Temperature","diagnostic","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.case }}","measurement","CaseTemperature", spaName, spaSerialNumber);
  //sensorADPublish("Mains Voltage","diagnostic","voltage",mqttStatusTopic,"V","{{ value_json.power.voltage }}","measurement","MainsVoltage", spaName, spaSerialNumber);
  //sensorADPublish("Mains Current","diagnostic","current",mqttStatusTopic,"A","{{ value_json.power.current }}","measurement","MainsCurrent", spaName, spaSerialNumber);
  //sensorADPublish("Power","","energy",mqttStatusTopic,"W","{{ value_json.power.energy }}","measurement","Power", spaName, spaSerialNumber);
  //sensorADPublish("Total Energy","","energy",mqttStatusTopic,"Wh","{{ value_json.totalenergy }}","total_increasing","TotalEnergy", spaName, spaSerialNumber);
  //sensorADPublish("Heatpump Ambient Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.heatpumpAmbient }}","measurement","HPAmbTemp", spaName, spaSerialNumber);
  //sensorADPublish("Heatpump Condensor Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.heatpumpCondensor }}","measurement","HPCondTemp", spaName, spaSerialNumber);
  //sensorADPublish("State","","",mqttStatusTopic,"","{{ value_json.status.state }}","","State", spaName, spaSerialNumber);
  spa.commandTopic = mqttSet;
  
  AutoDiscoveryInformationTemplate ADConf;

  ADConf.displayName = "Water Temperature";
  ADConf.valueTemplate = "{{ value_json.temperatures.water }}";
  ADConf.propertyId = "WaterTemperature";
  ADConf.deviceClass = "temperature";
  ADConf.entityCategory = "";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Case Temperature";
  ADConf.valueTemplate = "{{ value_json.temperatures.case }}";
  ADConf.propertyId = "CaseTemperature";
  ADConf.deviceClass = "temperature";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Heater Temperature";
  ADConf.valueTemplate = "{{ value_json.temperatures.heater }}";
  ADConf.propertyId = "HeaterTemperature";
  ADConf.deviceClass = "temperature";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Mains Voltage";
  ADConf.valueTemplate = "{{ value_json.power.voltage }}";
  ADConf.propertyId = "MainsVoltage";
  ADConf.deviceClass = "voltage";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "V");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Mains Current";
  ADConf.valueTemplate = "{{ value_json.power.current }}";
  ADConf.propertyId = "MainsCurrent";
  ADConf.deviceClass = "current";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "A");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Power";
  ADConf.valueTemplate = "{{ value_json.power.power }}";
  ADConf.propertyId = "Power";
  ADConf.deviceClass = "power";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "W");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Total Energy";
  ADConf.valueTemplate = "{{ value_json.power.totalenergy }}";
  ADConf.propertyId = "TotalEnergy";
  ADConf.deviceClass = "energy";
  ADConf.entityCategory = "diagnostic";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "kWh");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "State";
  ADConf.valueTemplate = "{{ value_json.status.state }}";
  ADConf.propertyId = "State";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateSensorAdJSON(output, ADConf, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //binarySensorADPublish("Heating Active","",mqttStatusTopic,"{{ value_json.status.heatingActive }}","HeatingActive", spaName, spaSerialNumber);
  //binarySensorADPublish("Ozone Active","",mqttStatusTopic,"{{ value_json.status.ozoneActive }}","OzoneActive", spaName, spaSerialNumber);
  ADConf.displayName = "Heating Active";
  ADConf.valueTemplate = "{{ value_json.status.heatingActive }}";
  ADConf.propertyId = "HeatingActive";
  ADConf.deviceClass = "heat";
  ADConf.entityCategory = "";
  generateBinarySensorAdJSON(output, ADConf, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Ozone Active";
  ADConf.valueTemplate = "{{ value_json.status.ozoneActive }}";
  ADConf.propertyId = "OzoneActive";
  ADConf.deviceClass = "running";
  ADConf.entityCategory = "";
  generateBinarySensorAdJSON(output, ADConf, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //climateADPublish(mqttClient, spa, spaName, "{{ value_json.temperatures }}", "Heating");
  ADConf.displayName = "";
  ADConf.valueTemplate = "{{ value_json.temperatures }}";
  ADConf.propertyId = "Heating";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateClimateAdJSON(output, ADConf, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);


  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  if (si.Pump1InstallState.getValue().startsWith("1") && !(si.Pump1InstallState.getValue().endsWith("4"))) {
     //switchADPublish(mqttClient, spa, "Pump 1", "{{ value_json.pumps.pump1.state }}", "pump1");
    ADConf.displayName = "Pump 1";
    ADConf.valueTemplate = "{{ value_json.pumps.pump1.state }}";
    ADConf.propertyId = "pump1";
    generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.Pump2InstallState.getValue().startsWith("1") && !(si.Pump2InstallState.getValue().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 2","{{ value_json.pumps.pump2.state }}", "pump2");
    ADConf.displayName = "Pump 2";
    ADConf.valueTemplate = "{{ value_json.pumps.pump2.state }}";
    ADConf.propertyId = "pump2";
    generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.Pump3InstallState.getValue().startsWith("1") && !(si.Pump3InstallState.getValue().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 3", "{{ value_json.pumps.pump3.state }}", "pump3");
    ADConf.displayName = "Pump 3";
    ADConf.valueTemplate = "{{ value_json.pumps.pump3.state }}";
    ADConf.propertyId = "pump3";
    generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.Pump4InstallState.getValue().startsWith("1") && !(si.Pump4InstallState.getValue().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 4", "{{ value_json.pumps.pump4.state }}", "pump4");
    ADConf.displayName = "Pump 4";
    ADConf.valueTemplate = "{{ value_json.pumps.pump4.state }}";
    ADConf.propertyId = "pump4";
    generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.Pump5InstallState.getValue().startsWith("1") && !(si.Pump5InstallState.getValue().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 5", "{{ value_json.pumps.pump5.state }}", "pump5");
    ADConf.displayName = "Pump 5";
    ADConf.valueTemplate = "{{ value_json.pumps.pump5.state }}";
    ADConf.propertyId = "pump5";
    generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  }

  if (si.HP_Present.getValue()) {
    ADConf.displayName = "Heatpump Ambient Temperature";
    ADConf.valueTemplate = "{{ value_json.temperatures.heatpumpAmbient }}";
    ADConf.propertyId = "HPAmbTemp";
    ADConf.deviceClass = "temperature";
    ADConf.entityCategory = "diagnostic";
    generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "°C");
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

    ADConf.displayName = "Heatpump Condensor Temperature";
    ADConf.valueTemplate = "{{ value_json.temperatures.heatpumpCondensor }}";
    ADConf.propertyId = "HPCondTemp";
    ADConf.deviceClass = "temperature";
    ADConf.entityCategory = "diagnostic";
    generateSensorAdJSON(output, ADConf, spa, discoveryTopic, "measurement", "°C");
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

    //selectADPublish(mqttClient, spa, "Heatpump Mode", "{{ value_json.heatpump.mode }}", "heatpump_mode", "", "", {"Auto","Heat","Cool","Off"});
    ADConf.displayName = "Heatpump Mode";
    ADConf.valueTemplate = "{{ value_json.heatpump.mode }}";
    ADConf.propertyId = "heatpump_mode";
    ADConf.deviceClass = "";
    ADConf.entityCategory = "";
    generateSelectAdJSON(output, ADConf, spa, discoveryTopic, si.HPMPStrings);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

    //switchADPublish(mqttClient, spa, "Aux Heat Element", "{{ value_json.heatpump.auxheat }}", "heatpump_auxheat");
    ADConf.displayName = "Aux Heat Element";
    ADConf.valueTemplate = "{{ value_json.heatpump.auxheat }}";
    ADConf.propertyId = "heatpump_auxheat";
    ADConf.deviceClass = "";
    ADConf.entityCategory = "";
    generateSwitchAdJSON(output, ADConf, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  //lightADPublish(mqttClient, spa, "Lights", "{{ value_json.lights }}", "lights", "", "", colorModeStrings);
  ADConf.displayName = "Lights";
  ADConf.valueTemplate = "{{ value_json.lights }}";
  ADConf.propertyId = "lights";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateLightAdJSON(output, ADConf, spa, discoveryTopic, si.colorModeStrings);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //selectADPublish(mqttClient, spa, "Lights Speed","{{ value_json.lights.speed }}","lights_speed", "", "", {"1","2","3","4","5"});
  ADConf.displayName = "Lights Speed";
  ADConf.valueTemplate = "{{ value_json.lights.speed }}";
  ADConf.propertyId = "lights_speed";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateSelectAdJSON(output, ADConf, spa, discoveryTopic, si.lightSpeedMap );
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //selectADPublish(mqttClient, spa, "Sleep Timer 1","{{ value_json.sleepTimers.timer1.state }}", "sleepTimers_1_state", "config", "", sleepStrings);
  //selectADPublish(mqttClient, spa, "Sleep Timer 2","{{ value_json.sleepTimers.timer2.state }}", "sleepTimers_2_state", "config", "", sleepStrings);
  ADConf.displayName = "Sleep Timer 1";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer1.state }}";
  ADConf.propertyId = "sleepTimers_1_state";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateSelectAdJSON(output, ADConf, spa, discoveryTopic, si.sleepSelection);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Sleep Timer 2";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer2.state }}";
  ADConf.propertyId = "sleepTimers_2_state";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateSelectAdJSON(output, ADConf, spa, discoveryTopic, si.sleepSelection);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Date Time";
  ADConf.valueTemplate = "{{ value_json.status.datetime }}";
  ADConf.propertyId = "status_datetime";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateTextAdJSON(output, ADConf, spa, discoveryTopic, "[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Sleep Timer 1 Begin";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer1.begin }}";
  ADConf.propertyId = "sleepTimers_1_begin";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateTextAdJSON(output, ADConf, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Sleep Timer 1 End";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer1.end }}";
  ADConf.propertyId = "sleepTimers_1_end";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateTextAdJSON(output, ADConf, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Sleep Timer 2 Begin";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer2.begin }}";
  ADConf.propertyId = "sleepTimers_2_begin";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateTextAdJSON(output, ADConf, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  ADConf.displayName = "Sleep Timer 2 End";
  ADConf.valueTemplate = "{{ value_json.sleepTimers.timer2.end }}";
  ADConf.propertyId = "sleepTimers_2_end";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "config";
  generateTextAdJSON(output, ADConf, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //fanADPublish(mqttClient, spa, "Blower", "{{ value_json.blower }}", "blower");
  ADConf.displayName = "Blower";
  ADConf.valueTemplate = "{{ value_json.blower }}";
  ADConf.propertyId = "blower";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateFanAdJSON(output, ADConf, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //selectADPublish(mqttClient, spa, "Spa Mode", "{{ value_json.status.spaMode }}", "status_spaMode", "", "", spaModeStrings);
  ADConf.displayName = "Spa Mode";
  ADConf.valueTemplate = "{{ value_json.status.spaMode }}";
  ADConf.propertyId = "status_spaMode";
  ADConf.deviceClass = "";
  ADConf.entityCategory = "";
  generateSelectAdJSON(output, ADConf, spa, discoveryTopic, si.spaModeStrings);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

}

#pragma region MQTT Publish / Subscribe

void mqttPublishStatusString(String s){

  mqttClient.publish(String(mqttBase+"rfResponse").c_str(),s.c_str());

}

void mqttPublishStatus() {
  String json;
  if (generateStatusJson(si, json)) {
    mqttClient.publish(mqttStatusTopic.c_str(),json.c_str());
  } else {
    debugD("Error generating json");
  }
}


void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);

  String p = "";
  for (uint x = 0; x < length; x++) {
    p += char(*payload);
    payload++;
  }

  debugD("MQTT subscribe received '%s' with payload '%s'",topic,p.c_str());

  String property = t.substring(t.lastIndexOf("/")+1);

  debugI("Received update for %s to %s",property.c_str(),p.c_str());

  if (property == "temperatures_setPoint") {
    si.STMP.sendValue(int(p.toFloat()*10));
  } else if (property == "heatpump_mode") {
    si.HPMP.sendValue(p);
  } else if (property == "pump1") {
    si.RB_TP_Pump1.sendValue(p=="OFF"?0:1);
  } else if (property == "pump2") {
    si.RB_TP_Pump2.sendValue(p=="OFF"?0:1);
  } else if (property == "pump3") {
    si.RB_TP_Pump3.sendValue(p=="OFF"?0:1);
  } else if (property == "pump4") {
    si.RB_TP_Pump4.sendValue(p=="OFF"?0:1);
  } else if (property == "pump5") {
    si.RB_TP_Pump5.sendValue(p=="OFF"?0:1);
  } else if (property == "heatpump_auxheat") {
    si.HELE.sendValue(p=="OFF"?0:1);
  } else if (property == "status_datetime") {
    tmElements_t tm;
    tm.Year=CalendarYrToTm(p.substring(0,4).toInt());
    tm.Month=p.substring(5,7).toInt();
    tm.Day=p.substring(8,10).toInt();
    tm.Hour=p.substring(11,13).toInt();
    tm.Minute=p.substring(14,16).toInt();
    tm.Second=p.substring(17).toInt();
    si.SpaTime.sendValue(makeTime(tm));
  } else if (property == "lights_state") {
    si.RB_TP_Light.sendValue(p=="ON"?1:0);
  } else if (property == "lights_effect") {
    si.ColorMode.sendValue(p);
  } else if (property == "lights_brightness") {
    si.LBRTValue.sendValue(p.toInt());
  } else if (property == "lights_color") {
    int pos = p.indexOf(',');
    if ( pos > 0) {
      int value = p.substring(0, pos).toInt();
      si.CurrClr.sendValue(si.colorMap[value/15]);
    }
  } else if (property == "lights_speed") {
    si.LSPDValue.sendValue(p);
  } else if (property == "blower_state") {
    si.Outlet_Blower.sendValue(p=="OFF"?2:0);
  } else if (property == "blower_speed") {
    if (p=="0") si.Outlet_Blower.sendValue(2);
    else si.VARIValue.sendValue(p.toInt());
  } else if (property == "blower_mode") {
    si.Outlet_Blower.sendValue(p=="Variable"?0:1);
  } else if (property == "sleepTimers_1_state" || property == "sleepTimers_2_state") {
    int member=0;
    for (const auto& i : si.sleepSelection) {
      if (i == p) {
        if (property == "sleepTimers_1_state")
          si.L_1SNZ_DAY.sendValue(si.sleepBitmap[member]);
        else if (property == "sleepTimers_2_state")
          si.L_2SNZ_DAY.sendValue(si.sleepBitmap[member]);
        break;
      }
      member++;
    }
  } else if (property == "sleepTimers_1_begin") {
    si.L_1SNZ_BGN.sendValue(convertToInteger(p));
  } else if (property == "sleepTimers_1_end") {
    si.L_1SNZ_END.sendValue(convertToInteger(p));
  } else if (property == "sleepTimers_2_begin") {
    si.L_2SNZ_BGN.sendValue(convertToInteger(p));
  } else if (property == "sleepTimers_2_end") {
    si.L_2SNZ_END.sendValue(convertToInteger(p));
  } else if (property == "status_spaMode") {
    si.Mode.sendValue(p);
  } else {
    debugE("Unhandled property - %s",property.c_str());
  }
}


#pragma endregion

void setup() {
  #if defined(EN_PIN)
    pinMode(EN_PIN, INPUT_PULLUP);
  #endif

  delay(200);
  debugA("Starting... %s", WiFi.getHostname());

  WiFi.mode(WIFI_STA); 
  WiFi.begin();

  Debug.begin(WiFi.getHostname());
  Debug.setResetCmdEnabled(true);
  Debug.showProfiler(true);

  debugI("Mounting FS");

  if (!LittleFS.begin()) {
    debugW("Failed to mount file system, formatting");
    LittleFS.format();
    LittleFS.begin();
  }

  if (!config.readConfigFile()) {
    debugW("Failed to open config.json, starting Wi-Fi Manager");
    startWiFiManager();
    //I'm not sure if we need a reboot here - probably not
  }

  mqttServer = config.MqttServer.getValue();
  mqttUsername = config.MqttUsername.getValue();
  mqttPassword = config.MqttPassword.getValue();
  mqttClient.setServer(mqttServer.c_str(), config.MqttPort.getValue());
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(2048);

  bootStartMillis = millis();  // Record the current boot time in milliseconds

  ui.begin();
  ui.setWifiManagerCallback(startWifiManagerCallback);
  si.setUpdateFrequency(config.UpdateFrequency.getValue());

  config.setCallback(configChangeCallbackString);
  config.setCallback(configChangeCallbackInt);

}



void loop() {  



  checkButton();
  #if defined(LED_PIN)
    led.tick();
  #endif
  mqttClient.loop();
  Debug.handle();

  if (ui.initialised) { 
    ui.server->handleClient(); 
  }

  if (updateMqtt) {
    debugD("Changing MQTT settings...");
    mqttClient.disconnect();
    mqttClient.setServer(mqttServer.c_str(), config.MqttPort.getValue());
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(2048);
    updateMqtt = false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    //wifi not connected
    #if defined(LED_PIN)
      led.setInterval(100);
    #endif

    if (millis()-wifiLastConnect > 10000) {
      debugI("Wifi reconnecting...");
      wifiLastConnect = millis();
      WiFi.reconnect();
    }
  } else {
    if (delayedStart) {
      delayedStart = !(bootTime + 10000 < millis());
    } else {

      si.loop();

      if (si.isInitialised()) {
        if ( spaSerialNumber=="" ) {
          debugI("Initialising...");
      
          spaSerialNumber = si.SerialNo1.getValue()+"-"+si.SerialNo2.getValue();
          debugI("Spa serial number is %s",spaSerialNumber.c_str());

          mqttBase = String("sn_esp32/") + spaSerialNumber + String("/");
          mqttStatusTopic = mqttBase + "status";
          mqttSet = mqttBase + "set";
          mqttAvailability = mqttBase+"available";
          debugI("MQTT base topic is %s",mqttBase.c_str());
        }
        if (!mqttClient.connected()) {  // MQTT broker reconnect if not connected
          long now=millis();
          if (now - mqttLastConnect > 1000) {
            #if defined(LED_PIN)
              led.setInterval(500);
            #endif
            debugW("MQTT not connected, attempting connection to %s:%i", mqttServer.c_str(), config.MqttPort.getValue());
            mqttLastConnect = now;


            if (mqttClient.connect("sn_esp32", mqttUsername.c_str(), mqttPassword.c_str(), mqttAvailability.c_str(),2,true,"offline")) {
              debugI("MQTT connected");
    
              String subTopic = mqttBase+"set/#";
              debugI("Subscribing to topic %s", subTopic.c_str());
              mqttClient.subscribe(subTopic.c_str());

              mqttClient.publish(mqttAvailability.c_str(),"online",true);
              autoDiscoveryPublished = false;
            } else {
              debugW("MQTT connection failed");
            }

          }
        } else {
          if (!autoDiscoveryPublished) {  // This is the setup area, gets called once when communication with Spa and MQTT broker have been established.
            debugI("Publish autodiscovery information");
            mqttHaAutoDiscovery();
            autoDiscoveryPublished = true;
            si.setUpdateCallback(mqttPublishStatus);
            mqttPublishStatus();

            si.statusResponse.setUpdateCallback(mqttPublishStatusString);

          }
          #if defined(LED_PIN)
            led.setInterval(2000);
          #endif
        }
      }
    }
  }
}