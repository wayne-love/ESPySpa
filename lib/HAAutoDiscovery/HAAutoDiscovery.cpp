#include "HAAutoDiscovery.h"

void generateCommonAdJSON(
   JsonDocument& json,
   const AutoDiscoveryInformationTemplate& config,
   const SpaADInformationTemplate& spa,
   String& discoveryTopic,
   String type
   ) {
/*
{ 
   "device_class":"temperature",
   "state_topic":"homeassistant/sensor/sensorBedroom/state",
   "unit_of_measurement":"°C",
   "value_template":"{{ value_json.temperature}}",
   "unique_id":"temp01ae",
   "device":{
      "identifiers":[
         "bedroom01ae"
      ],
      "name":"Bedroom"
   }
}
*/

   // Common fields for all types
   json["name"] = config.displayName;
   json["state_topic"] = spa.stateTopic;
   json["value_template"] = config.valueTemplate;
   json["unique_id"] = spa.spaSerialNumber + "-" + config.propertyId;
   json["device"]["identifiers"][0] = spa.spaSerialNumber;
   json["device"]["serial_number"] = spa.spaSerialNumber;
   json["device"]["name"] = spa.spaName;
   json["device"]["manufacturer"] = spa.manufacturer;
   json["device"]["model"] = spa.model;
   json["device"]["sw_version"] = spa.sw_version;
   json["device"]["configuration_url"] = spa.configuration_url;
   json["availability"]["topic"] = spa.availabilityTopic;
   if (!config.deviceClass.isEmpty()) json["device_class"] = config.deviceClass;
   if (!config.entityCategory.isEmpty()) json["entity_category"] = config.entityCategory;

   discoveryTopic = "homeassistant/" + type + "/" + spa.spaSerialNumber + "/" + spa.spaSerialNumber + "-" + config.propertyId + "/config";

}

void generateSensorAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic, String stateClass, String unitOfMeasure) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "sensor");

   if (!unitOfMeasure.isEmpty()) json["unit_of_measurement"] = unitOfMeasure;
   if (!stateClass.isEmpty()) json["state_class"] = stateClass;

   serializeJson(json, output);
}

void generateBinarySensorAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "binary_sensor");

   serializeJson(json, output);
}

void generateTextAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic, String regex) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "text");

   json["command_topic"] = spa.commandTopic + "/" + config.propertyId;
   if (!regex.isEmpty()) json["pattern"] = regex;

   serializeJson(json, output);
}

void generateSwitchAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "switch");

   json["command_topic"] = spa.commandTopic + "/" + config.propertyId;

   serializeJson(json, output);
}

void generateUpdateAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "update");

   json["command_topic"] = spa.commandTopic + "/" + config.propertyId;
   json["payload_install"] = "latest";

   serializeJson(json, output);
}

void generateFanAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic, int min, int max, const String* modes, const size_t modesSize) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "fan");

   // Find the last character that is not a space or curly brace
   int lastIndex = config.valueTemplate.length() - 1;
   while (lastIndex >= 0 && (config.valueTemplate[lastIndex] == ' ' || config.valueTemplate[lastIndex] == '}')) {
      lastIndex--;
   }
   json["state_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".state" + config.valueTemplate.substring(lastIndex + 1);
   json["command_topic"] = spa.commandTopic + "/" + config.propertyId + "_state";

    if (max > min) {
        json["percentage_state_topic"] = spa.stateTopic;
        json["percentage_command_topic"] = spa.commandTopic + "/" + config.propertyId + "_speed";
        json["percentage_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".speed" + config.valueTemplate.substring(lastIndex + 1);

        json["speed_range_min"]=min;
        json["speed_range_max"]=max;
    }

   if (modes != nullptr && modesSize > 0) {
      json["preset_mode_state_topic"] = spa.stateTopic;
      json["preset_mode_command_topic"] = spa.commandTopic + "/" + config.propertyId + "_mode";
      json["preset_mode_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".mode" + config.valueTemplate.substring(lastIndex + 1);

      JsonArray jsonModes = json["preset_modes"].to<JsonArray>();
      //for (const auto& mode : *modes) jsonModes.add(mode);
      for (size_t i = 0; i < modesSize; ++i) jsonModes.add(modes[i]);
   }

    if (config.propertyId.startsWith("pump")) {
      json["icon"] = "mdi:pump";
   }

   serializeJson(json, output);
}

void generateClimateAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "climate");

   // Find the last character that is not a space or curly brace
   int lastIndex = config.valueTemplate.length() - 1;
   while (lastIndex >= 0 && (config.valueTemplate[lastIndex] == ' ' || config.valueTemplate[lastIndex] == '}')) {
      lastIndex--;
   }

   json["current_temperature_topic"] = spa.stateTopic;
   json["current_temperature_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".water" + config.valueTemplate.substring(lastIndex + 1);

   json["initial"]=36;
   json["max_temp"]=41;
   json["min_temp"]=10;
   JsonArray modes = json["modes"].to<JsonArray>();
   modes.add("auto"); // the actual modes of the heat pump are controlled through a select control to avoid accidently turning off the HP and using the resistance heater
   json["mode_state_template"]="auto";
   json["mode_state_topic"] = spa.stateTopic;

   json["action_topic"] = spa.stateTopic;
   json["action_template"]="{% if value_json.status.heatingActive == 'ON' %}heating{% else %}off{% endif %}";

   json["temperature_command_topic"] = spa.commandTopic + "/temperatures_setPoint";
   json["temperature_state_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".setPoint" + config.valueTemplate.substring(lastIndex + 1);
   json["temperature_state_topic"] = spa.stateTopic;
   json["temperature_unit"]="C";
   json["temp_step"]=0.2;

   serializeJson(json, output);
}
