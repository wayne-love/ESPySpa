#ifndef CONFIG_H
#define CONFIG_H

#include <Preferences.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <RemoteDebug.h>

extern RemoteDebug Debug;

template <typename T>
class Setting {
private:
    T _value;
    const char* _name;
    static void (*_settingCallback)(const char*, T);

public:
    Setting(const char* name, T initialValue = T()) : _name(name), _value(initialValue) {}

    T getValue() { return _value; }
    void setValue(T newval) {
        T oldvalue = _value;
        _value = newval;
        if ((_settingCallback) && (oldvalue != newval)) {
                _settingCallback(_name, _value);
        }
    }

    static void setCallback(void (*callback)(const char*, T)) {
        _settingCallback = callback;
    }
};

template<>
class Setting<int> {
private:
    int _value;
    const char* _name;
    static void (*_settingCallback)(const char*, int);

    int _minValue;
    int _maxValue;

public:
    Setting(const char* name, int initialValue, int minValue = 10, int maxValue = 3600)
        : _name(name), _value(initialValue), _minValue(minValue), _maxValue(maxValue) {}

    int getValue() { return _value; }

    void setValue(int newval) {
        int oldvalue = _value;
        _value = (newval < _minValue) ? _minValue : (newval > _maxValue) ? _maxValue : newval;
        if ((_settingCallback) && (oldvalue != _value)) {
            _settingCallback(_name, _value);
        }
    }

    static void setCallback(void (*callback)(const char*, int)) {
        _settingCallback = callback;
    }
};

/// @brief represents the controller configuration options to be stored.
class ControllerConfig {
public:
    Setting<String> MqttServer = Setting<String>("MqttServer", "mqtt");
    Setting<int> MqttPort = Setting<int>("MqttPort", 1883, 1, 65535);
    Setting<String> MqttUsername = Setting<String>("MqttUsername");
    Setting<String> MqttPassword = Setting<String>("MqttPassword");
    Setting<String> SpaName = Setting<String>("SpaName", "eSpa");
    Setting<int> UpdateFrequency = Setting<int>("UpdateFrequency", 60, 10, 300);
};

class Config : public ControllerConfig {
public:
    // Constructor
    Config();

    bool readConfig();              // Read configuration from Preferences or file
    void writeConfig();             // Write configuration to Preferences

    // Set callback for all Setting instances
    template <typename T>
    void setCallback(void (*callback)(const char*, T)) {
        Setting<T>::setCallback(callback);
    }

};

#endif // CONFIG_H
