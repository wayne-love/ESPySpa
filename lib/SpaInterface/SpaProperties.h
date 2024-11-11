#ifndef SPAPROPERTIES_H
#define SPAPROPERTIES_H

#include <Arduino.h>
#include <RemoteDebug.h>
#include <time.h>
#include <TimeLib.h>
#include <array>

extern RemoteDebug Debug;

template<typename T>
void debugVT(const char* msg, const char* name, T oldValue, T newValue) {
    if (std::is_same<T, int>::value) {
        debugV("%s int, name: %s, old value: %i, new value: %i", msg, name, oldValue, newValue);
    }
    else if (std::is_same<T, float>::value) {
        debugV("%s float, name: %s, old value: %.2f, new value: %.2f", msg, name, oldValue, newValue);
    }
    else if (std::is_same<T, String>::value) {
        debugV("%s String, name: %s, old value: %s, new value: %s", msg, name, String(oldValue).c_str(), String(newValue).c_str());
    } else {
        debugV("%s unknown type, name: %s, old value: ?, new value: ?", msg, name);
    }
};

template <typename T>
class Property {
private:
    T _value;
    const char* _name;
    void (*_updateCallback)(T) = nullptr;
    static std::function<bool(const char*, T)> _sendCallback;

    friend class SpaInterface;
    friend class SpaProperties;

public:
    Property(const char* name, T initialValue = T()) : _name(name) {}

    T getValue() { return _value; }
    void sendValue(T newValue)
    {
        T oldValue = _value;
        debugVT("sendValue T", _name, oldValue, newValue);
        if ((_sendCallback) && (oldValue != newValue)) {
            _sendCallback(_name, newValue);
        }
    };
    static void setSendCallback(std::function<bool(const char*, T)> c) { _sendCallback = c; };
    void setUpdateCallback(void (*c)(T)) { _updateCallback = c; };

protected:
    //TODO do we need updateValue to return true on success??
    void updateValue(T newValue)
    {
        T oldValue = _value;
        _value = newValue;
        if ((_updateCallback) && (oldValue != newValue)) {
            _updateCallback(_value);
        }
    };
};

template<>
class Property<int> {
private:
    int _value;
    const char* _name;
    void (*_updateCallback)(int) = nullptr;
    static std::function<bool(const char*, int)> _sendCallback;
    static std::function<bool(const char*, String)> _sendStringCallback;

    int _minValue;
    int _maxValue;

    friend class SpaInterface;
    friend class SpaProperties;

public:
    Property(const char* name, int minValue = INT_MIN, int maxValue = INT_MAX)
        : _name(name), _minValue(minValue), _maxValue(maxValue) {}

    int getValue() { return _value; }

    void sendValue(int newValue) {
        int oldValue = _value;
        debugV("sendValue int, name: %s, old value: %i, new value: %i", _name, oldValue, newValue);
        if (newValue > _maxValue || newValue < _minValue) {
            debugW("Value for %s out of range: %i", _name, newValue);
        }
        else if ((_sendCallback) && (oldValue != newValue)) {
            _sendCallback(_name, newValue);
        }
    }

    // Overload setValue to accept String input and convert it to int
    void sendValue(const String& s) {
        debugV("sendValue int String, name: %s, old value: %s, new value: %s", _name, String(_value).c_str(), s.c_str());
        if ((_sendStringCallback)) {
            _sendStringCallback(_name, s);
        }
    }

    static void setSendCallback(std::function<bool(const char*, int)> c) { _sendCallback = c; };
    static void setSendCallback(std::function<bool(const char*, String)> c) { _sendStringCallback = c; };
    void setUpdateCallback(void (*c)(int)) { _updateCallback = c; };

protected:
    //TODO do we need updateValue to return true on success??
    void updateValue(int newValue) {
        int oldValue = _value;
        _value = newValue;
        if ((_updateCallback) && (oldValue != newValue)) {
            _updateCallback(_value);
        }
    };

    //TODO do we need updateValue to return true on success??
    // Overload updateValue to accept String input and convert it to int
    void updateValue(const String& s) {
        if (s.isEmpty()) return;
        for (u_int i = 0; i < s.length(); i++) {
            if ((!isDigit(s[i])) && !(s[i] == '-') && !(s[i]=='.')) {
                return;
            }
        }

        int newValue = s.toInt();
        updateValue(newValue);

    }
};

template<>
class Property<bool> {
private:
    bool _value;
    const char* _name;
    void (*_updateCallback)(bool) = nullptr;
    static std::function<bool(const char*, bool)> _sendCallback;

    friend class SpaInterface;
    friend class SpaProperties;

public:
    Property(const char* name)
        : _name(name) {}

    bool getValue() { return _value; }

    void sendValue(bool newValue) {
        bool oldValue = _value;
        debugV("sendValue bool String, name: %s, old value: %s, new value: %s", _name, String(oldValue?"true":"false"), String(newValue?"true":"false"));

        if ((_sendCallback) && (oldValue != newValue)) {
            _sendCallback(_name, newValue);
        }
    }

    // Overload setValue to accept String input and convert it to int
    void sendValue(const String& s) {
        bool newValue = s == "1";
        debugV("sendValue bool String, name: %s, old value: %s, new value: %s", _name, String(_value?"1":"0"), s.c_str());
        sendValue(newValue);
    }

    static void setSendCallback(std::function<bool(const char*, bool)> c) { _sendCallback = c; };
    void setUpdateCallback(void (*c)(bool)) { _updateCallback = c; };

protected:
    //TODO do we need updateValue to return true on success??
    void updateValue(bool newValue) {
        int oldValue = _value;
        _value = newValue;
        if ((_updateCallback) && (oldValue != newValue)) {
            _updateCallback(_value);
        }
    };

    //TODO do we need updateValue to return true on success??
    // Overload updateValue to accept String input and convert it to int
    void updateValue(const String& s) {
        if (s !="0" && s !="1") return;

        int newValue = s == "1";
        updateValue(newValue);

    }
};

/// @brief represents the properties of the spa.
class SpaProperties {
    protected:
        void updateMeasures(String statusResponseRaw[], int R2, int R3, int R4, int R5, int R6, int R7, int R9, int RA, int RB, int RC, int RE, int RG);

    public:

    #pragma region R2
        /// @brief Mains current draw (A)
        Property<int> MainsCurrent = Property<int>("MainsCurrent");
        /// @brief Mains voltage (V)
        Property<int> MainsVoltage = Property<int>("MainsVoltage");
        /// @brief Internal case temperature ('C)
        Property<int> CaseTemperature = Property<int>("CaseTemperature");
        /// @brief 12v port current (mA)
        Property<int> PortCurrent = Property<int>("PortCurrent");
        /// @brief Current time on Spa RTC
        // TODO #1 this should be settable.
        Property<time_t> SpaTime = Property<time_t>("SpaTime");
        /// @brief Heater temperature ('C)
        Property<int> HeaterTemperature = Property<int>("HeaterTemperature");
        /// @brief Pool temperature ('C). Note this seems to return rubbish most of the time.
        Property<int> PoolTemperature = Property<int>("PoolTemperature");
        /// @brief Water present
        Property<bool> WaterPresent = Property<bool>("WaterPresent");
        /// @brief AwakeMinutesRemaining (min)
        Property<int> AwakeMinutesRemaining = Property<int>("AwakeMinutesRemaining");
        /// @brief FiltPumpRunTimeTotal (min)
        Property<int> FiltPumpRunTimeTotal = Property<int>("FiltPumpRunTimeTotal");
        /// @brief FiltPumpReqMins
        // TODO - the value here does not match the value in the snampshot data (1442 != 2:00)
        Property<int> FiltPumpReqMins = Property<int>("FiltPumpReqMins");
        /// @brief LoadTimeOut (sec)
        Property<int> LoadTimeOut = Property<int>("LoadTimeOut");
        /// @brief HourMeter (hours)
        Property<int> HourMeter = Property<int>("HourMeter");
        /// @brief Relay1 (?)
        Property<int> Relay1 = Property<int>("Relay1");
        /// @brief Relay2 (?)
        Property<int> Relay2 = Property<int>("Relay2");
        /// @brief Relay3 (?)
        Property<int> Relay3 = Property<int>("Relay3");
        /// @brief Relay4 (?)
        Property<int> Relay4 = Property<int>("Relay4");
        /// @brief Relay5 (?)
        Property<int> Relay5 = Property<int>("Relay5");
        /// @brief Relay6 (?)
        Property<int> Relay6 = Property<int>("Relay6");
        /// @brief Relay7 (?)
        Property<int> Relay7 = Property<int>("Relay7");
        /// @brief Relay8 (?)
        Property<int> Relay8 = Property<int>("Relay8");
        /// @brief Relay9 (?)
        Property<int> Relay9 = Property<int>("Relay9");
    #pragma endregion
    #pragma region R3
        // R3
        /// @brief Current limit (A)
        Property<int> CLMT = Property<int>("CLMT");
        /// @brief Power phases in use
        Property<int> PHSE = Property<int>("PHSE");
        /// @brief Load limit - Phase 1
        ///
        /// Number of services that can be active on this phase before the keypad stops new services from starting.
        /// See SV-Series-OEM-Install-Manual.pdf page 20.
        Property<int> LLM1 = Property<int>("LLM1");
        /// @brief Load limit - Phase 2
        ///
        /// Number of services that can be active on this phase before the keypad stops new services from starting.
        /// See SV-Series-OEM-Install-Manual.pdf page 20.
        Property<int> LLM2 = Property<int>("LLM2");
        /// @brief Load limit - Phase 3
        ///
        /// Number of services that can be active on this phase before the keypad stops new services from starting.
        /// See SV-Series-OEM-Install-Manual.pdf page 20.
        Property<int> LLM3 = Property<int>("LLM3");
        /// @brief Software version
        Property<String> SVER = Property<String>("SVER");
        /// @brief Model
        Property<String> Model = Property<String>("Model");
        /// @brief SerialNo1
        Property<String> SerialNo1 = Property<String>("SerialNo1");
        /// @brief SerialNo2
        Property<String> SerialNo2 = Property<String>("SerialNo2");
        /// @brief Dipswitch 1
        Property<bool> D1 = Property<bool>("D1");
        /// @brief Dipswitch 2
        Property<bool> D2 = Property<bool>("D2");
        /// @brief Dipswitch 3
        Property<bool> D3 = Property<bool>("D3");
        /// @brief Dipswitch 4
        Property<bool> D4 = Property<bool>("D4");
        /// @brief Dipswitch 5
        Property<bool> D5 = Property<bool>("D5");
        /// @brief Dipswitch 6
        Property<bool> D6 = Property<bool>("D6");
        /// @brief Pump
        Property<String> Pump = Property<String>("Pump");
        /// @brief Load shed count
        ///
        /// Number of services active at which the heater will turn itself off.
        Property<int> LS = Property<int>("LS");
        /// @brief HV
        Property<bool> HV = Property<bool>("HV");
        /// @brief MR / name clash with MR constant from specreg.h
        Property<int> SnpMR = Property<int>("SnpMR");
        /// @brief Status (Filtering, etc)
        Property<String> Status = Property<String>("Status");
        /// @brief PrimeCount
        Property<int> PrimeCount = Property<int>("PrimeCount");
        /// @brief Heat element current draw (A)
        Property<int> EC = Property<int>("EC");
        /// @brief HAMB
        Property<int> HAMB = Property<int>("HAMB");
        /// @brief HCON
        Property<int> HCON = Property<int>("HCON");
    // Unclear encoding of HV_2
    /// @brief HV_2
    Property<bool> HV_2 = Property<bool>("HV_2");
    //
    #pragma endregion
    #pragma region R4
        // R4
        /// @brief Operation mode
        ///
        /// One of NORM, ECON, AWAY, WEEK
        Property<String> Mode = Property<String>("Mode");
        /// @brief Service Timer 1 (wks) 0 = off
        Property<int> Ser1_Timer = Property<int>("Ser1_Timer");
        /// @brief Service Timer 2 (wks) 0 = off
        Property<int> Ser2_Timer = Property<int>("Ser2_Timer");
        /// @brief Service Timer 3 (wks) 0 = off
        Property<int> Ser3_Timer = Property<int>("Ser3_Timer");
        /// @brief Heat mode
        ///
        /// 1 = Standby
        /// 2 = HeatMix
        Property<int> HeatMode = Property<int>("HeatMode");
        /// @brief Pump idle time (sec)
        Property<int> PumpIdleTimer = Property<int>("PumpIdleTimer");
        /// @brief Pump run time (sec)
        Property<int> PumpRunTimer = Property<int>("PumpRunTimer");
        /// @brief Pool temperature adaptive hysteresis
        Property<int> AdtPoolHys = Property<int>("AdtPoolHys");
        /// @brief  Heater temperature adaptive hysteresis
        Property<int> AdtHeaterHys = Property<int>("AdtHeaterHys");
        /// @brief Power consumtion * 10
        Property<int> Power = Property<int>("Power");
        Property<int> Power_kWh = Property<int>("Power_kWh");
        // (kWh)
        Property<int> Power_Today = Property<int>("Power_Today");
        // (kWh)
        Property<int> Power_Yesterday = Property<int>("Power_Yesterday");
        // 0 = ok
        Property<int> ThermalCutOut = Property<int>("ThermalCutOut");
        Property<int> Test_D1 = Property<int>("Test_D1");
        Property<int> Test_D2 = Property<int>("Test_D2");
        Property<int> Test_D3 = Property<int>("Test_D3");
        Property<int> ElementHeatSourceOffset = Property<int>("ElementHeatSourceOffset");
        Property<int> Frequency = Property<int>("Frequency");
        Property<int> HPHeatSourceOffset_Heat = Property<int>("HPHeatSourceOffset_Heat");
        // 100 = 0!?
        Property<int> HPHeatSourceOffset_Cool = Property<int>("HPHeatSourceOffset_Cool");
        Property<int> HeatSourceOffTime = Property<int>("HeatSourceOffTime");
        Property<int> Vari_Speed = Property<int>("Vari_Speed");
        Property<int> Vari_Percent = Property<int>("Vari_Percent");
        // 5 = Filt
        // 4 = Off
        /// @brief Varible speed mode
        ///
        /// 5 = Filtering, 4 = Off
        Property<int> Vari_Mode = Property<int>("Vari_Mode");
    #pragma endregion
    #pragma region R5
        // R5
        //  Unknown encoding - Attribute<int> TouchPad2;
        //  Unknown encoding - Attribute<int> TouchPad1;
        /// @brief Pump 1 state
        ///
        /// 0 = off, 1 = running, 4 = auto
        Property<int> RB_TP_Pump1 = Property<int>("RB_TP_Pump1", 0, 4);
        /// @brief Pump 2 state
        ///
        /// 0 = off, 1 = running, 4 = auto
        Property<int> RB_TP_Pump2 = Property<int>("RB_TP_Pump2", 0, 4);
        /// @brief Pump 3 state
        ///
        /// 0 = off, 1 = running, 4 = auto
        Property<int> RB_TP_Pump3 = Property<int>("RB_TP_Pump3", 0, 4);
        /// @brief Pump 4 state
        ///
        /// 0 = off, 1 = running, 4 = auto
        Property<int> RB_TP_Pump4 = Property<int>("RB_TP_Pump4", 0, 4);
        /// @brief Pump 5 state
        ///
        /// 0 = off, 1 = running, 4 = auto
        Property<int> RB_TP_Pump5 = Property<int>("RB_TP_Pump5", 0, 4);
        Property<int> RB_TP_Blower = Property<int>("RB_TP_Blower");
        Property<int> RB_TP_Light = Property<int>("RB_TP_Light", 0, 1);
        /// @brief Auto enabled
        ///
        /// True when auto enabled
        Property<bool> RB_TP_Auto = Property<bool>("RB_TP_Auto");
        /// @brief Heating running
        ///
        /// True when heating/cooling active
        Property<bool> RB_TP_Heater = Property<bool>("RB_TP_Heater");
        /// @brief Cleaning (UV/Ozone running)
        ///
        /// True when Ozone/UV is cleaning spa.
        Property<bool> RB_TP_Ozone = Property<bool>("RB_TP_Ozone");
        /// @brief Sleeping
        ///
        /// True when spa is sleeping due to sleep timer
        Property<bool> RB_TP_Sleep = Property<bool>("RB_TP_Sleep");
        /// @brief Water temperature ('C)
        Property<int> WTMP = Property<int>("WTMP");
        /// @brief Clean cycle running
        ///
        /// True when a clean cycle is running
        Property<bool> CleanCycle = Property<bool>("CleanCycle");
    #pragma endregion
    #pragma region R6
        // R6
        /// @brief Blower variable speed
        ///
        /// min 1, max 5
        Property<int> VARIValue = Property<int>("VARIValue", 1, 5);
        /// @brief Lights brightness
        ///
        /// min 1, max 5
        Property<int> LBRTValue = Property<int>("LBRTValue", 1, 5);
        /// @brief Light colour
        ///
        /// min 0, max 31
        Property<int> CurrClr = Property<int>("CurrClr", 0, 31);
        /// @brief Lights mode
        ///
        /// 0 = white, 1 = colour, 2 = step, 3 = fade, 4 = party
        Property<int> ColorMode = Property<int>("ColorMode", 0, 4);
        /// @brief Light effect speed
        ///
        /// min 1, max 5
        Property<int> LSPDValue = Property<int>("LSPDValue", 1, 5);
        /// @brief Filter run time (in hours) per block
        Property<int> FiltSetHrs = Property<int>("FiltSetHrs");
        /// @brief Filter block duration (hours)
        Property<int> FiltBlockHrs = Property<int>("FiltBlockHrs");
        /// @brief Water temperature set point ('C)
        Property<int> STMP = Property<int>("STMP", 100, 410);
        // 1 = 12 hrs
        Property<int> L_24HOURS = Property<int>("L_24HOURS", 1, 12);
        /// @brief Power save level
        ///
        /// 0 = off, 1 = low, 2 = high
        Property<int> PSAV_LVL = Property<int>("PSAV_LVL", 0, 2);
        /// @brief Peak power start time
        ///
        /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
        Property<int> PSAV_BGN = Property<int>("PSAV_BGN", 0, 5947);
        /// @brief Peak power end time
        ///
        /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
        Property<int> PSAV_END = Property<int>("PSAV_END", 0, 5947);
        /// @brief Sleep timer 1
        ///
        /// 128 = off, 127 = every day, 96 = weekends, 31 = weekdays
        Property<int> L_1SNZ_DAY = Property<int>("L_1SNZ_DAY", 0, 128);
        /// @brief Sleep timer 2
        ///
        /// 128 = off, 127 = every day, 96 = weekends, 31 = weekdays
        Property<int> L_2SNZ_DAY = Property<int>("L_2SNZ_DAY", 0, 128);
        /// @brief Sleep time 1 start time
        ///
        /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
        Property<int> L_1SNZ_BGN = Property<int>("L_1SNZ_BGN", 0, 5947);
        /// @brief Sleep time 2 start time
        ///
        /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
        Property<int> L_2SNZ_BGN = Property<int>("L_2SNZ_BGN", 0, 5947);
        /// @brief Sleep time 1 end time
        ///
        /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
        Property<int> L_1SNZ_END = Property<int>("L_1SNZ_END", 0, 5947);
        /// @brief Sleep time 1 end time
        ///
        /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
        Property<int> L_2SNZ_END = Property<int>("L_2SNZ_END", 0, 5947);
        /// @brief Default screen for control panels
        ///
        /// 0 = WTPM
        Property<int> DefaultScrn = Property<int>("DefaultScrn");
        /// @brief Time out duration (min)
        ///
        /// Time in min before pump and blower time out (min 10, max 30)
        Property<int> TOUT = Property<int>("TOUT", 10, 30);
        Property<bool> VPMP = Property<bool>("VPMP");
        Property<bool> HIFI = Property<bool>("HIFI");
        /// @brief BRND
        ///
        /// 2 = VORT
        Property<int> BRND = Property<int>("BRND");
        /// @brief PRME
        ///
        /// 0 = 10secF
        Property<int> PRME = Property<int>("PRME");
        Property<int> ELMT = Property<int>("ELMT");
        /// @brief TYPE
        ///
        /// 3 = SV3
        Property<int> TYPE = Property<int>("TYPE");
        Property<int> GAS = Property<int>("GAS");
    #pragma endregion
    #pragma region R7
        // R7
        /// @brief Daily clean cycle start time
        ///
        /// Time with the formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
        Property<int> WCLNTime = Property<int>("WCLNTime");
        /// @brief Use 'F instead of 'C as the temp. UMO
        Property<bool> TemperatureUnits = Property<bool>("TemperatureUnits");
        Property<bool> OzoneOff = Property<bool>("OzoneOff");
        /// @brief Sanitiser 24 hrs
        ///
        /// True if sanitiser (ozone) power outlet on permanently, false if automatically controlled.
        /// See SV-Series-OEM-Install-Manual.pdf page 20.
        Property<bool> Ozone24 = Property<bool>("Ozone24");
        /// @brief Circulation pump 24hrs
        ///
        /// True if circulation pump is always on, false if automatically controlled.
        /// See SV-Series-OEM-Install-Manual.pdf page 20.
        Property<bool> Circ24 = Property<bool>("Circ24");
        Property<bool> CJET = Property<bool>("CJET");
        /// @brief Variable heat element operation
        ///
        /// If true allows variable power to be fed to the heating element.
        /// See SV-Series-OEM-Install-Manual.pdf page 19.
        Property<bool> VELE = Property<bool>("VELE");

        /// TODO #2 - Not Implemented
        /// @brief Date of comissioning
        /// Property<time_t> ComissionDate = Property<time_t>("ComissionDate");

        /// @brief Highest voltage ever recorded (V)
        Property<int> V_Max = Property<int>("V_Max");
        /// @brief Lowest voltage ever recorded (V)
        Property<int> V_Min = Property<int>("V_Min");
        /// @brief Highest voltage in past 24 hrs (V)
        Property<int> V_Max_24 = Property<int>("V_Max_24");
        /// @brief Lowest voltage in past 24 hrs (V)
        Property<int> V_Min_24 = Property<int>("V_Min_24");
        Property<int> CurrentZero = Property<int>("CurrentZero");
        Property<int> CurrentAdjust = Property<int>("CurrentAdjust");
        Property<int> VoltageAdjust = Property<int>("VoltageAdjust");
        Property<int> Ser1 = Property<int>("Ser1");
        Property<int> Ser2 = Property<int>("Ser2");
        Property<int> Ser3 = Property<int>("Ser3");
        /// @brief Variable heat element max power (A)
        ///
        /// Maximum current that the heat element is allowed to draw (between 3 and 25A)
        /// See SV-Series-OEM-Install-Manual.pdf page 19.
        Property<int> VMAX = Property<int>("VMAX");
        /// @brief Adaptive Hysteresis
        ///
        /// Maximum adaptive hysteresis value (0=disabled).  See SV-Series-OEM-Install-Manual.pdf page 20.
        Property<int> AHYS = Property<int>("AHYS");
        /// @brief HUSE
        ///
        /// 1 = Off
        Property<bool> HUSE = Property<bool>("HUSE");
        /// @brief Heat pump active whilst spa is in use
        ///
        /// If false then when spa is in use then heat pump will not run to reduce noise levels
        /// See SV-Series-OEM-Install-Manual.pdf page 19.
        Property<bool> HELE = Property<bool>("HELE");
        /// @brief Heatpump mode
        ///
        /// 0 = Auto, 1 = Heat, 2 = Cool, 3 = disabled
        Property<int> HPMP = Property<int>("HPMP", 0, 3);
        /// @brief Varible pump minimum speed setting
        ///
        /// Min 20%, Max 100%
        /// See SV-Series-OEM-Install-Manual.pdf page 21.
        Property<int> PMIN = Property<int>("PMIN");
        /// @brief Variable pump filtration speed setting
        ///
        /// Min 20%, Max 100%
        /// See SV-Series-OEM-Install-Manual.pdf page 21.
        Property<int> PFLT = Property<int>("PFLT");
        /// @brief Varible pump heater speed setting
        ///
        /// Min 20%, Max 100%
        /// See SV-Series-OEM-Install-Manual.pdf page 21.
        Property<int> PHTR = Property<int>("PHTR");
        /// @brief Varible pump maximum speed setting
        ///
        /// Maximum speed the varible pump will run at.
        /// Min 20%, Max 100%
        Property<int> PMAX = Property<int>("PMAX");
    #pragma endregion
    #pragma region R9
        // R9
        /// @brief Fault runtime occurance (hrs)
        Property<int> F1_HR = Property<int>("F1_HR");
        /// @brief Fault time of day occurance
        Property<int> F1_Time = Property<int>("F1_Time");
        /// @brief Fault error codes
        ///
        /// 6 = ER612VOverload - High current detected on 12v line
        Property<int> F1_ER = Property<int>("F1_ER");
        /// @brief Supply current draw at time of error (A)
        Property<int> F1_I = Property<int>("F1_I");
        /// @brief Supply voltage at time of error (V)
        Property<int> F1_V = Property<int>("F1_V");
        /// @brief Pool temperature at time of error ('C)
        Property<int> F1_PT = Property<int>("F1_PT");
        /// @brief Heater temperature at time of error ('C)
        Property<int> F1_HT = Property<int>("F1_HT");
        Property<int> F1_CT = Property<int>("F1_CT");
        Property<int> F1_PU = Property<int>("F1_PU");
        Property<bool> F1_VE = Property<bool>("F1_VE");
        /// @brief Heater setpoint at time of error ('C)
        Property<int> F1_ST = Property<int>("F1_ST");
    #pragma endregion
    #pragma region RA
        // RA
        /// @brief Fault runtime occurance (hrs)
        Property<int> F2_HR = Property<int>("F2_HR");
        /// @brief Fault time of day occurance
        Property<int> F2_Time = Property<int>("F2_Time");
        /// @brief Fault error codes
        ///
        /// 6 = ER612VOverload - High current detected on 12v line
        Property<int> F2_ER = Property<int>("F2_ER");
        /// @brief Supply current draw at time of error (A)
        Property<int> F2_I = Property<int>("F2_I");
        /// @brief Supply voltage at time of error (V)
        Property<int> F2_V = Property<int>("F2_V");
        /// @brief Pool temperature at time of error ('C)
        Property<int> F2_PT = Property<int>("F2_PT");
        /// @brief Heater temperature at time of error ('C)
        Property<int> F2_HT = Property<int>("F2_HT");
        Property<int> F2_CT = Property<int>("F2_CT");
        Property<int> F2_PU = Property<int>("F2_PU");
        Property<bool> F2_VE = Property<bool>("F2_VE");
        /// @brief Heater setpoint at time of error ('C)
        Property<int> F2_ST = Property<int>("F2_ST");
    #pragma endregion
    #pragma region RB
        // RB
        /// @brief Fault runtime occurance (hrs)
        Property<int> F3_HR = Property<int>("F3_HR");
        /// @brief Fault time of day occurance
        Property<int> F3_Time = Property<int>("F3_Time");
        /// @brief Fault error codes
        ///
        /// 6 = ER612VOverload - High current detected on 12v line
        Property<int> F3_ER = Property<int>("F3_ER");
        /// @brief Supply current draw at time of error (A)
        Property<int> F3_I = Property<int>("F3_I");
        /// @brief Supply voltage at time of error (V)
        Property<int> F3_V = Property<int>("F3_V");
        /// @brief Pool temperature at time of error ('C)
        Property<int> F3_PT = Property<int>("F3_PT");
        /// @brief Heater temperature at time of error ('C)
        Property<int> F3_HT = Property<int>("F3_HT");
        Property<int> F3_CT = Property<int>("F3_CT");
        Property<int> F3_PU = Property<int>("F3_PU");
        Property<bool> F3_VE = Property<bool>("F3_VE");
        /// @brief Heater setpoint at time of error ('C)
        Property<int> F3_ST = Property<int>("F3_ST");
    #pragma endregion
    #pragma region RC
        // RC
        //  Encoding of the RC registers is not obvious
        // Attribute<bool> Outlet_Heater;
        // Attribute<bool> Outlet_Circ;
        // Attribute<bool> Outlet_Sanitise;
        // Attribute<bool> Outlet_Pump1;
        // Attribute<bool> Outlet_Pump2;
        // Attribute<bool> Outlet_Pump4;
        // Attribute<bool> Outlet_Pump5;
        /// @brief Blower status
        ///
        /// 0 = variable mode, 1 = ramp mode, 2 = off
        Property<int> Outlet_Blower = Property<int>("Outlet_Blower", 0, 2);
    #pragma endregion
    #pragma region RE
        // RE
        /// @brief Heatpump installed / interface version
        Property<int> HP_Present = Property<int>("HP_Present");
        // Encoding of these registers is not clear
        // Attribute<bool> HP_FlowSwitch;
        // Attribute<bool> HP_HighSwitch;
        // Attribute<bool> HP_LowSwitch;
        // Attribute<bool> HP_CompCutOut;
        // Attribute<bool> HP_ExCutOut;
        // Attribute<bool> HP_D1;
        // Attribute<bool> HP_D2;
        // Attribute<bool> HP_D3;
        /// @brief Ambient air temperature ('C)
        Property<int> HP_Ambient = Property<int>("HP_Ambient");
        /// @brief Compressor temperature ('C)
        Property<int> HP_Condensor = Property<int>("HP_Condensor");
        /// @brief Compressor running
        Property<bool> HP_Compressor_State = Property<bool>("HP_Compressor_State");
        /// @brief Fan running
        Property<bool> HP_Fan_State = Property<bool>("HP_Fan_State");
        Property<bool> HP_4W_Valve = Property<bool>("HP_4W_Valve");
        Property<bool> HP_Heater_State = Property<bool>("HP_Heater_State");
        /// @brief Heatpump state
        ///
        /// 0 = Standby
        Property<int> HP_State = Property<int>("HP_State");
        /// @brief Heatpump mode
        ///
        /// 1 = Heat
        Property<int> HP_Mode = Property<int>("HP_Mode");
        Property<int> HP_Defrost_Timer = Property<int>("HP_Defrost_Timer");
        Property<int> HP_Comp_Run_Timer = Property<int>("HP_Comp_Run_Timer");
        Property<int> HP_Low_Temp_Timer = Property<int>("HP_Low_Temp_Timer");
        Property<int> HP_Heat_Accum_Timer = Property<int>("HP_Heat_Accum_Timer");
        Property<int> HP_Sequence_Timer = Property<int>("HP_Sequence_Timer");
        Property<int> HP_Warning = Property<int>("HP_Warning");
        Property<int> FrezTmr = Property<int>("FrezTmr");
        Property<int> DBGN = Property<int>("DBGN");
        Property<int> DEND = Property<int>("DEND");
        Property<int> DCMP = Property<int>("DCMP");
        Property<int> DMAX = Property<int>("DMAX");
        Property<int> DELE = Property<int>("DELE");
        Property<int> DPMP = Property<int>("DPMP");
    // Attribute<int> CMAX;
    // Attribute<int> HP_Compressor;
    // Attribute<int> HP_Pump_State;
    // Attribute<int> HP_Status;
    #pragma endregion
    #pragma region RG
        /// @brief Pump 1 install state
        ///
        /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
        /// means it is), the second part (1- above) indicates it's speed type. The third
        /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
        Property<String> Pump1InstallState = Property<String>("Pump1InstallState");
        /// @brief Pump 2 install state
        ///
        /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
        /// means it is), the second part (1- above) indicates it's speed type. The third
        /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
        Property<String> Pump2InstallState = Property<String>("Pump2InstallState");
        /// @brief Pump 3 install state
        ///
        /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
        /// means it is), the second part (1- above) indicates it's speed type. The third
        /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
        Property<String> Pump3InstallState = Property<String>("Pump3InstallState");
        /// @brief Pump 4 install state
        ///
        /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
        /// means it is), the second part (1- above) indicates it's speed type. The third
        /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
        Property<String> Pump4InstallState = Property<String>("Pump4InstallState");
        /// @brief Pump 5 install state
        ///
        /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
        /// means it is), the second part (1- above) indicates it's speed type. The third
        /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
        Property<String> Pump5InstallState = Property<String>("Pump5InstallState");
        /// @brief Pump 1 is in safe state to start
        Property<bool> Pump1OkToRun = Property<bool>("Pump1OkToRun");
        /// @brief Pump 2 is in safe state to start
        Property<bool> Pump2OkToRun = Property<bool>("Pump2OkToRun");
        /// @brief Pump 3 is in safe state to start
        Property<bool> Pump3OkToRun = Property<bool>("Pump3OkToRun");
        /// @brief Pump 4 is in safe state to start
        Property<bool> Pump4OkToRun = Property<bool>("Pump4OkToRun");
        /// @brief Pump 5 is in safe state to start
        Property<bool> Pump5OkToRun = Property<bool>("Pump5OkToRun");
        /// @brief Lock mode
        ///
        /// 0 = keypad unlocked, 1 = partial lock, 2 = full lock
        Property<int> LockMode = Property<int>("LockMode");

    #pragma endregion

        const std::array <String, 4> spaModeStrings = {"NORM","ECON", "AWAY","WEEK"};
        const std::array <int, 25> colorMap = {0, 4, 4, 19, 13, 25, 25, 16, 10, 7, 2, 8, 5, 3, 6, 6, 21, 21, 21, 18, 18, 9, 9, 1, 1};
        const std::array <String, 5> colorModeStrings = {"White","Color","Fade","Step","Party"};
        const std::array <String, 5> lightSpeedMap = {"1","2","3","4","5"};
        const std::array <String, 11> sleepSelection = {"Off", "Everyday", "Weekends", "Weekdays", "Monday", "Tuesday", "Wednesday", "Thuesday", "Friday", "Saturday", "Sunday"};
        const std::array <byte, 11> sleepBitmap = {128, 127, 96, 31, 16, 8, 4, 2, 1, 64, 32};
        const std::array <String, 4> HPMPStrings = {"Auto","Heat","Cool","Off"};

        Property<String>* pumpStates[5] = {
            &Pump1InstallState,
            &Pump2InstallState,
            &Pump3InstallState,
            &Pump4InstallState,
            &Pump5InstallState
        };

};

#endif  //SPAPROPERTIES_H
