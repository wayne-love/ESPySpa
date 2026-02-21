#ifndef SPAPROPERTIES_H
#define SPAPROPERTIES_H

#include <Arduino.h>
#include <RemoteDebug.h>
#include <time.h>
#include <TimeLib.h>
#include <array>


template <typename T>
// Simple value holder with an optional change callback.
class Property
{
private:
    T _value;
    void (*_callback)(T) = nullptr;

public:
    T getValue() { return _value; }
    void update_Value(T newval)
    {
        T oldvalue = _value;
        _value = newval;
        // Only notify on actual value change.
        if ((_callback) && (oldvalue != newval))
            {
                _callback(_value);
            }
    };
    void setCallback(void (*c)(T)) { _callback = c; };
    void clearCallback() { _callback = nullptr; };
};

/// @brief represents the properties of the spa.
class SpaProperties
{
    
private:

#pragma region R2
    /// @brief Mains voltage (V)
    Property<int> MainsVoltage;
    /// @brief Internal case temperature ('C)
    Property<int> CaseTemperature;
    /// @brief 12v port current (mA)
    Property<int> PortCurrent;

    /// @brief Current day of week on Spa RTC
    /// 0 = Monday, 1 = Tuesday, ..., 6 = Sunday
    Property<int> SpaDayOfWeek;
        /// @brief Current time on Spa RTC
    Property<time_t> SpaTime;
    /// @brief Heater temperature ('C)
    Property<int> HeaterTemperature;
    /// @brief Pool temperature ('C). Note this seems to return rubbish most of the time.
    Property<int> PoolTemperature;
    /// @brief Water present
    Property<bool> WaterPresent;
    /// @brief AwakeMinutesRemaining (min)
    Property<int> AwakeMinutesRemaining;
    /// @brief FiltPumpRunTimeTotal (min)
    Property<int> FiltPumpRunTimeTotal;
    /// @brief FiltPumpReqMins
    // TODO - the value here does not match the value in the snampshot data (1442 != 2:00)
    Property<int> FiltPumpReqMins;
    /// @brief LoadTimeOut (sec)
    Property<int> LoadTimeOut;
    /// @brief HourMeter (hours)
    Property<int> HourMeter;
    /// @brief Relay1 (?)
    Property<int> Relay1;
    /// @brief Relay2 (?)
    Property<int> Relay2;
    /// @brief Relay3 (?)
    Property<int> Relay3;
    /// @brief Relay4 (?)
    Property<int> Relay4;
    /// @brief Relay5 (?)
    Property<int> Relay5;
    /// @brief Relay6 (?)
    Property<int> Relay6;
    /// @brief Relay7 (?)
    Property<int> Relay7;
    /// @brief Relay8 (?)
    Property<int> Relay8;
    /// @brief Relay9 (?)
    Property<int> Relay9;
#pragma endregion
#pragma region R3
    // R3
    /// @brief Current limit (A)
    Property<int> CLMT;
    /// @brief Power phases in use
    Property<int> PHSE;
    /// @brief Load limit - Phase 1
    ///
    /// Number of services that can be active on this phase before the keypad stops new services from starting.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<int> LLM1;
    /// @brief Load limit - Phase 2
    ///
    /// Number of services that can be active on this phase before the keypad stops new services from starting.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<int> LLM2;
    /// @brief Load limit - Phase 3
    ///
    /// Number of services that can be active on this phase before the keypad stops new services from starting.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<int> LLM3;
    /// @brief Software version
    Property<String> SVER;
    /// @brief Model
    Property<String> Model; 
    /// @brief SerialNo1
    Property<String> SerialNo1;
    /// @brief SerialNo2
    Property<String> SerialNo2;
    /// @brief Dipswitch 1
    Property<bool> D1;
    /// @brief Dipswitch 2
    Property<bool> D2;
    /// @brief Dipswitch 3
    Property<bool> D3;
    /// @brief Dipswitch 4
    Property<bool> D4;
    /// @brief Dipswitch 5
    Property<bool> D5;
    /// @brief Dipswitch 6
    Property<bool> D6;
    /// @brief Pump
    Property<String> Pump;
    /// @brief Load shed count
    ///
    /// Number of services active at which the heater will turn itself off.
    Property<int> LS;
    /// @brief HV
    Property<bool> HV;
    /// @brief MR / name clash with MR constant from specreg.h
    Property<int> SnpMR;
    /// @brief Status (Filtering, etc)
    Property<String> Status;
    /// @brief PrimeCount
    Property<int> PrimeCount;
    /// @brief Heat element current draw (A)
    Property<int> EC;
    /// @brief HAMB
    Property<int> HAMB;
    /// @brief HCON
    Property<int> HCON;
// Unclear encoding of HV_2
/// @brief HV_2
Property<bool> HV_2;
//
#pragma endregion
#pragma region R4
    // R4
    /// @brief Operation mode
    ///
    /// One of NORM, ECON, AWAY, WEEK
    Property<String> Mode;
    /// @brief Service Timer 1 (wks) 0 = off
    Property<int> Ser1_Timer;
    /// @brief Service Timer 2 (wks) 0 = off
    Property<int> Ser2_Timer;
    /// @brief Service Timer 3 (wks) 0 = off
    Property<int> Ser3_Timer;
    /// @brief Heat mode
    ///
    /// 1 = Standby
    /// 2 = HeatMix
    Property<int> HeatMode;
    /// @brief Pump idle time (sec)
    Property<int> PumpIdleTimer;
    /// @brief Pump run time (sec)
    Property<int> PumpRunTimer;
    /// @brief Pool temperature adaptive hysteresis
    Property<int> AdtPoolHys;
    /// @brief  Heater temperature adaptive hysteresis
    Property<int> AdtHeaterHys;
    /// @brief Power consumtion * 10
    Property<int> Power; 
    Property<int> Power_kWh;
    // (kWh)
    Property<int> Power_Today;
    // (kWh)
    Property<int> Power_Yesterday;
    // 0 = ok
    Property<int> ThermalCutOut;
    Property<int> Test_D1;
    Property<int> Test_D2;
    Property<int> Test_D3;
    Property<int> ElementHeatSourceOffset;
    Property<int> Frequency;
    Property<int> HPHeatSourceOffset_Heat;
    // 100 = 0!?
    Property<int> HPHeatSourceOffset_Cool;
    Property<int> HeatSourceOffTime;
    Property<int> Vari_Speed;
    Property<int> Vari_Percent;
    // 5 = Filt
    // 4 = Off
    /// @brief Varible speed mode
    ///
    /// 5 = Filtering, 4 = Off
    Property<int> Vari_Mode;
#pragma endregion
#pragma region R5
    // R5
    //  Unknown encoding - Attribute<int> TouchPad2;
    //  Unknown encoding - Attribute<int> TouchPad1;
    /// @brief Pump 1 state
    ///
    /// 0 = off, 1 = running, 4 = auto
    Property<int> RB_TP_Pump1;
    /// @brief Pump 2 state
    ///
    /// 0 = off, 1 = running, 4 = auto
    Property<int> RB_TP_Pump2;
    /// @brief Pump 3 state
    ///
    /// 0 = off, 1 = running, 4 = auto
    Property<int> RB_TP_Pump3;
    /// @brief Pump 4 state
    ///
    /// 0 = off, 1 = running, 4 = auto
    Property<int> RB_TP_Pump4;
    /// @brief Pump 5 state
    ///
    /// 0 = off, 1 = running, 4 = auto
    Property<int> RB_TP_Pump5;
    Property<int> RB_TP_Blower;
    Property<int> RB_TP_Light;
    /// @brief Auto enabled
    ///
    /// True when auto enabled
    Property<bool> RB_TP_Auto;
    /// @brief Heating running
    ///
    /// True when heating/cooling active
    Property<bool> RB_TP_Heater;
    /// @brief Cleaning (UV/Ozone running)
    ///
    /// True when Ozone/UV is cleaning spa.
    Property<bool> RB_TP_Ozone;
    /// @brief Sleeping
    ///
    /// True when spa is sleeping due to sleep timer
    Property<bool> RB_TP_Sleep;
    /// @brief Water temperature ('C)
    Property<int> WTMP;
    /// @brief Clean cycle running
    ///
    /// True when a clean cycle is running
    Property<bool> CleanCycle;
#pragma endregion
#pragma region R6
    // R6
    /// @brief Blower variable speed
    ///
    /// min 1, max 5
    Property<int> VARIValue;
    /// @brief Lights brightness
    ///
    /// min 1, max 5
    Property<int> LBRTValue;
    /// @brief Light colour
    ///
    /// min 0, max 31
    Property<int> CurrClr;
    /// @brief Light effect speed
    ///
    /// min 1, max 5
    Property<int> LSPDValue;
    /// @brief Filter run time (in hours) per block
    Property<int> FiltSetHrs;
    /// @brief Filter block duration (hours)
    Property<int> FiltBlockHrs;
    /// @brief Water temperature set point ('C)
    // 1 = 12 hrs
    Property<int> L_24HOURS;
    /// @brief Power save level
    ///
    /// 0 = off, 1 = low, 2 = high
    Property<int> PSAV_LVL;
    /// @brief Peak power start time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> PSAV_BGN;
    /// @brief Peak power end time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> PSAV_END;

    /// @brief Sleep time 1 start time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> L_1SNZ_BGN;
    /// @brief Sleep time 2 start time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> L_2SNZ_BGN;
    /// @brief Sleep time 1 end time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> L_1SNZ_END;
    /// @brief Sleep time 1 end time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> L_2SNZ_END;
    /// @brief Default screen for control panels
    ///
    /// 0 = WTPM
    Property<int> DefaultScrn;
    /// @brief Time out duration (min)
    ///
    /// Time in min before pump and blower time out (min 10, max 30)
    Property<int> TOUT;
    Property<bool> VPMP;
    Property<bool> HIFI;
    /// @brief BRND
    ///
    /// 2 = VORT
    Property<int> BRND;
    /// @brief PRME
    ///
    /// 0 = 10secF
    Property<int> PRME;
    Property<int> ELMT;
    /// @brief TYPE
    ///
    /// 3 = SV3
    Property<int> TYPE;
    Property<int> GAS;
#pragma endregion
#pragma region R7
    // R7
    /// @brief Daily clean cycle start time
    ///
    /// Time with the formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    Property<int> WCLNTime;
    /// @brief Use 'F instead of 'C as the temp. UMO
    Property<bool> TemperatureUnits;
    Property<bool> OzoneOff;
    /// @brief Sanitiser 24 hrs
    ///
    /// True if sanitiser (ozone) power outlet on permanently, false if automatically controlled.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<bool> Ozone24;
    /// @brief Circulation pump 24hrs
    ///
    /// True if circulation pump is always on, false if automatically controlled.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<bool> Circ24;
    Property<bool> CJET;
    /// @brief Variable heat element operation
    ///
    /// If true allows variable power to be fed to the heating element.
    /// See SV-Series-OEM-Install-Manual.pdf page 19.
    Property<bool> VELE;

    /// TODO #2 - Not Implemented
    /// @brief Date of comissioning
    /// Property<time_t> ComissionDate;

    /// @brief Highest voltage ever recorded (V)
    Property<int> V_Max;
    /// @brief Lowest voltage ever recorded (V)
    Property<int> V_Min;
    /// @brief Highest voltage in past 24 hrs (V)
    Property<int> V_Max_24;
    /// @brief Lowest voltage in past 24 hrs (V)
    Property<int> V_Min_24;
    Property<int> CurrentZero;
    Property<int> CurrentAdjust;
    Property<int> VoltageAdjust;
    Property<int> Ser1;
    Property<int> Ser2;
    Property<int> Ser3;
    /// @brief Variable heat element max power (A)
    ///
    /// Maximum current that the heat element is allowed to draw (between 3 and 25A)
    /// See SV-Series-OEM-Install-Manual.pdf page 19.
    Property<int> VMAX;
    /// @brief Adaptive Hysteresis
    ///
    /// Maximum adaptive hysteresis value (0=disabled).  See SV-Series-OEM-Install-Manual.pdf page 20.
    Property<int> AHYS;
    /// @brief HUSE
    ///
    /// 1 = Off
    Property<bool> HUSE;
    /// @brief Heat pump active whilst spa is in use
    ///
    /// If false then when spa is in use then heat pump will not run to reduce noise levels
    /// See SV-Series-OEM-Install-Manual.pdf page 19.
    Property<bool> HELE;
    /// @brief Varible pump minimum speed setting
    ///
    /// Min 20%, Max 100%
    /// See SV-Series-OEM-Install-Manual.pdf page 21.
    Property<int> PMIN;
    /// @brief Variable pump filtration speed setting
    ///
    /// Min 20%, Max 100%
    /// See SV-Series-OEM-Install-Manual.pdf page 21.
    Property<int> PFLT;
    /// @brief Varible pump heater speed setting
    ///
    /// Min 20%, Max 100%
    /// See SV-Series-OEM-Install-Manual.pdf page 21.
    Property<int> PHTR;
    /// @brief Varible pump maximum speed setting
    ///
    /// Maximum speed the varible pump will run at.
    /// Min 20%, Max 100%
    Property<int> PMAX;
#pragma endregion
#pragma region R9
    // R9
    /// @brief Fault runtime occurance (hrs)
    Property<int> F1_HR;
    /// @brief Fault time of day occurance
    Property<int> F1_Time;
    /// @brief Fault error codes
    ///
    /// 6 = ER612VOverload - High current detected on 12v line
    Property<int> F1_ER;
    /// @brief Supply current draw at time of error (A)
    Property<int> F1_I;
    /// @brief Supply voltage at time of error (V)
    Property<int> F1_V;
    /// @brief Pool temperature at time of error ('C)
    Property<int> F1_PT;
    /// @brief Heater temperature at time of error ('C)
    Property<int> F1_HT;
    Property<int> F1_CT;
    Property<int> F1_PU;
    Property<bool> F1_VE;
    /// @brief Heater setpoint at time of error ('C)
    Property<int> F1_ST;
#pragma endregion
#pragma region RA
    // RA
    /// @brief Fault runtime occurance (hrs)
    Property<int> F2_HR;
    /// @brief Fault time of day occurance
    Property<int> F2_Time;
    /// @brief Fault error codes
    ///
    /// 6 = ER612VOverload - High current detected on 12v line
    Property<int> F2_ER;
    /// @brief Supply current draw at time of error (A)
    Property<int> F2_I;
    /// @brief Supply voltage at time of error (V)
    Property<int> F2_V;
    /// @brief Pool temperature at time of error ('C)
    Property<int> F2_PT;
    /// @brief Heater temperature at time of error ('C)
    Property<int> F2_HT;
    Property<int> F2_CT;
    Property<int> F2_PU;
    Property<bool> F2_VE;
    /// @brief Heater setpoint at time of error ('C)
    Property<int> F2_ST;
#pragma endregion
#pragma region RB
    // RB
    /// @brief Fault runtime occurance (hrs)
    Property<int> F3_HR;
    /// @brief Fault time of day occurance
    Property<int> F3_Time;
    /// @brief Fault error codes
    ///
    /// 6 = ER612VOverload - High current detected on 12v line
    Property<int> F3_ER;
    /// @brief Supply current draw at time of error (A)
    Property<int> F3_I;
    /// @brief Supply voltage at time of error (V)
    Property<int> F3_V;
    /// @brief Pool temperature at time of error ('C)
    Property<int> F3_PT;
    /// @brief Heater temperature at time of error ('C)
    Property<int> F3_HT;
    Property<int> F3_CT;
    Property<int> F3_PU;
    Property<bool> F3_VE;
    /// @brief Heater setpoint at time of error ('C)
    Property<int> F3_ST;
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
    Property<int> Outlet_Blower;
#pragma endregion
#pragma region RE
    // RE
    /// @brief Heatpump installed / interface version
    Property<int> HP_Present;
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
    Property<int> HP_Ambient;
    /// @brief Compressor temperature ('C)
    Property<int> HP_Condensor;
    /// @brief Compressor running
    Property<bool> HP_Compressor_State;
    /// @brief Fan running
    Property<bool> HP_Fan_State;
    Property<bool> HP_4W_Valve;
    Property<bool> HP_Heater_State;
    /// @brief Heatpump state
    ///
    /// 0 = Standby
    Property<int> HP_State;
    /// @brief Heatpump mode
    ///
    /// 1 = Heat
    Property<int> HP_Mode;
    Property<int> HP_Defrost_Timer;
    Property<int> HP_Comp_Run_Timer;
    Property<int> HP_Low_Temp_Timer;
    Property<int> HP_Heat_Accum_Timer;
    Property<int> HP_Sequence_Timer;
    Property<int> HP_Warning;
    Property<int> FrezTmr;
    Property<int> DBGN;
    Property<int> DEND;
    Property<int> DCMP;
    Property<int> DMAX;
    Property<int> DELE;
    Property<int> DPMP;
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
    Property<String> Pump1InstallState;
    /// @brief Pump 2 install state
    ///
    /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
    /// means it is), the second part (1- above) indicates it's speed type. The third
    /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
    Property<String> Pump2InstallState;
    /// @brief Pump 3 install state
    ///
    /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
    /// means it is), the second part (1- above) indicates it's speed type. The third
    /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
    Property<String> Pump3InstallState;
    /// @brief Pump 4 install state
    ///
    /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
    /// means it is), the second part (1- above) indicates it's speed type. The third
    /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
    Property<String> Pump4InstallState;
    /// @brief Pump 5 install state
    ///
    /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
    /// means it is), the second part (1- above) indicates it's speed type. The third
    /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
    Property<String> Pump5InstallState;
    /// @brief Pump 1 is in safe state to start
    Property<bool> Pump1OkToRun;
    /// @brief Pump 2 is in safe state to start
    Property<bool> Pump2OkToRun;
    /// @brief Pump 3 is in safe state to start
    Property<bool> Pump3OkToRun;
    /// @brief Pump 4 is in safe state to start
    Property<bool> Pump4OkToRun;
    /// @brief Pump 5 is in safe state to start
    Property<bool> Pump5OkToRun;
    /// @brief Lock mode
    ///
    /// 0 = keypad unlocked, 1 = partial lock, 2 = full lock
    Property<int> LockMode;

#pragma endregion


protected:
#pragma region R2
    boolean update_SpaDayOfWeek(const String&);
    boolean update_SpaTime(const String& year, const String& month, const String& day, const String& hour, const String& minute, const String& second);
    boolean update_MainsVoltage(const String&);
    boolean update_CaseTemperature(const String&);
    boolean update_PortCurrent(const String&);
    boolean update_SpaTime(const String&);
    boolean update_HeaterTemperature(const String&);
    boolean update_PoolTemperature(const String&);
    boolean update_WaterPresent(const String&);
    boolean update_AwakeMinutesRemaining(const String&);
    boolean update_FiltPumpRunTimeTotal(const String&);
    boolean update_FiltPumpReqMins(const String&);
    boolean update_LoadTimeOut(const String&);
    boolean update_HourMeter(const String&);
    boolean update_Relay1(const String&);
    boolean update_Relay2(const String&);
    boolean update_Relay3(const String&);
    boolean update_Relay4(const String&);
    boolean update_Relay5(const String&);
    boolean update_Relay6(const String&);
    boolean update_Relay7(const String&);
    boolean update_Relay8(const String&);
    boolean update_Relay9(const String&);
#pragma endregion
#pragma region R3
    boolean update_CLMT(const String&);
    boolean update_PHSE(const String&);
    boolean update_LLM1(const String&);
    boolean update_LLM2(const String&);
    boolean update_LLM3(const String&);
    boolean update_SVER(const String&);
    boolean update_Model(const String&);
    boolean update_SerialNo1(const String&);
    boolean update_SerialNo2(const String&);
    boolean update_D1(const String&);
    boolean update_D2(const String&);
    boolean update_D3(const String&);
    boolean update_D4(const String&);
    boolean update_D5(const String&);
    boolean update_D6(const String&);
    boolean update_Pump(const String&);
    boolean update_LS(const String&);
    boolean update_HV(const String&);
    boolean update_SnpMR(const String&);
    boolean update_Status(const String&);
    boolean update_PrimeCount(const String&);
    boolean update_EC(const String&);
    boolean update_HAMB(const String&);
    boolean update_HCON(const String&);
//    boolean update_HV_2(const String&);
#pragma endregion
#pragma region R4
    boolean update_Mode(const String&);
    boolean update_Ser1_Timer(const String&);
    boolean update_Ser2_Timer(const String&);
    boolean update_Ser3_Timer(const String&);
    boolean update_HeatMode(const String&);
    boolean update_PumpIdleTimer(const String&);
    boolean update_PumpRunTimer(const String&);
    boolean update_AdtPoolHys(const String&);
    boolean update_AdtHeaterHys(const String&);
    boolean update_Power(const String&);
    boolean update_Power_kWh(const String&);
    boolean update_Power_Today(const String&);
    boolean update_Power_Yesterday(const String&);
    boolean update_ThermalCutOut(const String&);
    boolean update_Test_D1(const String&);
    boolean update_Test_D2(const String&);
    boolean update_Test_D3(const String&);
    boolean update_ElementHeatSourceOffset(const String&);
    boolean update_Frequency(const String&);
    boolean update_HPHeatSourceOffset_Heat(const String&);
    boolean update_HPHeatSourceOffset_Cool(const String&);
    boolean update_HeatSourceOffTime(const String&);
    boolean update_Vari_Speed(const String&);
    boolean update_Vari_Percent(const String&);
    boolean update_Vari_Mode(const String&);
#pragma endregion
#pragma region R5
    // R5
    //  Unknown encoding - TouchPad2.update_Value();
    //  Unknown encoding - TouchPad1.update_Value();
    boolean update_RB_TP_Pump1(const String&);
    boolean update_RB_TP_Pump2(const String&);
    boolean update_RB_TP_Pump3(const String&);
    boolean update_RB_TP_Pump4(const String&);
    boolean update_RB_TP_Pump5(const String&);
    boolean update_RB_TP_Blower(const String&);
    boolean update_RB_TP_Light(const String&);
    boolean update_RB_TP_Auto(const String&);
    boolean update_RB_TP_Heater(const String&);
    boolean update_RB_TP_Ozone(const String&);
    boolean update_RB_TP_Sleep(const String&);
    boolean update_WTMP(const String&);
    boolean update_CleanCycle(const String&);
#pragma endregion
#pragma region R6
    boolean update_VARIValue(const String&);
    boolean update_LBRTValue(const String&);
    boolean update_CurrClr(const String&);
    boolean update_LSPDValue(const String&);
    boolean update_FiltHrs(const String&);
    boolean update_FiltBlockHrs(const String&);
    boolean update_L_24HOURS(const String&);
    boolean update_PSAV_LVL(const String&);
    boolean update_PSAV_BGN(const String&);
    boolean update_PSAV_END(const String&);
    boolean update_DefaultScrn(const String&);
    boolean update_TOUT(const String&);
    boolean update_VPMP(const String&);
    boolean update_HIFI(const String&);
    boolean update_BRND(const String&);
    boolean update_PRME(const String&);
    boolean update_ELMT(const String&);
    boolean update_TYPE(const String&);
    boolean update_GAS(const String&);
#pragma endregion
#pragma region R7
    boolean update_WCLNTime(const String&);
    // The following 2 may be reversed
    boolean update_TemperatureUnits(const String&);
    boolean update_OzoneOff(const String&);
    boolean update_Ozone24(const String&);
    // The following 2 may be reversed
    boolean update_Circ24(const String&);
    boolean update_CJET(const String&);
    boolean update_VELE(const String&);
    boolean update_V_Max(const String&);
    boolean update_V_Min(const String&);
    boolean update_V_Max_24(const String&);
    boolean update_V_Min_24(const String&);
    boolean update_CurrentZero(const String&);
    boolean update_CurrentAdjust(const String&);
    boolean update_VoltageAdjust(const String&);
    boolean update_Ser1(const String&);
    boolean update_Ser2(const String&);
    boolean update_Ser3(const String&);
    boolean update_VMAX(const String&);
    boolean update_AHYS(const String&);
    boolean update_HUSE(const String&);
    boolean update_HELE(const String&);
    boolean update_PMIN(const String&);
    boolean update_PFLT(const String&);
    boolean update_PHTR(const String&);
    boolean update_PMAX(const String&);
#pragma endregion
#pragma region R9
    boolean update_F1_HR(const String&);
    boolean update_F1_Time(const String&);
    boolean update_F1_ER(const String&);
    boolean update_F1_I(const String&);
    boolean update_F1_V(const String&);
    boolean update_F1_PT(const String&);
    boolean update_F1_HT(const String&);
    boolean update_F1_CT(const String&);
    boolean update_F1_PU(const String&);
    boolean update_F1_VE(const String&);
    boolean update_F1_ST(const String&);
#pragma endregion
#pragma region RA
    boolean update_F2_HR(const String&);
    boolean update_F2_Time(const String&);
    boolean update_F2_ER(const String&);
    boolean update_F2_I(const String&);
    boolean update_F2_V(const String&);
    boolean update_F2_PT(const String&);
    boolean update_F2_HT(const String&);
    boolean update_F2_CT(const String&);
    boolean update_F2_PU(const String&);
    boolean update_F2_VE(const String&);
    boolean update_F2_ST(const String&);
#pragma endregion
#pragma region RB
    boolean update_F3_HR(const String&);
    boolean update_F3_Time(const String&);
    boolean update_F3_ER(const String&);
    boolean update_F3_I(const String&);
    boolean update_F3_V(const String&);
    boolean update_F3_PT(const String&);
    boolean update_F3_HT(const String&);
    boolean update_F3_CT(const String&);
    boolean update_F3_PU(const String&);
    boolean update_F3_VE(const String&);
    boolean update_F3_ST(const String&);
#pragma endregion
#pragma region RC
    // Outlet_Heater.update_Value(String);
    // Outlet_Circ.update_Value(String);
    // Outlet_Sanitise.update_Value(String);
    // Outlet_Pump1.update_Value(String);
    // Outlet_Pump2.update_Value(String);
    // Outlet_Pump4.update_Value(String);
    // Outlet_Pump5.update_Value(String);
    boolean update_Outlet_Blower(const String&);
#pragma endregion
#pragma region RE
    boolean update_HP_Present(const String&);
    // HP_FlowSwitch.update_Value(String);
    // HP_HighSwitch.update_Value(String);
    // HP_LowSwitch.update_Value(String);
    // HP_CompCutOut.update_Value(String);
    // HP_ExCutOut.update_Value(String);
    // HP_D1.update_Value(String);
    // HP_D2.update_Value(String);
    // HP_D3.update_Value(String);
    boolean update_HP_Ambient(const String&);
    boolean update_HP_Condensor(const String&);
    boolean update_HP_Compressor_State(const String&);
    boolean update_HP_Fan_State(const String&);
    boolean update_HP_4W_Valve(const String&);
    boolean update_HP_Heater_State(const String&);
    boolean update_HP_State(const String&);
    boolean update_HP_Mode(const String&);
    boolean update_HP_Defrost_Timer(const String&);
    boolean update_HP_Comp_Run_Timer(const String&);
    boolean update_HP_Low_Temp_Timer(const String&);
    boolean update_HP_Heat_Accum_Timer(const String&);
    boolean update_HP_Sequence_Timer(const String&);
    boolean update_HP_Warning(const String&);
    boolean update_FrezTmr(const String&);
    boolean update_DBGN(const String&);
    boolean update_DEND(const String&);
    boolean update_DCMP(const String&);
    boolean update_DMAX(const String&);
    boolean update_DELE(const String&);
    boolean update_DPMP(const String&);
// CMAX.update_Value(String);
// HP_Compressor.update_Value(String);
// HP_Pump_State.update_Value(String);
// HP_Status.update_Value(String);
#pragma endregion
#pragma region RG
    boolean update_Pump1InstallState(const String&);
    boolean update_Pump2InstallState(const String&);
    boolean update_Pump3InstallState(const String&);
    boolean update_Pump4InstallState(const String&);
    boolean update_Pump5InstallState(const String&);
    boolean update_Pump1OkToRun(const String&);
    boolean update_Pump2OkToRun(const String&);
    boolean update_Pump3OkToRun(const String&);
    boolean update_Pump4OkToRun(const String&);
    boolean update_Pump5OkToRun(const String&);
    boolean update_LockMode(const String&);
#pragma endregion


public:
    int getMainsVoltage() { return MainsVoltage.getValue(); }
    void setMainsVoltageCallback(void (*callback)(int)) { MainsVoltage.setCallback(callback); }

    /// @brief Gets current case temperature multiplied by 10 (245 = 24.5 actual)
    /// @return 
    int getCaseTemperature() { return CaseTemperature.getValue(); }
    void setCaseTemperatureCallback(void (*callback)(int)) { CaseTemperature.setCallback(callback); }

    int getPortCurrent() { return PortCurrent.getValue(); }
    void setPortCurrentCallback(void (*callback)(int)) { PortCurrent.setCallback(callback); }

    /// @brief Gets the day of week from the spa
    /// @return
    int getSpaDayOfWeek() { return SpaDayOfWeek.getValue(); }
    void SpaDayOfWeekCallback(void (*callback)(int)) { SpaDayOfWeek.setCallback(callback); }
    const std::array <String, 7> spaDayOfWeekStrings = {"Monday","Tuesday", "Wednesday","Thursday","Friday","Saturday","Sunday"};

    /// @brief Gets the current time from the spa clock
    /// @return 
    time_t getSpaTime() { return SpaTime.getValue(); }
    void setSpaTimeCallback(void (*callback)(time_t)) { SpaTime.setCallback(callback); }
    

    /// @brief Get current heater temperature multiplied by 10 (245 = 24.5 actual)
    /// @return 
    int getHeaterTemperature() { return HeaterTemperature.getValue(); }
    void setHeaterTemperatureCallback(void (*callback)(int)) { HeaterTemperature.setCallback(callback); }

    int getPoolTemperature() { return PoolTemperature.getValue(); }
    void setPoolTemperatureCallback(void (*callback)(int)) { PoolTemperature.setCallback(callback); }

    bool getWaterPresent() { return WaterPresent.getValue(); }
    void setWaterPresentCallback(void (*callback)(bool)) { WaterPresent.setCallback(callback); }

    int getAwakeMinutesRemaining() { return AwakeMinutesRemaining.getValue(); }
    void setAwakeMinutesRemainingCallback(void (*callback)(int)) { AwakeMinutesRemaining.setCallback(callback); }

    int getFiltPumpRunTimeTotal() { return FiltPumpRunTimeTotal.getValue(); }
    void setFiltPumpRunTimeTotalCallback(void (*callback)(int)) { FiltPumpRunTimeTotal.setCallback(callback); }

    int getFiltPumpReqMins() { return FiltPumpReqMins.getValue(); }
    void setFiltPumpReqMinsCallback(void (*callback)(int)) { FiltPumpReqMins.setCallback(callback); }

    int getLoadTimeOut() { return LoadTimeOut.getValue(); }
    void setLoadTimeOutCallback(void (*callback)(int)) { LoadTimeOut.setCallback(callback); }

    /// @brief Get runtime hours multiplied by 10 (899 = 89.9 actual)
    /// @return 
    int getHourMeter() { return HourMeter.getValue(); }
    void setHourMeterCallback(void (*callback)(int)) { HourMeter.setCallback(callback); }

    int getRelay1() { return Relay1.getValue(); }
    void setRelay1Callback(void (*callback)(int)) { Relay1.setCallback(callback); }

    int getRelay2() { return Relay2.getValue(); }
    void setRelay2Callback(void (*callback)(int)) { Relay2.setCallback(callback); }

    int getRelay3() { return Relay3.getValue(); }
    void setRelay3Callback(void (*callback)(int)) { Relay3.setCallback(callback); }

    int getRelay4() { return Relay4.getValue(); }
    void setRelay4Callback(void (*callback)(int)) { Relay4.setCallback(callback); }

    int getRelay5() { return Relay5.getValue(); }
    void setRelay5Callback(void (*callback)(int)) { Relay5.setCallback(callback); }

    int getRelay6() { return Relay6.getValue(); }
    void setRelay6Callback(void (*callback)(int)) { Relay6.setCallback(callback); }

    int getRelay7() { return Relay7.getValue(); }
    void setRelay7Callback(void (*callback)(int)) { Relay7.setCallback(callback); }

    int getRelay8() { return Relay8.getValue(); }
    void setRelay8Callback(void (*callback)(int)) { Relay8.setCallback(callback); }

    int getRelay9() { return Relay9.getValue(); }
    void setRelay9Callback(void (*callback)(int)) { Relay9.setCallback(callback); }

    int getCLMT() { return CLMT.getValue(); }
    void setCLMTCallback(void (*callback)(int)) { CLMT.setCallback(callback); }

    int getPHSE() { return PHSE.getValue(); }
    void setPHSECallback(void (*callback)(int)) { PHSE.setCallback(callback); }

    int getLLM1() { return LLM1.getValue(); }
    void setLLM1Callback(void (*callback)(int)) { LLM1.setCallback(callback); }

    int getLLM2() { return LLM2.getValue(); }
    void setLLM2Callback(void (*callback)(int)) { LLM2.setCallback(callback); }

    int getLLM3() { return LLM3.getValue(); }
    void setLLM3Callback(void (*callback)(int)) { LLM3.setCallback(callback); }

    String getSVER() { return SVER.getValue(); }
    void setSVERCallback(void (*callback)(String)) { SVER.setCallback(callback); }

    String getModel() { return Model.getValue(); }
    void setModelCallback(void (*callback)(String)) { Model.setCallback(callback); }

    String getSerialNo1() { return SerialNo1.getValue(); }
    void setSerialNo1Callback(void (*callback)(String)) { SerialNo1.setCallback(callback); }

    String getSerialNo2() { return SerialNo2.getValue(); }
    void setSerialNo2Callback(void (*callback)(String)) { SerialNo2.setCallback(callback); }

    bool getD1() { return D1.getValue(); }
    void setD1Callback(void (*callback)(bool)) { D1.setCallback(callback); }

    bool getD2() { return D2.getValue(); }
    void setD2Callback(void (*callback)(bool)) { D2.setCallback(callback); }

    bool getD3() { return D3.getValue(); }
    void setD3Callback(void (*callback)(bool)) { D3.setCallback(callback); }

    bool getD4() { return D4.getValue(); }
    void setD4Callback(void (*callback)(bool)) { D4.setCallback(callback); }

    bool getD5() { return D5.getValue(); }
    void setD5Callback(void (*callback)(bool)) { D5.setCallback(callback); }

    bool getD6() { return D6.getValue(); }
    void setD6Callback(void (*callback)(bool)) { D6.setCallback(callback); }

    String getPump() { return Pump.getValue(); }
    void setPumpCallback(void (*callback)(String)) { Pump.setCallback(callback); }

    int getLS() { return LS.getValue(); }
    void setLSCallback(void (*callback)(int)) { LS.setCallback(callback); }

    bool getHV() { return HV.getValue(); }
    void setHVCallback(void (*callback)(bool)) { HV.setCallback(callback); }

    int getSnpMR() { return SnpMR.getValue(); }
    void setSnpMRCallback(void (*callback)(int)) { SnpMR.setCallback(callback); }

    String getStatus() { return Status.getValue(); }
    void setStatusCallback(void (*callback)(String)) { Status.setCallback(callback); }

    int getPrimeCount() { return PrimeCount.getValue(); }
    void setPrimeCountCallback(void (*callback)(int)) { PrimeCount.setCallback(callback); }

    /// @brief Get EC value multiplied by 10 (66 = 6.6 actual)
    /// @return 
    int getEC() { return EC.getValue(); }
    void setECCallback(void (*callback)(int)) { EC.setCallback(callback); }

    int getHAMB() { return HAMB.getValue(); }
    void setHAMBCallback(void (*callback)(int)) { HAMB.setCallback(callback); }

    int getHCON() { return HCON.getValue(); }
    void setHCONCallback(void (*callback)(int)) { HCON.setCallback(callback); }

    String getMode() { return Mode.getValue(); }
    int getModeIndex(String mode) {
        for (size_t i = 0; i < spaModeStrings.size(); i++)
        {
            if (spaModeStrings[i] == mode)
                return i;
        }
        return -1;
    }
    void setModeCallback(void (*callback)(String)) { Mode.setCallback(callback); }
    const std::array <String, 4> spaModeStrings = {"NORM","ECON", "AWAY","WEEK"};

    int getSer1_Timer() { return Ser1_Timer.getValue(); }
    void setSer1_TimerCallback(void (*callback)(int)) { Ser1_Timer.setCallback(callback); }

    int getSer2_Timer() { return Ser2_Timer.getValue(); }
    void setSer2_TimerCallback(void (*callback)(int)) { Ser2_Timer.setCallback(callback); }

    int getSer3_Timer() { return Ser3_Timer.getValue(); }
    void setSer3_TimerCallback(void (*callback)(int)) { Ser3_Timer.setCallback(callback); }

    int getHeatMode() { return HeatMode.getValue(); }
    void setHeatModeCallback(void (*callback)(int)) { HeatMode.setCallback(callback); }

    int getPumpIdleTimer() { return PumpIdleTimer.getValue(); }
    void setPumpIdleTimerCallback(void (*callback)(int)) { PumpIdleTimer.setCallback(callback); }

    int getPumpRunTimer() { return PumpRunTimer.getValue(); }
    void setPumpRunTimerCallback(void (*callback)(int)) { PumpRunTimer.setCallback(callback); }

    /// @brief Get pool hysteris value multiplied by 10 (66 = 6.6 actual)
    /// @return 
    int getAdtPoolHys() { return AdtPoolHys.getValue(); }
    void setAdtPoolHysCallback(void (*callback)(int)) { AdtPoolHys.setCallback(callback); }

    /// @brief Get heater hysteris value multiplied by 10 (66 = 6.6 actual)
    /// @return 
    int getAdtHeaterHys() { return AdtHeaterHys.getValue(); }
    void setAdtHeaterHysCallback(void (*callback)(int)) { AdtHeaterHys.setCallback(callback); }

    /// @brief Get current power consumption (W) multiplied by 10 (24350 = 2435.0)
    /// @return 
    int getPower() { return Power.getValue(); }
    void setPowerCallback(void (*callback)(int)) { Power.setCallback(callback); }

    /// @brief Get energy consumption since last reset (kWh) multiplied by 100 (24350 = 243.50)
    /// @return 
    int getPower_kWh() { return Power_kWh.getValue(); }
    void setPower_kWhCallback(void (*callback)(int)) { Power_kWh.setCallback(callback); }

    /// @brief Get energy consumption today (kWh) multiplied by 100 (24350 = 243.50)
    /// @return 
    int getPower_Today() { return Power_Today.getValue(); }
    void setPower_TodayCallback(void (*callback)(int)) { Power_Today.setCallback(callback); }

    /// @brief Get energy consumption yesterday (kWh) multiplied by 100 (24350 = 243.50)
    /// @return 
    int getPower_Yesterday() { return Power_Yesterday.getValue(); }
    void setPower_YesterdayCallback(void (*callback)(int)) { Power_Yesterday.setCallback(callback); }

    int getThermalCutOut() { return ThermalCutOut.getValue(); }
    void setThermalCutOutCallback(void (*callback)(int)) { ThermalCutOut.setCallback(callback); }

    int getTest_D1() { return Test_D1.getValue(); }
    void setTest_D1Callback(void (*callback)(int)) { Test_D1.setCallback(callback); }

    int getTest_D2() { return Test_D2.getValue(); }
    void setTest_D2Callback(void (*callback)(int)) { Test_D2.setCallback(callback); }

    int getTest_D3() { return Test_D3.getValue(); }
    void setTest_D3Callback(void (*callback)(int)) { Test_D3.setCallback(callback); }

    /// @brief Get Heat Element Source Offset multiplied by 10 (543 = 54.3 actual)
    /// @return 
    int getElementHeatSourceOffset() { return ElementHeatSourceOffset.getValue(); }
    void setElementHeatSourceOffsetCallback(void (*callback)(int)) { ElementHeatSourceOffset.setCallback(callback); }

    int getFrequency() { return Frequency.getValue(); }
    void setFrequencyCallback(void (*callback)(int)) { Frequency.setCallback(callback); }

    /// @brief Get Heat Pump Heating Source Offset multiplied by 10 (543 = 54.3 actual)
    /// @return 
    int getHPHeatSourceOffset_Heat() { return HPHeatSourceOffset_Heat.getValue(); }
    void setHPHeatSourceOffset_HeatCallback(void (*callback)(int)) { HPHeatSourceOffset_Heat.setCallback(callback); }

    /// @brief Get Heat Pump Cooling Source Offset multiplied by 10 (543 = 54.3 actual)
    /// @return 
    int getHPHeatSourceOffset_Cool() { return HPHeatSourceOffset_Cool.getValue(); }
    void setHPHeatSourceOffset_CoolCallback(void (*callback)(int)) { HPHeatSourceOffset_Cool.setCallback(callback); }

    int getHeatSourceOffTime() { return HeatSourceOffTime.getValue(); }
    void setHeatSourceOffTimeCallback(void (*callback)(int)) { HeatSourceOffTime.setCallback(callback); }

    int getVari_Speed() { return Vari_Speed.getValue(); }
    void setVari_SpeedCallback(void (*callback)(int)) { Vari_Speed.setCallback(callback); }

    int getVari_Percent() { return Vari_Percent.getValue(); }
    void setVari_PercentCallback(void (*callback)(int)) { Vari_Percent.setCallback(callback); }

    int getVari_Mode() { return Vari_Mode.getValue(); }
    void setVari_ModeCallback(void (*callback)(int)) { Vari_Mode.setCallback(callback); }

    int getRB_TP_Pump1() { return RB_TP_Pump1.getValue(); }
    void setRB_TP_Pump1Callback(void (*callback)(int)) { RB_TP_Pump1.setCallback(callback); }

    int getRB_TP_Pump2() { return RB_TP_Pump2.getValue(); }
    void setRB_TP_Pump2Callback(void (*callback)(int)) { RB_TP_Pump2.setCallback(callback); }

    int getRB_TP_Pump3() { return RB_TP_Pump3.getValue(); }
    void setRB_TP_Pump3Callback(void (*callback)(int)) { RB_TP_Pump3.setCallback(callback); }

    int getRB_TP_Pump4() { return RB_TP_Pump4.getValue(); }
    void setRB_TP_Pump4Callback(void (*callback)(int)) { RB_TP_Pump4.setCallback(callback); }

    int getRB_TP_Pump5() { return RB_TP_Pump5.getValue(); }
    void setRB_TP_Pump5Callback(void (*callback)(int)) { RB_TP_Pump5.setCallback(callback); }
    const std::array<String, 2> autoPumpOptions = {"Manual", "Auto"};

    int getRB_TP_Blower() { return RB_TP_Blower.getValue(); }
    void setRB_TP_BlowerCallback(void (*callback)(int)) { RB_TP_Blower.setCallback(callback); }
    const std::array <String, 2> blowerStrings = {"Variable", "Ramp"};

    int getRB_TP_Light() { return RB_TP_Light.getValue(); }
    void setRB_TP_LightCallback(void (*callback)(int)) { RB_TP_Light.setCallback(callback); }

    bool getRB_TP_Auto() { return RB_TP_Auto.getValue(); }
    void setRB_TP_AutoCallback(void (*callback)(bool)) { RB_TP_Auto.setCallback(callback); }

    bool getRB_TP_Heater() { return RB_TP_Heater.getValue(); }
    void setRB_TP_HeaterCallback(void (*callback)(bool)) { RB_TP_Heater.setCallback(callback); }

    bool getRB_TP_Ozone() { return RB_TP_Ozone.getValue(); }
    void setRB_TP_OzoneCallback(void (*callback)(bool)) { RB_TP_Ozone.setCallback(callback); }

    bool getRB_TP_Sleep() { return RB_TP_Sleep.getValue(); }
    void setRB_TP_SleepCallback(void (*callback)(bool)) { RB_TP_Sleep.setCallback(callback); }

    /// @brief Get current water temperature divide by 10 to get actual temp (384 = 38.4 )
    /// @return 
    int getWTMP() { return WTMP.getValue(); }
    void setWTMPCallback(void (*callback)(int)) { WTMP.setCallback(callback); }

    bool getCleanCycle() { return CleanCycle.getValue(); }
    void setCleanCycleCallback(void (*callback)(bool)) { CleanCycle.setCallback(callback); }

    int getVARIValue() { return VARIValue.getValue(); }
    void setVARIValueCallback(void (*callback)(int)) { VARIValue.setCallback(callback); }

    int getLBRTValue() { return LBRTValue.getValue(); }
    void setLBRTValueCallback(void (*callback)(int)) { LBRTValue.setCallback(callback); }

    int getCurrClr() { return CurrClr.getValue(); }
    void setCurrClrCallback(void (*callback)(int)) { CurrClr.setCallback(callback); }
    const std::array <int, 25> colorMap = {0, 4, 4, 19, 13, 25, 25, 16, 10, 7, 2, 8, 5, 3, 6, 6, 21, 21, 21, 18, 18, 9, 9, 1, 1};

    int getLSPDValue() { return LSPDValue.getValue(); }
    void setLSPDValueCallback(void (*callback)(int)) { LSPDValue.setCallback(callback); }
    const std::array <String, 5> lightSpeedMap = {"1","2","3","4","5"};

    int getFiltHrs() { return FiltSetHrs.getValue(); }
    void setFiltHrsCallback(void (*callback)(int)) { FiltSetHrs.setCallback(callback); }
    // This is used to generated the select 
    

    int getFiltBlockHrs() { return FiltBlockHrs.getValue(); }
    void setFiltBlockHrsCallback(void (*callback)(int)) { FiltBlockHrs.setCallback(callback); }
    // According to the docs only certain values are valid here.  Using a select to ensure only valid values are used.
    const std::array <String, 8> FiltBlockHrsSelect = {"24","12","8","6","4","3","2","1"};

    int getL_24HOURS() { return L_24HOURS.getValue(); }
    void setL_24HOURSCallback(void (*callback)(int)) { L_24HOURS.setCallback(callback); }

    int getPSAV_LVL() { return PSAV_LVL.getValue(); }
    void setPSAV_LVLCallback(void (*callback)(int)) { PSAV_LVL.setCallback(callback); }

    int getPSAV_BGN() { return PSAV_BGN.getValue(); }
    void setPSAV_BGNCallback(void (*callback)(int)) { PSAV_BGN.setCallback(callback); }

    int getPSAV_END() { return PSAV_END.getValue(); }
    void setPSAV_ENDCallback(void (*callback)(int)) { PSAV_END.setCallback(callback); }

    //int getL_1SNZ_BGN() { return L_1SNZ_BGN.getValue(); }
    //void setL_1SNZ_BGNCallback(void (*callback)(int)) { L_1SNZ_BGN.setCallback(callback); }

    //int getL_2SNZ_BGN() { return L_2SNZ_BGN.getValue(); }
    //void setL_2SNZ_BGNCallback(void (*callback)(int)) { L_2SNZ_BGN.setCallback(callback); }

    //int getL_1SNZ_END() { return L_1SNZ_END.getValue(); }
    //void setL_1SNZ_ENDCallback(void (*callback)(int)) { L_1SNZ_END.setCallback(callback); }

    //int getL_2SNZ_END() { return L_2SNZ_END.getValue(); }
    //void setL_2SNZ_ENDCallback(void (*callback)(int)) { L_2SNZ_END.setCallback(callback); }

    int getDefaultScrn() { return DefaultScrn.getValue(); }
    void setDefaultScrnCallback(void (*callback)(int)) { DefaultScrn.setCallback(callback); }

    int getTOUT() { return TOUT.getValue(); }
    void setTOUTCallback(void (*callback)(int)) { TOUT.setCallback(callback); }

    bool getVPMP() { return VPMP.getValue(); }
    void setVPMPCallback(void (*callback)(bool)) { VPMP.setCallback(callback); }

    bool getHIFI() { return HIFI.getValue(); }
    void setHIFICallback(void (*callback)(bool)) { HIFI.setCallback(callback); }

    int getBRND() { return BRND.getValue(); }
    void setBRNDCallback(void (*callback)(int)) { BRND.setCallback(callback); }

    int getPRME() { return PRME.getValue(); }
    void setPRMECallback(void (*callback)(int)) { PRME.setCallback(callback); }

    int getELMT() { return ELMT.getValue(); }
    void setELMTCallback(void (*callback)(int)) { ELMT.setCallback(callback); }

    int getTYPE() { return TYPE.getValue(); }
    void setTYPECallback(void (*callback)(int)) { TYPE.setCallback(callback); }

    int getGAS() { return GAS.getValue(); }
    void setGASCallback(void (*callback)(int)) { GAS.setCallback(callback); }

    int getWCLNTime() { return WCLNTime.getValue(); }
    void setWCLNTimeCallback(void (*callback)(int)) { WCLNTime.setCallback(callback); }

    bool getTemperatureUnits() { return TemperatureUnits.getValue(); }
    void setTemperatureUnitsCallback(void (*callback)(bool)) { TemperatureUnits.setCallback(callback); }

    bool getOzoneOff() { return OzoneOff.getValue(); }
    void setOzoneOffCallback(void (*callback)(bool)) { OzoneOff.setCallback(callback); }

    bool getCirc24() { return Circ24.getValue(); }
    void setCirc24Callback(void (*callback)(bool)) { Circ24.setCallback(callback); }

    bool getCJET() { return CJET.getValue(); }
    void setCJETCallback(void (*callback)(bool)) { CJET.setCallback(callback); }

    bool getVELE() { return VELE.getValue(); }
    void setVELECallback(void (*callback)(bool)) { VELE.setCallback(callback); }

    int getV_Max() { return V_Max.getValue(); }
    void setV_MaxCallback(void (*callback)(int)) { V_Max.setCallback(callback); }

    int getV_Min() { return V_Min.getValue(); }
    void setV_MinCallback(void (*callback)(int)) { V_Min.setCallback(callback); }

    int getV_Max_24() { return V_Max_24.getValue(); }
    void setV_Max_24Callback(void (*callback)(int)) { V_Max_24.setCallback(callback); }

    int getV_Min_24() { return V_Min_24.getValue(); }
    void setV_Min_24Callback(void (*callback)(int)) { V_Min_24.setCallback(callback); }

    int getCurrentZero() { return CurrentZero.getValue(); }
    void setCurrentZeroCallback(void (*callback)(int)) { CurrentZero.setCallback(callback); }

    /// @brief Get Current measurement adjustment multiplied by 10 (77 = 7.7 actual)
    /// @return 
    int getCurrentAdjust() { return CurrentAdjust.getValue(); }
    void setCurrentAdjustCallback(void (*callback)(int)) { CurrentAdjust.setCallback(callback); }

    /// @brief Get Voltage measurement adjustment multiplied by 10 (77 = 7.7 actual)
    /// @return 
    int getVoltageAdjust() { return VoltageAdjust.getValue(); }
    void setVoltageAdjustCallback(void (*callback)(int)) { VoltageAdjust.setCallback(callback); }

    int getSer1() { return Ser1.getValue(); }
    void setSer1Callback(void (*callback)(int)) { Ser1.setCallback(callback); }

    int getSer2() { return Ser2.getValue(); }
    void setSer2Callback(void (*callback)(int)) { Ser2.setCallback(callback); }

    int getSer3() { return Ser3.getValue(); }
    void setSer3Callback(void (*callback)(int)) { Ser3.setCallback(callback); }

    int getVMAX() { return VMAX.getValue(); }
    void setVMAXCallback(void (*callback)(int)) { VMAX.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getAHYS() { return AHYS.getValue(); }
    void setAHYSCallback(void (*callback)(int)) { AHYS.setCallback(callback); }   

    bool getHUSE() { return HUSE.getValue(); }
    void setHUSECallback(void (*callback)(bool)) { HUSE.setCallback(callback); }

    bool getHELE() { return HELE.getValue(); }
    void setHELECallback(void (*callback)(bool)) { HELE.setCallback(callback); }

    int getPMIN() { return PMIN.getValue(); }
    void setPMINCallback(void (*callback)(int)) { PMIN.setCallback(callback); }

    int getPFLT() { return PFLT.getValue(); }
    void setPFLTCallback(void (*callback)(int)) { PFLT.setCallback(callback); }

    int getPHTR() { return PHTR.getValue(); }
    void setPHTRCallback(void (*callback)(int)) { PHTR.setCallback(callback); }

    int getPMAX() { return PMAX.getValue(); }
    void setPMAXCallback(void (*callback)(int)) { PMAX.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF1_HR() { return F1_HR.getValue(); }
    void setF1_HRCallback(void (*callback)(int)) { F1_HR.setCallback(callback); }  

    int getF1_Time() { return F1_Time.getValue(); }
    void setF1_TimeCallback(void (*callback)(int)) { F1_Time.setCallback(callback); }

    int getF1_ER() { return F1_ER.getValue(); }
    void setF1_ERCallback(void (*callback)(int)) { F1_ER.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF1_I() { return F1_I.getValue(); }
    void setF1_ICallback(void (*callback)(int)) { F1_I.setCallback(callback); }

    int getF1_V() { return F1_V.getValue(); }
    void setF1_VCallback(void (*callback)(int)) { F1_V.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF1_PT() { return F1_PT.getValue(); }
    void setF1_PTCallback(void (*callback)(int)) { F1_PT.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF1_HT() { return F1_HT.getValue(); }
    void setF1_HTCallback(void (*callback)(int)) { F1_HT.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF1_CT() { return F1_CT.getValue(); }
    void setF1_CTCallback(void (*callback)(int)) { F1_CT.setCallback(callback); }

    int getF1_PU() { return F1_PU.getValue(); }
    void setF1_PUCallback(void (*callback)(int)) { F1_PU.setCallback(callback); }

    bool getF1_VE() { return F1_VE.getValue(); }
    void setF1_VECallback(void (*callback)(bool)) { F1_VE.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF1_ST() { return F1_ST.getValue(); }
    void setF1_STCallback(void (*callback)(int)) { F1_ST.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF2_HR() { return F2_HR.getValue(); }
    void setF2_HRCallback(void (*callback)(int)) { F2_HR.setCallback(callback); }  

    int getF2_Time() { return F2_Time.getValue(); }
    void setF2_TimeCallback(void (*callback)(int)) { F2_Time.setCallback(callback); }

    int getF2_ER() { return F2_ER.getValue(); }
    void setF2_ERCallback(void (*callback)(int)) { F2_ER.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF2_I() { return F2_I.getValue(); }
    void setF2_ICallback(void (*callback)(int)) { F2_I.setCallback(callback); }

    int getF2_V() { return F2_V.getValue(); }
    void setF2_VCallback(void (*callback)(int)) { F2_V.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF2_PT() { return F2_PT.getValue(); }
    void setF2_PTCallback(void (*callback)(int)) { F2_PT.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF2_HT() { return F2_HT.getValue(); }
    void setF2_HTCallback(void (*callback)(int)) { F2_HT.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF2_CT() { return F2_CT.getValue(); }
    void setF2_CTCallback(void (*callback)(int)) { F2_CT.setCallback(callback); }

    int getF2_PU() { return F2_PU.getValue(); }
    void setF2_PUCallback(void (*callback)(int)) { F2_PU.setCallback(callback); }

    bool getF2_VE() { return F2_VE.getValue(); }
    void setF2_VECallback(void (*callback)(bool)) { F2_VE.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF2_ST() { return F2_ST.getValue(); }
    void setF2_STCallback(void (*callback)(int)) { F2_ST.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF3_HR() { return F3_HR.getValue(); }
    void setF3_HRCallback(void (*callback)(int)) { F3_HR.setCallback(callback); }  

    int getF3_Time() { return F3_Time.getValue(); }
    void setF3_TimeCallback(void (*callback)(int)) { F3_Time.setCallback(callback); }

    int getF3_ER() { return F3_ER.getValue(); }
    void setF3_ERCallback(void (*callback)(int)) { F3_ER.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF3_I() { return F3_I.getValue(); }
    void setF3_ICallback(void (*callback)(int)) { F3_I.setCallback(callback); }

    int getF3_V() { return F3_V.getValue(); }
    void setF3_VCallback(void (*callback)(int)) { F3_V.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF3_PT() { return F3_PT.getValue(); }
    void setF3_PTCallback(void (*callback)(int)) { F3_PT.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF3_HT() { return F3_HT.getValue(); }
    void setF3_HTCallback(void (*callback)(int)) { F3_HT.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    int getF3_CT() { return F3_CT.getValue(); }
    void setF3_CTCallback(void (*callback)(int)) { F3_CT.setCallback(callback); }

    int getF3_PU() { return F3_PU.getValue(); }
    void setF3_PUCallback(void (*callback)(int)) { F3_PU.setCallback(callback); }

    /// @brief Actual value multiplied by 10 (5 = 0.5)
    /// @return 
    bool getF3_VE() { return F3_VE.getValue(); }
    void setF3_VECallback(void (*callback)(bool)) { F3_VE.setCallback(callback); }

    int getF3_ST() { return F3_ST.getValue(); }
    void setF3_STCallback(void (*callback)(int)) { F3_ST.setCallback(callback); }

    int getOutlet_Blower() { return Outlet_Blower.getValue(); }
    void setOutlet_BlowerCallback(void (*callback)(int)) { Outlet_Blower.setCallback(callback); }

    int getHP_Present() { return HP_Present.getValue(); }
    void setHP_PresentCallback(void (*callback)(int)) { HP_Present.setCallback(callback); }

    int getHP_Ambient() { return HP_Ambient.getValue(); }
    void setHP_AmbientCallback(void (*callback)(int)) { HP_Ambient.setCallback(callback); }

    int getHP_Condensor() { return HP_Condensor.getValue(); }
    void setHP_CondensorCallback(void (*callback)(int)) { HP_Condensor.setCallback(callback); }

    bool getHP_Compressor_State() { return HP_Compressor_State.getValue(); }
    void setHP_Compressor_StateCallback(void (*callback)(bool)) { HP_Compressor_State.setCallback(callback); }

    bool getHP_Fan_State() { return HP_Fan_State.getValue(); }
    void setHP_Fan_StateCallback(void (*callback)(bool)) { HP_Fan_State.setCallback(callback); }

    bool getHP_4W_Valve() { return HP_4W_Valve.getValue(); }
    void setHP_4W_ValveCallback(void (*callback)(bool)) { HP_4W_Valve.setCallback(callback); }

    bool getHP_Heater_State() { return HP_Heater_State.getValue(); }
    void setHP_Heater_StateCallback(void (*callback)(bool)) { HP_Heater_State.setCallback(callback); }

    int getHP_Mode() { return HP_Mode.getValue(); }
    void setHP_ModeCallback(void (*callback)(int)) { HP_Mode.setCallback(callback); }

    int getHP_Defrost_Timer() { return HP_Defrost_Timer.getValue(); }
    void setHP_Defrost_TimerCallback(void (*callback)(int)) { HP_Defrost_Timer.setCallback(callback); }

    int getHP_Comp_Run_Timer() { return HP_Comp_Run_Timer.getValue(); }
    void setHP_Comp_Run_TimerCallback(void (*callback)(int)) { HP_Comp_Run_Timer.setCallback(callback); }

    int getHP_Low_Temp_Timer() { return HP_Low_Temp_Timer.getValue(); }
    void setHP_Low_Temp_TimerCallback(void (*callback)(int)) { HP_Low_Temp_Timer.setCallback(callback); }

    int getHP_Heat_Accum_Timer() { return HP_Heat_Accum_Timer.getValue(); }
    void setHP_Heat_Accum_TimerCallback(void (*callback)(int)) { HP_Heat_Accum_Timer.setCallback(callback); }

    int getHP_Warning() { return HP_Warning.getValue(); }
    void setHP_WarningCallback(void (*callback)(int)) { HP_Warning.setCallback(callback); }

    int getHP_FrezTmr() { return FrezTmr.getValue(); }
    void setHP_FrezTmrCallback(void (*callback)(int)) { FrezTmr.setCallback(callback); }

    int getDBGN() { return DBGN.getValue(); }
    void setDBGNCallback(void (*callback)(int)) { DBGN.setCallback(callback); }

    int getDEND() { return DEND.getValue(); }
    void setDENDCallback(void (*callback)(int)) { DEND.setCallback(callback); }

    int getDCMP() { return DCMP.getValue(); }
    void setDCMPCallback(void (*callback)(int)) { DCMP.setCallback(callback); }

    int getDMAX() { return DMAX.getValue(); }
    void setDMAXCallback(void (*callback)(int)) { DMAX.setCallback(callback); }

    int getDELE() { return DELE.getValue(); }
    void setDELECallback(void (*callback)(int)) { DELE.setCallback(callback); }

    int getDPMP() { return DPMP.getValue(); }
    void setDPMPCallback(void (*callback)(int)) { DPMP.setCallback(callback); }

    String getPump1InstallState() { return Pump1InstallState.getValue(); }
    void setPump1InstallStateCallback(void (*callback)(String)) { Pump1InstallState.setCallback(callback); }

    String getPump2InstallState() { return Pump2InstallState.getValue(); }
    void setPump2InstallStateCallback(void (*callback)(String)) { Pump2InstallState.setCallback(callback); }

    String getPump3InstallState() { return Pump3InstallState.getValue(); }
    void setPump3InstallStateCallback(void (*callback)(String)) { Pump3InstallState.setCallback(callback); }

    String getPump4InstallState() { return Pump4InstallState.getValue(); }
    void setPump4InstallStateCallback(void (*callback)(String)) { Pump4InstallState.setCallback(callback); }

    String getPump5InstallState() { return Pump5InstallState.getValue(); }
    void setPump5InstallStateCallback(void (*callback)(String)) { Pump5InstallState.setCallback(callback); }

    bool getPump1OkToRun() { return Pump1OkToRun.getValue(); }
    void setPump1OkToRunCallback(void (*callback)(bool)) { Pump1OkToRun.setCallback(callback); }

    bool getPump2OkToRun() { return Pump2OkToRun.getValue(); }
    void setPump2OkToRunCallback(void (*callback)(bool)) { Pump2OkToRun.setCallback(callback); }

    bool getPump3OkToRun() { return Pump3OkToRun.getValue(); }
    void setPump3OkToRunCallback(void (*callback)(bool)) { Pump3OkToRun.setCallback(callback); }

    bool getPump4OkToRun() { return Pump4OkToRun.getValue(); }
    void setPump4OkToRunCallback(void (*callback)(bool)) { Pump4OkToRun.setCallback(callback); }

    bool getPump5OkToRun() { return Pump5OkToRun.getValue(); }
    void setPump5OkToRunCallback(void (*callback)(bool)) { Pump5OkToRun.setCallback(callback); }

    int getLockMode() { return LockMode.getValue(); }
    void setLockModeCallback(void (*callback)(int)) { LockMode.setCallback(callback); }
    const std::array <String, 3> lockModeMap = {"Unlocked", "Partially Locked", "Locked"};
};

#endif

