#ifndef SPAPROPERTIES_H
#define SPAPROPERTIES_H

#include <Arduino.h>
#include <RemoteDebug.h>
#include <time.h>
#include <TimeLib.h>
#include <array>


template <typename T>
class Property {
private:
    friend class SpaProperties;
    friend class SpaInterface;

    T _value{};
    void (*_callback)(T) = nullptr;
    std::function<T(const String&)> _converter = nullptr;

    // Assignment operator for direct value
    template<typename U = T, typename = typename std::enable_if<!std::is_same<U, String>::value>::type>
    Property& operator=(const T& newval) noexcept {
        update_Value(newval);
        return *this;
    }

    // Assignment operator for string value
    Property& operator=(const String& strVal) noexcept {
        update_Value(strVal);
        return *this;
    }

    // Update with direct value
    template<typename U>
    void update_Value(U&& newval) noexcept {
        T oldvalue = _value;
        _value = std::forward<U>(newval);
        if (_callback && oldvalue != _value) {
            _callback(_value);
        }
    }

    // Update with string value using converter
    bool update_Value(const String& strVal) noexcept {
        if (!_converter) {
            return false;
        }
        try {
            T newval = _converter(strVal);
            update_Value(std::move(newval));
            return true;
        }
        catch (...) {
            return false;
        }
    }

public:
    // Default constructor
    Property() = default;
    
    // Constructor with converter
    explicit Property(std::function<T(const String&)> converter) 
        : _converter(std::move(converter)) {}
    
    // Implicit conversion operator to get value
    operator T() const noexcept {
        return _value;
    }

    // Get value with const correctness (kept for backward compatibility)
    [[nodiscard]] const T& getValue() const noexcept { 
        return _value; 
    }

    // Callback management
    void setCallback(void (*c)(T)) noexcept { 
        _callback = c; 
    }

    void clearCallback() noexcept { 
        _callback = nullptr; 
    }

    // Converter management
    void setConverter(std::function<T(const String&)> converter) noexcept {
        _converter = std::move(converter);
    }
};

// Common converter functions
namespace PropertyConverters {
    inline int toInt(const String& s) { 
        return s.toInt(); 
    }
    
    inline bool toBool(const String& s) { 
        return s == "1" || s.equalsIgnoreCase("true"); 
    }
    
    inline float toFloat(const String& s) { 
        return s.toFloat(); 
    }
    
    inline String toString(const String& s) { 
        return s; 
    }
    
    // Fixed-point conversion (for temperatures etc)
    inline int toFixed(const String& s, int multiplier = 10) {
        return static_cast<int>(s.toFloat() * multiplier);
    }
}

struct PumpState {
    /// @brief install state of the pump
    /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
    /// means it is), the second part (1- above) indicates it's speed type. The third
    /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
    Property<String> installState{PropertyConverters::toString};  // Format: "1-1-014"
    Property<bool> okToRun{PropertyConverters::toBool};          // Safe to start
    Property<int> currentState{PropertyConverters::toInt};       // 0=off, 1=running, 4=auto
    
    // Helper methods to parse install state
    [[nodiscard]] bool isInstalled() const { 
        return installState.getValue().startsWith("1-"); 
    }
    
    [[nodiscard]] bool isVariableSpeed() const { 
        return isInstalled() && installState.getValue().substring(2,3) == "1"; 
    }
};

/// @brief represents the properties of the spa.
class SpaProperties
{
private:
    static constexpr size_t NUM_PUMPS = 5;
    std::array<PumpState, NUM_PUMPS> pumps;
    
public:

#pragma region R2
    /// @brief Mains current draw (A)
    Property<int> MainsCurrent{PropertyConverters::toInt};
    /// @brief Mains voltage (V)
    Property<int> MainsVoltage{PropertyConverters::toInt};
    /// @brief Internal case temperature ('C)
    Property<int> CaseTemperature{PropertyConverters::toInt};
    /// @brief 12v port current (mA)
    Property<int> PortCurrent{PropertyConverters::toInt};
    /// @brief Current time on Spa RTC
    // TODO #1 this should be settable.
    Property<time_t> SpaTime;
    /// @brief Heater temperature ('C)
    Property<int> HeaterTemperature{PropertyConverters::toInt};
    /// @brief Pool temperature ('C). Note this seems to return rubbish most of the time.
    Property<int> PoolTemperature{PropertyConverters::toInt};
    /// @brief Water present
    Property<bool> WaterPresent{PropertyConverters::toBool};
    /// @brief AwakeMinutesRemaining (min)
    Property<int> AwakeMinutesRemaining{PropertyConverters::toInt};
    /// @brief FiltPumpRunTimeTotal (min)
    Property<int> FiltPumpRunTimeTotal{PropertyConverters::toInt};
    /// @brief FiltPumpReqMins
    // TODO - the value here does not match the value in the snampshot data (1442 != 2:00)
    Property<int> FiltPumpReqMins{PropertyConverters::toInt};
    /// @brief LoadTimeOut (sec)
    Property<int> LoadTimeOut{PropertyConverters::toInt};
    /// @brief HourMeter (hours)
    Property<int> HourMeter{PropertyConverters::toInt};
    /// @brief Relay1 (?)
    Property<int> Relay1{PropertyConverters::toInt};
    /// @brief Relay2 (?)
    Property<int> Relay2{PropertyConverters::toInt};
    /// @brief Relay3 (?)
    Property<int> Relay3{PropertyConverters::toInt};
    /// @brief Relay4 (?)
    Property<int> Relay4{PropertyConverters::toInt};
    /// @brief Relay5 (?)
    Property<int> Relay5{PropertyConverters::toInt};
    /// @brief Relay6 (?)
    Property<int> Relay6{PropertyConverters::toInt};
    /// @brief Relay7 (?)
    Property<int> Relay7{PropertyConverters::toInt};
    /// @brief Relay8 (?)
    Property<int> Relay8{PropertyConverters::toInt};
    /// @brief Relay9 (?)
    Property<int> Relay9{PropertyConverters::toInt};
#pragma endregion
#pragma region R3
    // R3
    /// @brief Current limit (A)
    Property<int> CLMT{PropertyConverters::toInt};
    /// @brief Power phases in use
    Property<int> PHSE{PropertyConverters::toInt};
    /// @brief Load limit - Phase 1
    ///
    /// Number of services that can be active on this phase before the keypad stops new services from starting.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<int> LLM1{PropertyConverters::toInt};
    /// @brief Load limit - Phase 2
    ///
    /// Number of services that can be active on this phase before the keypad stops new services from starting.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<int> LLM2{PropertyConverters::toInt};
    /// @brief Load limit - Phase 3
    ///
    /// Number of services that can be active on this phase before the keypad stops new services from starting.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<int> LLM3{PropertyConverters::toInt};
    /// @brief Software version
    Property<String> SVER{PropertyConverters::toString};
    /// @brief Model
    Property<String> Model{PropertyConverters::toString}; 
    /// @brief SerialNo1
    Property<String> SerialNo1{PropertyConverters::toString};
    /// @brief SerialNo2
    Property<String> SerialNo2{PropertyConverters::toString};
    /// @brief Dipswitch 1
    Property<bool> D1{PropertyConverters::toBool};
    /// @brief Dipswitch 2
    Property<bool> D2{PropertyConverters::toBool};
    /// @brief Dipswitch 3
    Property<bool> D3{PropertyConverters::toBool};
    /// @brief Dipswitch 4
    Property<bool> D4{PropertyConverters::toBool};
    /// @brief Dipswitch 5
    Property<bool> D5{PropertyConverters::toBool};
    /// @brief Dipswitch 6
    Property<bool> D6{PropertyConverters::toBool};
    /// @brief Pump
    Property<String> Pump{PropertyConverters::toString};
    /// @brief Load shed count
    ///
    /// Number of services active at which the heater will turn itself off.
    Property<int> LS{PropertyConverters::toInt};
    /// @brief HV
    Property<bool> HV{PropertyConverters::toBool};
    /// @brief MR / name clash with MR constant from specreg.h
    Property<int> SnpMR{PropertyConverters::toInt};
    /// @brief Status (Filtering, etc)
    Property<String> Status{PropertyConverters::toString};
    /// @brief PrimeCount
    Property<int> PrimeCount{PropertyConverters::toInt};
    /// @brief Heat element current draw (A)
    Property<int> EC{PropertyConverters::toInt};
    /// @brief HAMB
    Property<int> HAMB{PropertyConverters::toInt};
    /// @brief HCON
    Property<int> HCON{PropertyConverters::toInt};
    // Unclear encoding of HV_2
    /// @brief HV_2
    Property<bool> HV_2{PropertyConverters::toBool};

#pragma endregion
#pragma region R4
    // R4
    /// @brief Operation mode
    ///
    /// One of NORM, ECON, AWAY, WEEK
    Property<String> Mode{PropertyConverters::toString};
    /// @brief Service Timer 1 (wks) 0 = off
    Property<int> Ser1_Timer{PropertyConverters::toInt};
    /// @brief Service Timer 2 (wks) 0 = off
    Property<int> Ser2_Timer{PropertyConverters::toInt};
    /// @brief Service Timer 3 (wks) 0 = off
    Property<int> Ser3_Timer{PropertyConverters::toInt};
    /// @brief Heat mode
    ///
    /// 1 = Standby
    /// 2 = HeatMix
    Property<int> HeatMode{PropertyConverters::toInt};
    /// @brief Pump idle time (sec)
    Property<int> PumpIdleTimer{PropertyConverters::toInt};
    /// @brief Pump run time (sec)
    Property<int> PumpRunTimer{PropertyConverters::toInt};
    /// @brief Pool temperature adaptive hysteresis
    Property<int> AdtPoolHys{PropertyConverters::toInt};
    /// @brief  Heater temperature adaptive hysteresis
    Property<int> AdtHeaterHys{PropertyConverters::toInt};
    /// @brief Power consumtion * 10
    Property<int> Power{PropertyConverters::toInt}; 
    Property<int> Power_kWh{PropertyConverters::toInt};
    // (kWh)
    Property<int> Power_Today{PropertyConverters::toInt};
    // (kWh)
    Property<int> Power_Yesterday{PropertyConverters::toInt};
    // 0 = ok
    Property<int> ThermalCutOut{PropertyConverters::toInt};
    Property<int> Test_D1{PropertyConverters::toInt};
    Property<int> Test_D2{PropertyConverters::toInt};
    Property<int> Test_D3{PropertyConverters::toInt};
    Property<int> ElementHeatSourceOffset{PropertyConverters::toInt};
    Property<int> Frequency{PropertyConverters::toInt};
    Property<int> HPHeatSourceOffset_Heat{PropertyConverters::toInt};
    // 100 = 0!?
    Property<int> HPHeatSourceOffset_Cool{PropertyConverters::toInt};
    Property<int> HeatSourceOffTime{PropertyConverters::toInt};
    Property<int> Vari_Speed{PropertyConverters::toInt};
    Property<int> Vari_Percent{PropertyConverters::toInt};
    // 5 = Filt
    // 4 = Off
    /// @brief Varible speed mode
    ///
    /// 5 = Filtering, 4 = Off
    Property<int> Vari_Mode{PropertyConverters::toInt};
#pragma endregion
#pragma region R5
    // R5
    //  Unknown encoding - Attribute<int> TouchPad2;
    //  Unknown encoding - Attribute<int> TouchPad1;

    Property<int> RB_TP_Blower{PropertyConverters::toInt};
    Property<int> RB_TP_Light{PropertyConverters::toInt};  /// Should this be a bool?
    /// @brief Auto enabled
    ///
    /// True when auto enabled
    Property<bool> RB_TP_Auto{PropertyConverters::toBool};
    /// @brief Heating running
    ///
    /// True when heating/cooling active
    Property<bool> RB_TP_Heater{PropertyConverters::toBool};
    /// @brief Cleaning (UV/Ozone running)
    ///
    /// True when Ozone/UV is cleaning spa.
    Property<bool> RB_TP_Ozone{PropertyConverters::toBool};
    /// @brief Sleeping
    ///
    /// True when spa is sleeping due to sleep timer
    Property<bool> RB_TP_Sleep{PropertyConverters::toBool};
    /// @brief Water temperature ('C)
    Property<int> WTMP{PropertyConverters::toInt};
    /// @brief Clean cycle running
    ///
    /// True when a clean cycle is running
    Property<bool> CleanCycle{PropertyConverters::toBool};
#pragma endregion
#pragma region R6
    // R6
    /// @brief Blower variable speed
    ///
    /// min 1, max 5
    Property<int> VARIValue{PropertyConverters::toInt};
    /// @brief Lights brightness
    ///
    /// min 1, max 5
    Property<int> LBRTValue{PropertyConverters::toInt};
    /// @brief Light colour
    ///
    /// min 0, max 31
    Property<int> CurrClr{PropertyConverters::toInt};
    /// @brief Lights mode
    ///
    /// 0 = white, 1 = colour, 2 = step, 3 = fade, 4 = party
    Property<int> ColorMode{PropertyConverters::toInt};
    /// @brief Light effect speed
    ///
    /// min 1, max 5
    Property<int> LSPDValue{PropertyConverters::toInt};
    /// @brief Filter run time (in hours) per block
    Property<int> FiltSetHrs{PropertyConverters::toInt};
    /// @brief Filter block duration (hours)
    Property<int> FiltBlockHrs{PropertyConverters::toInt};
    /// @brief Water temperature set point ('C)
    Property<int> STMP{PropertyConverters::toInt};
    // 1 = 12 hrs
    Property<int> L_24HOURS{PropertyConverters::toInt};
    /// @brief Power save level
    ///
    /// 0 = off, 1 = low, 2 = high
    Property<int> PSAV_LVL{PropertyConverters::toInt};
    /// @brief Peak power start time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> PSAV_BGN{PropertyConverters::toInt};
    /// @brief Peak power end time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> PSAV_END{PropertyConverters::toInt};
    /// @brief Sleep timer 1
    ///
    /// 128 = off, 127 = every day, 96 = weekends, 31 = weekdays
    Property<int> L_1SNZ_DAY{PropertyConverters::toInt};
    /// @brief Sleep timer 2
    ///
    /// 128 = off, 127 = every day, 96 = weekends, 31 = weekdays
    Property<int> L_2SNZ_DAY{PropertyConverters::toInt};
    /// @brief Sleep time 1 start time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> L_1SNZ_BGN{PropertyConverters::toInt};
    /// @brief Sleep time 2 start time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> L_2SNZ_BGN{PropertyConverters::toInt};
    /// @brief Sleep time 1 end time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> L_1SNZ_END{PropertyConverters::toInt};
    /// @brief Sleep time 1 end time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> L_2SNZ_END{PropertyConverters::toInt};
    /// @brief Default screen for control panels
    ///
    /// 0 = WTPM
    Property<int> DefaultScrn{PropertyConverters::toInt};
    /// @brief Time out duration (min)
    ///
    /// Time in min before pump and blower time out (min 10, max 30)
    Property<int> TOUT{PropertyConverters::toInt};
    Property<bool> VPMP{PropertyConverters::toBool};
    Property<bool> HIFI{PropertyConverters::toBool};
    /// @brief BRND
    ///
    /// 2 = VORT
    Property<int> BRND{PropertyConverters::toInt};
    /// @brief PRME
    ///
    /// 0 = 10secF
    Property<int> PRME{PropertyConverters::toInt};
    Property<int> ELMT{PropertyConverters::toInt};
    /// @brief TYPE
    ///
    /// 3 = SV3
    Property<int> TYPE{PropertyConverters::toInt};
    Property<int> GAS{PropertyConverters::toInt};
#pragma endregion
#pragma region R7
    // R7
    /// @brief Daily clean cycle start time
    ///
    /// Time with the formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> WCLNTime{PropertyConverters::toInt};
    /// @brief Use 'F instead of 'C as the temp. UMO
    Property<bool> TemperatureUnits{PropertyConverters::toBool};
    Property<bool> OzoneOff{PropertyConverters::toBool};
    /// @brief Sanitiser 24 hrs
    ///
    /// True if sanitiser (ozone) power outlet on permanently, false if automatically controlled.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<bool> Ozone24{PropertyConverters::toBool};
    /// @brief Circulation pump 24hrs
    ///
    /// True if circulation pump is always on, false if automatically controlled.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<bool> Circ24{PropertyConverters::toBool};
    Property<bool> CJET{PropertyConverters::toBool};
    /// @brief Variable heat element operation
    ///
    /// If true allows variable power to be fed to the heating element.
    /// See SV-Series-OEM-Install-Manual.pdf page 19.
    Property<bool> VELE{PropertyConverters::toBool};

    /// TODO #2 - Not Implemented
    /// @brief Date of comissioning
    /// Property<time_t> ComissionDate;

    /// @brief Highest voltage ever recorded (V)
    Property<int> V_Max{PropertyConverters::toInt};
    /// @brief Lowest voltage ever recorded (V)
    Property<int> V_Min{PropertyConverters::toInt};
    /// @brief Highest voltage in past 24 hrs (V)
    Property<int> V_Max_24{PropertyConverters::toInt};
    /// @brief Lowest voltage in past 24 hrs (V)
    Property<int> V_Min_24{PropertyConverters::toInt};
    Property<int> CurrentZero{PropertyConverters::toInt};
    Property<int> CurrentAdjust{PropertyConverters::toInt};
    Property<int> VoltageAdjust{PropertyConverters::toInt};
    Property<int> Ser1{PropertyConverters::toInt};
    Property<int> Ser2{PropertyConverters::toInt};
    Property<int> Ser3{PropertyConverters::toInt};
    /// @brief Variable heat element max power (A)
    ///
    /// Maximum current that the heat element is allowed to draw (between 3 and 25A)
    /// See SV-Series-OEM-Install-Manual.pdf page 19.
    Property<int> VMAX{PropertyConverters::toInt};
    /// @brief Adaptive Hysteresis
    ///
    /// Maximum adaptive hysteresis value (0=disabled).  See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<int> AHYS{PropertyConverters::toInt};
    /// @brief HUSE
    ///
    /// 1 = Off
    Property<bool> HUSE{PropertyConverters::toBool};
    /// @brief Heat pump active whilst spa is in use
    ///
    /// If false then when spa is in use then heat pump will not run to reduce noise levels
    /// See SV-Series-OEM-Install-Manual.pdf page 19.
    Property<bool> HELE{PropertyConverters::toBool};
    /// @brief Heatpump mode
    ///
    /// 0 = Auto, 1 = Heat, 2 = Cool, 3 = disabled
    Property<int> HPMP{PropertyConverters::toInt};
    /// @brief Varible pump minimum speed setting
    ///
    /// Min 20%, Max 100%
    /// See SV-Series-OEM-Install-Manual.pdf page 21.
    Property<int> PMIN{PropertyConverters::toInt};
    /// @brief Variable pump filtration speed setting
    ///
    /// Min 20%, Max 100%
    /// See SV-Series-OEM-Install-Manual.pdf page 21.
    Property<int> PFLT{PropertyConverters::toInt};
    /// @brief Varible pump heater speed setting
    ///
    /// Min 20%, Max 100%
    /// See SV-Series-OEM-Install-Manual.pdf page 21.
    Property<int> PHTR{PropertyConverters::toInt};
    /// @brief Varible pump maximum speed setting
    ///
    /// Maximum speed the varible pump will run at.
    /// Min 20%, Max 100%
    Property<int> PMAX{PropertyConverters::toInt};
#pragma endregion
#pragma region R9
    // R9
    /// @brief Fault runtime occurance (hrs)
    Property<int> F1_HR{PropertyConverters::toInt};
    /// @brief Fault time of day occurance
    Property<int> F1_Time{PropertyConverters::toInt};
    /// @brief Fault error codes
    ///
    /// 6 = ER612VOverload - High current detected on 12v line
    Property<int> F1_ER{PropertyConverters::toInt};
    /// @brief Supply current draw at time of error (A)
    Property<int> F1_I{PropertyConverters::toInt};
    /// @brief Supply voltage at time of error (V)
    Property<int> F1_V{PropertyConverters::toInt};
    /// @brief Pool temperature at time of error ('C)
    Property<int> F1_PT{PropertyConverters::toInt};
    /// @brief Heater temperature at time of error ('C)
    Property<int> F1_HT{PropertyConverters::toInt};
    Property<int> F1_CT{PropertyConverters::toInt};
    Property<int> F1_PU{PropertyConverters::toInt};
    Property<bool> F1_VE{PropertyConverters::toBool};
    /// @brief Heater setpoint at time of error ('C)
    Property<int> F1_ST{PropertyConverters::toInt};
#pragma endregion
#pragma region RA
    // RA
    /// @brief Fault runtime occurance (hrs)
    Property<int> F2_HR{PropertyConverters::toInt};
    /// @brief Fault time of day occurance
    Property<int> F2_Time{PropertyConverters::toInt};
    /// @brief Fault error codes
    ///
    /// 6 = ER612VOverload - High current detected on 12v line
    Property<int> F2_ER{PropertyConverters::toInt};
    /// @brief Supply current draw at time of error (A)
    Property<int> F2_I{PropertyConverters::toInt};
    /// @brief Supply voltage at time of error (V)
    Property<int> F2_V{PropertyConverters::toInt};
    /// @brief Pool temperature at time of error ('C)
    Property<int> F2_PT{PropertyConverters::toInt};
    /// @brief Heater temperature at time of error ('C)
    Property<int> F2_HT{PropertyConverters::toInt};
    Property<int> F2_CT{PropertyConverters::toInt};
    Property<int> F2_PU{PropertyConverters::toInt};
    Property<bool> F2_VE{PropertyConverters::toBool};
    /// @brief Heater setpoint at time of error ('C)
    Property<int> F2_ST{PropertyConverters::toInt};
#pragma endregion
#pragma region RB
    // RB
    /// @brief Fault runtime occurance (hrs)
    Property<int> F3_HR{PropertyConverters::toInt};
    /// @brief Fault time of day occurance
    Property<int> F3_Time{PropertyConverters::toInt};
    /// @brief Fault error codes
    ///
    /// 6 = ER612VOverload - High current detected on 12v line
    Property<int> F3_ER{PropertyConverters::toInt};
    /// @brief Supply current draw at time of error (A)
    Property<int> F3_I{PropertyConverters::toInt};
    /// @brief Supply voltage at time of error (V)
    Property<int> F3_V{PropertyConverters::toInt};
    /// @brief Pool temperature at time of error ('C)
    Property<int> F3_PT{PropertyConverters::toInt};
    /// @brief Heater temperature at time of error ('C)
    Property<int> F3_HT{PropertyConverters::toInt};
    Property<int> F3_CT{PropertyConverters::toInt};
    Property<int> F3_PU{PropertyConverters::toInt};
    Property<bool> F3_VE{PropertyConverters::toBool};
    /// @brief Heater setpoint at time of error ('C)
    Property<int> F3_ST{PropertyConverters::toInt};
#pragma endregion
#pragma region RC
    // RC
    //  Encoding of the RC registers is not obvious
    // Property<bool> Outlet_Heater{PropertyConverters::toBool};
    // Property<bool> Outlet_Circ{PropertyConverters::toBool};
    // Property<bool> Outlet_Sanitise{PropertyConverters::toBool};
    // Property<bool> Outlet_Pump1{PropertyConverters::toBool};
    // Property<bool> Outlet_Pump2{PropertyConverters::toBool};
    // Property<bool> Outlet_Pump4{PropertyConverters::toBool};
    // Property<bool> Outlet_Pump5{PropertyConverters::toBool};
    /// @brief Blower status
    ///
    /// 0 = variable mode, 1 = ramp mode, 2 = off
    Property<int> Outlet_Blower{PropertyConverters::toInt};
#pragma endregion
#pragma region RE
    // RE
    /// @brief Heatpump installed / interface version
    Property<int> HP_Present{PropertyConverters::toInt};
    // Encoding of these registers is not clear
    // Property<bool> HP_FlowSwitch{PropertyConverters::toBool};
    // Property<bool> HP_HighSwitch{PropertyConverters::toBool};
    // Property<bool> HP_LowSwitch{PropertyConverters::toBool};
    // Property<bool> HP_CompCutOut{PropertyConverters::toBool};
    // Property<bool> HP_ExCutOut{PropertyConverters::toBool};
    // Property<bool> HP_D1{PropertyConverters::toBool};
    // Property<bool> HP_D2{PropertyConverters::toBool};
    // Property<bool> HP_D3{PropertyConverters::toBool};
    /// @brief Ambient air temperature ('C)
    Property<int> HP_Ambient{PropertyConverters::toInt};
    /// @brief Compressor temperature ('C)
    Property<int> HP_Condensor{PropertyConverters::toInt};
    /// @brief Compressor running
    Property<bool> HP_Compressor_State{PropertyConverters::toBool};
    /// @brief Fan running
    Property<bool> HP_Fan_State{PropertyConverters::toBool};
    Property<bool> HP_4W_Valve{PropertyConverters::toBool};
    Property<bool> HP_Heater_State{PropertyConverters::toBool};
    /// @brief Heatpump state
    ///
    /// 0 = Standby
    Property<int> HP_State{PropertyConverters::toInt};
    /// @brief Heatpump mode
    ///
    /// 1 = Heat
    Property<int> HP_Mode{PropertyConverters::toInt};
    Property<int> HP_Defrost_Timer{PropertyConverters::toInt};
    Property<int> HP_Comp_Run_Timer{PropertyConverters::toInt};
    Property<int> HP_Low_Temp_Timer{PropertyConverters::toInt};
    Property<int> HP_Heat_Accum_Timer{PropertyConverters::toInt};
    Property<int> HP_Sequence_Timer{PropertyConverters::toInt};
    Property<int> HP_Warning{PropertyConverters::toInt};
    Property<int> FrezTmr{PropertyConverters::toInt};
    Property<int> DBGN{PropertyConverters::toInt};
    Property<int> DEND{PropertyConverters::toInt};
    Property<int> DCMP{PropertyConverters::toInt};
    Property<int> DMAX{PropertyConverters::toInt};
    Property<int> DELE{PropertyConverters::toInt};
    Property<int> DPMP{PropertyConverters::toInt};
// Property<int> CMAX{PropertyConverters::toInt};
// Property<int> HP_Compressor{PropertyConverters::toInt};
// Property<int> HP_Pump_State{PropertyConverters::toInt};
// Property<int> HP_Status{PropertyConverters::toInt};
#pragma endregion
#pragma region RG

    Property<bool> Pump5OkToRun{PropertyConverters::toBool};
    /// @brief Lock mode
    ///
    /// 0 = keypad unlocked, 1 = partial lock, 2 = full lock
    Property<int> LockMode{PropertyConverters::toInt};

#pragma endregion

    boolean update_SpaTime(String year, String month, String day, String hour, String minute, String second);

    // Array access method
    [[nodiscard]] PumpState& pump(size_t index) { 
        return pumps.at(index); 
    }
    
    [[nodiscard]] const PumpState& pump(size_t index) const { 
        return pumps.at(index); 
    }
};
#endif