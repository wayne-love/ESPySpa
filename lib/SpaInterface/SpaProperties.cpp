#include "SpaProperties.h"

template<typename T>
std::function<bool(const char*, T)> Property<T>::_sendCallback = nullptr;
template<>
std::function<bool(const char*, String)> Property<String>::_sendCallback = nullptr;
template<>
std::function<bool(const char*, long)> Property<long>::_sendCallback = nullptr;

std::function<bool(const char*, int)> Property<int>::_sendCallback = nullptr;
std::function<bool(const char*, String)> Property<int>::_sendStringCallback = nullptr;
std::function<bool(const char*, bool)> Property<bool>::_sendCallback = nullptr;

void SpaProperties::updateMeasures(String statusResponseRaw[], int R2, int R3, int R4, int R5, int R6, int R7, int R9, int RA, int RB, int RC, int RE, int RG) {
    #pragma region R2
    MainsCurrent.updateValue(statusResponseRaw[R2+1]);
    MainsVoltage.updateValue(statusResponseRaw[R2+2]);
    CaseTemperature.updateValue(statusResponseRaw[R2+3]);
    PortCurrent.updateValue(statusResponseRaw[R2+4]);
    tmElements_t tm;
    tm.Hour=statusResponseRaw[R2+6].toInt();
    tm.Minute=statusResponseRaw[R2+7].toInt();
    tm.Second=statusResponseRaw[R2+8].toInt();
    tm.Day=statusResponseRaw[R2+9].toInt();
    tm.Month=statusResponseRaw[R2+10].toInt();
    tm.Year=CalendarYrToTm(statusResponseRaw[R2+11].toInt());
    SpaTime.updateValue(makeTime(tm));
    HeaterTemperature.updateValue(statusResponseRaw[R2+12]);
    PoolTemperature.updateValue(statusResponseRaw[R2+13]);
    WaterPresent.updateValue(statusResponseRaw[R2+14]);
    AwakeMinutesRemaining.updateValue(statusResponseRaw[R2+16]);
    FiltPumpRunTimeTotal.updateValue(statusResponseRaw[R2+17]);
    FiltPumpReqMins.updateValue(statusResponseRaw[R2+18]);
    LoadTimeOut.updateValue(statusResponseRaw[R2+19]);
    HourMeter.updateValue(statusResponseRaw[R2+20]);
    Relay1.updateValue(statusResponseRaw[R2+21]);
    Relay2.updateValue(statusResponseRaw[R2+22]);
    Relay3.updateValue(statusResponseRaw[R2+23]);
    Relay4.updateValue(statusResponseRaw[R2+24]);
    Relay5.updateValue(statusResponseRaw[R2+25]);
    Relay6.updateValue(statusResponseRaw[R2+26]);
    Relay7.updateValue(statusResponseRaw[R2+27]);
    Relay8.updateValue(statusResponseRaw[R2+28]);
    Relay9.updateValue(statusResponseRaw[R2+29]);
    #pragma endregion
    #pragma region R3
    CLMT.updateValue(statusResponseRaw[R3+1]);
    PHSE.updateValue(statusResponseRaw[R3+2]);
    LLM1.updateValue(statusResponseRaw[R3+3]);
    LLM2.updateValue(statusResponseRaw[R3+4]);
    LLM3.updateValue(statusResponseRaw[R3+5]);
    SVER.updateValue(statusResponseRaw[R3+6]);
    Model.updateValue(statusResponseRaw[R3+7]);
    SerialNo1.updateValue(statusResponseRaw[R3+8]);
    SerialNo2.updateValue(statusResponseRaw[R3+9]);
    D1.updateValue(statusResponseRaw[R3+10]);
    D2.updateValue(statusResponseRaw[R3+11]);
    D3.updateValue(statusResponseRaw[R3+12]);
    D4.updateValue(statusResponseRaw[R3+13]);
    D5.updateValue(statusResponseRaw[R3+14]);
    D6.updateValue(statusResponseRaw[R3+15]);
    Pump.updateValue(statusResponseRaw[R3+16]);
    LS.updateValue(statusResponseRaw[R3+17]);
    HV.updateValue(statusResponseRaw[R3+18]);
    SnpMR.updateValue(statusResponseRaw[R3+19]);
    Status.updateValue(statusResponseRaw[R3+20]);
    PrimeCount.updateValue(statusResponseRaw[R3+21]);
    EC.updateValue(statusResponseRaw[R3+22]);
    HAMB.updateValue(statusResponseRaw[R3+23]);
    HCON.updateValue(statusResponseRaw[R3+24]);
    // HV_2.updateValue(statusResponseRaw[R3+25]);
    #pragma endregion
    #pragma region R4
    Mode.updateValue(statusResponseRaw[R4+1]);
    Ser1_Timer.updateValue(statusResponseRaw[R4+2]);
    Ser2_Timer.updateValue(statusResponseRaw[R4+3]);
    Ser3_Timer.updateValue(statusResponseRaw[R4+4]);
    HeatMode.updateValue(statusResponseRaw[R4+5]);
    PumpIdleTimer.updateValue(statusResponseRaw[R4+6]);
    PumpRunTimer.updateValue(statusResponseRaw[R4+7]);
    AdtPoolHys.updateValue(statusResponseRaw[R4+8]);
    AdtHeaterHys.updateValue(statusResponseRaw[R4+9]);
    Power.updateValue(statusResponseRaw[R4+10]);
    Power_kWh.updateValue(statusResponseRaw[R4+11]);
    Power_Today.updateValue(statusResponseRaw[R4+12]);
    Power_Yesterday.updateValue(statusResponseRaw[R4+13]);
    ThermalCutOut.updateValue(statusResponseRaw[R4+14]);
    Test_D1.updateValue(statusResponseRaw[R4+15]);
    Test_D2.updateValue(statusResponseRaw[R4+16]);
    Test_D3.updateValue(statusResponseRaw[R4+17]);
    ElementHeatSourceOffset.updateValue(statusResponseRaw[R4+18]);
    Frequency.updateValue(statusResponseRaw[R4+19]);
    HPHeatSourceOffset_Heat.updateValue(statusResponseRaw[R4+20]);
    HPHeatSourceOffset_Cool.updateValue(statusResponseRaw[R4+21]);
    HeatSourceOffTime.updateValue(statusResponseRaw[R4+22]);
    Vari_Speed.updateValue(statusResponseRaw[R4+24]);
    Vari_Percent.updateValue(statusResponseRaw[R4+25]);
    Vari_Mode.updateValue(statusResponseRaw[R4+23]);
    #pragma endregion
    #pragma region R5
    //R5
    // Unknown encoding - TouchPad2.updateValue();
    // Unknown encoding - TouchPad1.updateValue();
    //RB_TP_Blower.updateValue(statusResponseRaw[R5 + 5]);
    RB_TP_Sleep.updateValue(statusResponseRaw[R5 + 10]);
    RB_TP_Ozone.updateValue(statusResponseRaw[R5 + 11]);
    RB_TP_Heater.updateValue(statusResponseRaw[R5 + 12]);
    RB_TP_Auto.updateValue(statusResponseRaw[R5 + 13]);
    RB_TP_Light.updateValue(statusResponseRaw[R5 + 14]);
    WTMP.updateValue(statusResponseRaw[R5 + 15]);
    CleanCycle.updateValue(statusResponseRaw[R5 + 16]);
    RB_TP_Pump1.updateValue(statusResponseRaw[R5 + 18]);
    RB_TP_Pump2.updateValue(statusResponseRaw[R5 + 19]);
    RB_TP_Pump3.updateValue(statusResponseRaw[R5 + 20]);
    RB_TP_Pump4.updateValue(statusResponseRaw[R5 + 21]);
    RB_TP_Pump5.updateValue(statusResponseRaw[R5 + 22]);
    #pragma endregion
    #pragma region R6
    VARIValue.updateValue(statusResponseRaw[R6 + 1]);
    LBRTValue.updateValue(statusResponseRaw[R6 + 2]);
    CurrClr.updateValue(statusResponseRaw[R6 + 3]);
    ColorMode.updateValue(statusResponseRaw[R6 + 4]);
    LSPDValue.updateValue(statusResponseRaw[R6 + 5]);
    FiltSetHrs.updateValue(statusResponseRaw[R6 + 6]);
    FiltBlockHrs.updateValue(statusResponseRaw[R6 + 7]);
    STMP.updateValue(statusResponseRaw[R6 + 8]);
    L_24HOURS.updateValue(statusResponseRaw[R6 + 9]);
    PSAV_LVL.updateValue(statusResponseRaw[R6 + 10]);
    PSAV_BGN.updateValue(statusResponseRaw[R6 + 11]);
    PSAV_END.updateValue(statusResponseRaw[R6 + 12]);
    L_1SNZ_DAY.updateValue(statusResponseRaw[R6 + 13]);
    L_2SNZ_DAY.updateValue(statusResponseRaw[R6 + 14]);
    L_1SNZ_BGN.updateValue(statusResponseRaw[R6 + 15]);
    L_2SNZ_BGN.updateValue(statusResponseRaw[R6 + 16]);
    L_1SNZ_END.updateValue(statusResponseRaw[R6 + 17]);
    L_2SNZ_END.updateValue(statusResponseRaw[R6 + 18]);
    DefaultScrn.updateValue(statusResponseRaw[R6 + 19]);
    TOUT.updateValue(statusResponseRaw[R6 + 20]);
    VPMP.updateValue(statusResponseRaw[R6 + 21]);
    HIFI.updateValue(statusResponseRaw[R6 + 22]);
    BRND.updateValue(statusResponseRaw[R6 + 23]);
    PRME.updateValue(statusResponseRaw[R6 + 24]);
    ELMT.updateValue(statusResponseRaw[R6 + 25]);
    TYPE.updateValue(statusResponseRaw[R6 + 26]);
    GAS.updateValue(statusResponseRaw[R6 + 27]);
    #pragma endregion
    #pragma region R7
    WCLNTime.updateValue(statusResponseRaw[R7 + 1]);
    // The following 2 may be reversed
    TemperatureUnits.updateValue(statusResponseRaw[R7 + 3]);
    OzoneOff.updateValue(statusResponseRaw[R7 + 2]);
    Ozone24.updateValue(statusResponseRaw[R7 + 4]);
    Circ24.updateValue(statusResponseRaw[R7 + 6]);
    CJET.updateValue(statusResponseRaw[R7 + 5]);
    // 0 = off, 1 = step, 2 = variable
    VELE.updateValue(statusResponseRaw[R7 + 7]);
    //StartDD.updateValue(statusResponseRaw[R7 + 8]);
    //StartMM.updateValue(statusResponseRaw[R7 + 9]);
    //StartYY.updateValue(statusResponseRaw[R7 + 10]);
    V_Max.updateValue(statusResponseRaw[R7 + 11]);
    V_Min.updateValue(statusResponseRaw[R7 + 12]);
    V_Max_24.updateValue(statusResponseRaw[R7 + 13]);
    V_Min_24.updateValue(statusResponseRaw[R7 + 14]);
    CurrentZero.updateValue(statusResponseRaw[R7 + 15]);
    CurrentAdjust.updateValue(statusResponseRaw[R7 + 16]);
    VoltageAdjust.updateValue(statusResponseRaw[R7 + 17]);
    // 168 is unknown
    Ser1.updateValue(statusResponseRaw[R7 + 19]);
    Ser2.updateValue(statusResponseRaw[R7 + 20]);
    Ser3.updateValue(statusResponseRaw[R7 + 21]);
    VMAX.updateValue(statusResponseRaw[R7 + 22]);
    AHYS.updateValue(statusResponseRaw[R7 + 23]);
    HUSE.updateValue(statusResponseRaw[R7 + 24]);
    HELE.updateValue(statusResponseRaw[R7 + 25]);
    HPMP.updateValue(statusResponseRaw[R7 + 26]);
    PMIN.updateValue(statusResponseRaw[R7 + 27]);
    PFLT.updateValue(statusResponseRaw[R7 + 28]);
    PHTR.updateValue(statusResponseRaw[R7 + 29]);
    PMAX.updateValue(statusResponseRaw[R7 + 30]);
    #pragma endregion
    #pragma region R9
    F1_HR.updateValue(statusResponseRaw[R9 + 2]);
    F1_Time.updateValue(statusResponseRaw[R9 + 3]);
    F1_ER.updateValue(statusResponseRaw[R9 + 4]);
    F1_I.updateValue(statusResponseRaw[R9 + 5]);
    F1_V.updateValue(statusResponseRaw[R9 + 6]);
    F1_PT.updateValue(statusResponseRaw[R9 + 7]);
    F1_HT.updateValue(statusResponseRaw[R9 + 8]);
    F1_CT.updateValue(statusResponseRaw[R9 + 9]);
    F1_PU.updateValue(statusResponseRaw[R9 + 10]);
    F1_VE.updateValue(statusResponseRaw[R9 + 11]);
    F1_ST.updateValue(statusResponseRaw[R9 + 12]);
    #pragma endregion
    #pragma region RA
    F2_HR.updateValue(statusResponseRaw[RA + 2]);
    F2_Time.updateValue(statusResponseRaw[RA + 3]);
    F2_ER.updateValue(statusResponseRaw[RA + 4]);
    F2_I.updateValue(statusResponseRaw[RA + 5]);
    F2_V.updateValue(statusResponseRaw[RA + 6]);
    F2_PT.updateValue(statusResponseRaw[RA + 7]);
    F2_HT.updateValue(statusResponseRaw[RA + 8]);
    F2_CT.updateValue(statusResponseRaw[RA + 9]);
    F2_PU.updateValue(statusResponseRaw[RA + 10]);
    F2_VE.updateValue(statusResponseRaw[RA + 11]);
    F2_ST.updateValue(statusResponseRaw[RA + 12]);
    #pragma endregion
    #pragma region RB
    F3_HR.updateValue(statusResponseRaw[RB + 2]);
    F3_Time.updateValue(statusResponseRaw[RB + 3]);
    F3_ER.updateValue(statusResponseRaw[RB + 4]);
    F3_I.updateValue(statusResponseRaw[RB + 5]);
    F3_V.updateValue(statusResponseRaw[RB + 6]);
    F3_PT.updateValue(statusResponseRaw[RB + 7]);
    F3_HT.updateValue(statusResponseRaw[RB + 8]);
    F3_CT.updateValue(statusResponseRaw[RB + 9]);
    F3_PU.updateValue(statusResponseRaw[RB + 10]);
    F3_VE.updateValue(statusResponseRaw[RB + 11]);
    F3_ST.updateValue(statusResponseRaw[RB + 12]);
    #pragma endregion
    #pragma region RC
    //Outlet_Heater.updateValue(statusResponseRaw[]);
    //Outlet_Circ.updateValue(statusResponseRaw[]);
    //Outlet_Sanitise.updateValue(statusResponseRaw[]);
    //Outlet_Pump1.updateValue(statusResponseRaw[]);
    //Outlet_Pump2.updateValue(statusResponseRaw[]);
    //Outlet_Pump4.updateValue(statusResponseRaw[]);
    //Outlet_Pump5.updateValue(statusResponseRaw[]);
    Outlet_Blower.updateValue(statusResponseRaw[RC + 10]);
    #pragma endregion
    #pragma region RE
    HP_Present.updateValue(statusResponseRaw[RE + 1]);
    //HP_FlowSwitch.updateValue(statusResponseRaw[]);
    //HP_HighSwitch.updateValue(statusResponseRaw[]);
    //HP_LowSwitch.updateValue(statusResponseRaw[]);
    //HP_CompCutOut.updateValue(statusResponseRaw[]);
    //HP_ExCutOut.updateValue(statusResponseRaw[]);
    //HP_D1.updateValue(statusResponseRaw[]);
    //HP_D2.updateValue(statusResponseRaw[]);
    //HP_D3.updateValue(statusResponseRaw[]);
    HP_Ambient.updateValue(statusResponseRaw[RE + 10]);
    HP_Condensor.updateValue(statusResponseRaw[RE + 11]);
    HP_Compressor_State.updateValue(statusResponseRaw[RE + 12]);
    HP_Fan_State.updateValue(statusResponseRaw[RE + 13]);
    HP_4W_Valve.updateValue(statusResponseRaw[RE + 14]);
    HP_Heater_State.updateValue(statusResponseRaw[RE + 15]);
    HP_State.updateValue(statusResponseRaw[RE + 16]);
    HP_Mode.updateValue(statusResponseRaw[RE + 17]);
    HP_Defrost_Timer.updateValue(statusResponseRaw[RE + 18]);
    HP_Comp_Run_Timer.updateValue(statusResponseRaw[RE + 19]);
    HP_Low_Temp_Timer.updateValue(statusResponseRaw[RE + 20]);
    HP_Heat_Accum_Timer.updateValue(statusResponseRaw[RE + 21]);
    HP_Sequence_Timer.updateValue(statusResponseRaw[RE + 22]);
    HP_Warning.updateValue(statusResponseRaw[RE + 23]);
    FrezTmr.updateValue(statusResponseRaw[RE + 24]);
    DBGN.updateValue(statusResponseRaw[RE + 25]);
    DEND.updateValue(statusResponseRaw[RE + 26]);
    DCMP.updateValue(statusResponseRaw[RE + 27]);
    DMAX.updateValue(statusResponseRaw[RE + 28]);
    DELE.updateValue(statusResponseRaw[RE + 29]);
    DPMP.updateValue(statusResponseRaw[RE + 30]);
    //CMAX.updateValue(statusResponseRaw[]);
    //HP_Compressor.updateValue(statusResponseRaw[]);
    //HP_Pump_State.updateValue(statusResponseRaw[]);
    //HP_Status.updateValue(statusResponseRaw[]);
    #pragma endregion
    #pragma region RG
    Pump1InstallState.updateValue(statusResponseRaw[RG + 7]);
    Pump2InstallState.updateValue(statusResponseRaw[RG + 8]);
    Pump3InstallState.updateValue(statusResponseRaw[RG + 9]);
    Pump4InstallState.updateValue(statusResponseRaw[RG + 10]);
    Pump5InstallState.updateValue(statusResponseRaw[RG + 11]);
    Pump1OkToRun.updateValue(statusResponseRaw[RG + 1]);
    Pump2OkToRun.updateValue(statusResponseRaw[RG + 2]);
    Pump3OkToRun.updateValue(statusResponseRaw[RG + 3]);
    Pump4OkToRun.updateValue(statusResponseRaw[RG + 4]);
    Pump5OkToRun.updateValue(statusResponseRaw[RG + 5]);
    LockMode.updateValue(statusResponseRaw[RG + 12]);
    #pragma endregion

}
