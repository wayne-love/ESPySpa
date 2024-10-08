#ifndef HAAUTODISCOVERY_H
#define HAAUTODISCOVERY_H

#include <Arduino.h>
#include <ArduinoJson.h>


/// @brief Configuration structure for the data elements for the Spa.
struct SpaAdConfig {
    String spaName;             // Spa name (eg MySpa)
    String spaSerialNumber;     // Spa serial number 
    String stateTopic;          // MQTT topic where staus informaion is published.
    String availabilityTopic;   // MQTT topic for availability of the Spa (not sensor)
};

/// @brief Configuration structure for sensor autodiscovery - https://www.home-assistant.io/integrations/sensor.mqtt/
struct SensorAdConfig {
    String displayName;         // Display name for the sensor
    String valueTemplate;       // Value template to extract the status information
    String propertyId;          // Unique ID of the sensor, will be concated with deviceName to give a globally unique ID
    String deviceClass;         // https://www.home-assistant.io/integrations/sensor#device-class (empty string accepted)
    String entityCategory;      // https://developers.home-assistant.io/blog/2021/10/26/config-entity?_highlight=diagnostic#entity-categories (empty string accepted)
    String stateClass;          // https://developers.home-assistant.io/docs/core/entity/sensor/#long-term-statistics (empty string accepted)
    String unitOfMeasure;       // V, W, A, mV, etc (empty string accepted)
};

/// @brief Configureation struction for binary sensor autodiscovery - https://www.home-assistant.io/integrations/binary_sensor.mqtt/
struct BinarySensorAdConfig {
    String displayName;         // Display name for the sensor
    String valueTemplate;       // Value template to extract the status information
    String propertyId;          // Unique ID of the sensor, will be concated with deviceName to give a globally unique ID
    String deviceClass;         // https://www.home-assistant.io/integrations/binary_sensor/#device-class (empty string accepted)
};



/// @brief Generate JSON string to publish for Sensor auto discovery
/// @param output String to revceive JSON output
/// @param config Structure to define JSON output
void sensorAdJSON(String& output, const SensorAdConfig& config, const SpaAdConfig& spa, String &discoveryTopic);

/// @brief Generate JSON string to publish for Sensor auto discovery
/// @param output String to revceive JSON output
/// @param config Structure to define JSON output
void binarySensorAdJSON(String& output, const BinarySensorAdConfig& config, const SpaAdConfig& spa, String &discoveryTopic);


#endif  