#ifndef HAAUTODISCOVERY_H
#define HAAUTODISCOVERY_H

#include <Arduino.h>
#include <ArduinoJson.h>


/// @brief Configuration structure for the data elements for the Spa.
struct SpaADInformationTemplate {
    String spaName;             // Spa name (eg MySpa)
    String spaSerialNumber;     // Spa serial number
    String stateTopic;          // MQTT topic where staus informaion is published.
    String availabilityTopic;   // MQTT topic for availability of the Spa (not sensor)
    String commandTopic;        // MQTT topic where command information is sent.
    String manufacturer;        // MQTT topic for device manufacturer.
    String model;               // MQTT topic for device model.
    String sw_version;             // MQTT topic for device software version.
    String configuration_url;   // MQTT topic for config url.
};

/// @brief Base configuration structure for common data elements
struct AutoDiscoveryInformationTemplate {
    String displayName;         // Display name for the sensor
    String valueTemplate;       // Value template to extract the status information
    String propertyId;          // Unique ID of the sensor
    String deviceClass;         // Device class (empty string accepted)
    String entityCategory;      // https://developers.home-assistant.io/blog/2021/10/26/config-entity?_highlight=diagnostic#entity-categories (empty string accepted)
};

/// @brief Generate JSON string to publish for Sensor auto discovery
/// @param output String to revceive JSON output
/// @param config Structure to define entity information
/// @param spa Structure to define Spa information
/// @param discoveryTopic String to retun discovrery topic
/// @param type String to provide the type
void generateCommonAdJSON(JsonDocument& json, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic, String type);

void generateSensorAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic, String stateClass="", String unitOfMeasure="");
void generateBinarySensorAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic);
void generateTextAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic, String regex="");
void generateSwitchAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic);
void generateUpdateAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic);

template <typename T, size_t N>
void generateSelectAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic, const std::array<T, N>& options) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "select");

   json["command_topic"] = spa.commandTopic + "/" + config.propertyId;
   JsonArray opts = json["options"].to<JsonArray>();
   for (const auto& o : options) opts.add(o);

   serializeJson(json, output);
}

void generateFanAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic, int min, int max, const String* modes, const size_t modesSize=0);

template <typename T, size_t N>
void generateLightAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic, const std::array<T, N>& colorModes) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "light");

   json["brightness_state_topic"] = spa.stateTopic;
   json["color_mode_state_topic"] = spa.stateTopic;
   json["effect_state_topic"] = spa.stateTopic;
   json["hs_state_topic"] = spa.stateTopic;

   json["command_topic"] = spa.commandTopic + "/" + config.propertyId + "_state";
   json["brightness_command_topic"] = spa.commandTopic + "/" + config.propertyId + "_brightness";
   json["effect_command_topic"] = spa.commandTopic + "/" + config.propertyId + "_effect";
   json["hs_command_topic"] = spa.commandTopic + "/" + config.propertyId + "_color";

   // Find the last character that is not a space or curly brace
   int lastIndex = config.valueTemplate.length() - 1;
   while (lastIndex >= 0 && (config.valueTemplate[lastIndex] == ' ' || config.valueTemplate[lastIndex] == '}')) {
      lastIndex--;
   }

   // Value templates to extract values from the same topic
   json["state_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".state" + config.valueTemplate.substring(lastIndex + 1);
   json["brightness_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".brightness" + config.valueTemplate.substring(lastIndex + 1);
   json["effect_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".effect" + config.valueTemplate.substring(lastIndex + 1);
   json["hs_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".color.h" + config.valueTemplate.substring(lastIndex + 1) + ","
                              + config.valueTemplate.substring(0, lastIndex + 1) + ".color.s" + config.valueTemplate.substring(lastIndex + 1);
   json["color_mode_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".color_mode" + config.valueTemplate.substring(lastIndex + 1);

   json["brightness"] = true;
   json["brightness_scale"]=5;
   json["effect"] = true;
   JsonArray effect_list = json["effect_list"].to<JsonArray>();
   for (const auto& effect: colorModes) effect_list.add(effect);
   JsonArray color_modes = json["supported_color_modes"].to<JsonArray>();
   color_modes.add("hs");

   serializeJson(json, output);
}

void generateClimateAdJSON(String& output, const AutoDiscoveryInformationTemplate& config, const SpaADInformationTemplate& spa, String &discoveryTopic);

/*
struct SensorAdConfig {
    String stateClass;          // https://developers.home-assistant.io/docs/core/entity/sensor/#long-term-statistics (empty string accepted)
    String unitOfMeasure;       // V, W, A, mV, etc (empty string accepted)
};*/

#endif
