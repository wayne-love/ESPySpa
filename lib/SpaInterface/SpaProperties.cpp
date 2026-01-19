#include "SpaProperties.h"


inline boolean isNumber(String s) {
    if (s.isEmpty()) {
        return false;
    }
    // Accepts optional leading '-', digits, and a single decimal point.
    u_int start = 0;
    if (s[0] == '-') {
        if (s.length() == 1) {
            return false;
        }
        start = 1;
    }
    bool saw_digit = false;
    bool saw_dot = false;
    for (u_int i = start; i < s.length(); i++) {
        if (isDigit(s[i])) {
            saw_digit = true;
            continue;
        }
        if (s[i] == '.' && !saw_dot) {
            saw_dot = true;
            continue;
        }
        return false;
    }
    return saw_digit;
}

static boolean updateIntProperty(Property<int>& prop, const String& s) {
    if (!isNumber(s)) {
        return false;
    }

    prop.update_Value(s.toInt());
    return true;
}

static boolean updateBool01Property(Property<bool>& prop, const String& s) {
    if (s!="0" && s!="1") {
        return false;
    }

    prop.update_Value(s == "1");
    return true;
}

static boolean updateStringProperty(Property<String>& prop, const String& s) {
    prop.update_Value(s);
    return true;
}

static boolean updateTriStateProperty(Property<int>& prop, const String& s) {
    if (s!="0" && s!="1" && s!="2") {
        return false;
    }

    prop.update_Value(s.toInt());
    return true;
}

boolean SpaProperties::update_MainsCurrent(String s){
    return updateIntProperty(MainsCurrent, s);
}

boolean SpaProperties::update_SpaDayOfWeek(String s){
    return updateIntProperty(SpaDayOfWeek, s);
}

boolean SpaProperties::update_SpaTime(String year, String month, String day, String hour, String minute, String second){

    tmElements_t tm;
    tm.Year=CalendarYrToTm(year.toInt());
    tm.Month=month.toInt();
    tm.Day=day.toInt();
    tm.Hour=hour.toInt();
    tm.Minute=minute.toInt();
    tm.Second=second.toInt();

    SpaTime.update_Value(makeTime(tm));

    return true;
}

boolean SpaProperties::update_MainsVoltage(String s){
    return updateIntProperty(MainsVoltage, s);
}

boolean SpaProperties::update_CaseTemperature(String s){
    return updateIntProperty(CaseTemperature, s);
}

boolean SpaProperties::update_PortCurrent(String s){
    return updateIntProperty(PortCurrent, s);
}

boolean SpaProperties::update_HeaterTemperature(String s){
    return updateIntProperty(HeaterTemperature, s);
}

boolean SpaProperties::update_PoolTemperature(String s){
    return updateIntProperty(PoolTemperature, s);
}

boolean SpaProperties::update_WaterPresent(String s) {
    return updateBool01Property(WaterPresent, s);
}

boolean SpaProperties::update_AwakeMinutesRemaining(String s){
    return updateIntProperty(AwakeMinutesRemaining, s);
}

boolean SpaProperties::update_FiltPumpRunTimeTotal(String s){
    return updateIntProperty(FiltPumpRunTimeTotal, s);
}

boolean SpaProperties::update_FiltPumpReqMins(String s){
    return updateIntProperty(FiltPumpReqMins, s);
}

boolean SpaProperties::update_LoadTimeOut(String s){
    return updateIntProperty(LoadTimeOut, s);
}

boolean SpaProperties::update_HourMeter(String s){
    return updateIntProperty(HourMeter, s);
}

boolean SpaProperties::update_Relay1(String s){
    return updateIntProperty(Relay1, s);
}

boolean SpaProperties::update_Relay2(String s){
    return updateIntProperty(Relay2, s);
}

boolean SpaProperties::update_Relay3(String s){
    return updateIntProperty(Relay3, s);
}

boolean SpaProperties::update_Relay4(String s){
    return updateIntProperty(Relay4, s);
}

boolean SpaProperties::update_Relay5(String s){
    return updateIntProperty(Relay5, s);
}

boolean SpaProperties::update_Relay6(String s){
    return updateIntProperty(Relay6, s);
}

boolean SpaProperties::update_Relay7(String s){
    return updateIntProperty(Relay7, s);
}

boolean SpaProperties::update_Relay8(String s){
    return updateIntProperty(Relay8, s);
}

boolean SpaProperties::update_Relay9(String s){
    return updateIntProperty(Relay9, s);
}

boolean SpaProperties::update_CLMT(String s){
    return updateIntProperty(CLMT, s);
}

boolean SpaProperties::update_PHSE(String s){
    return updateIntProperty(PHSE, s);
}

boolean SpaProperties::update_LLM1(String s){
    return updateIntProperty(LLM1, s);
}

boolean SpaProperties::update_LLM2(String s){
    return updateIntProperty(LLM2, s);
}

boolean SpaProperties::update_LLM3(String s){
    return updateIntProperty(LLM3, s);
}

boolean SpaProperties::update_SVER(String s){
    return updateStringProperty(SVER, s);
}

boolean SpaProperties::update_Model(String s){
    return updateStringProperty(Model, s);
}

boolean SpaProperties::update_SerialNo1(String s){
    return updateStringProperty(SerialNo1, s);
}

boolean SpaProperties::update_SerialNo2(String s){
    return updateStringProperty(SerialNo2, s);
}

boolean SpaProperties::update_D1(String s) {
    return updateBool01Property(D1, s);
}

boolean SpaProperties::update_D2(String s) {
    return updateBool01Property(D2, s);
}

boolean SpaProperties::update_D3(String s) {
    return updateBool01Property(D3, s);
}

boolean SpaProperties::update_D4(String s) {
    return updateBool01Property(D4, s);
}

boolean SpaProperties::update_D5(String s) {
    return updateBool01Property(D5, s);
}

boolean SpaProperties::update_D6(String s) {
    return updateBool01Property(D6, s);
}

boolean SpaProperties::update_Pump(String s){
    return updateStringProperty(Pump, s);
}

boolean SpaProperties::update_LS(String s){
    return updateIntProperty(LS, s);
}

boolean SpaProperties::update_HV(String s) {
    return updateBool01Property(HV, s);
}

boolean SpaProperties::update_SnpMR(String s){
    return updateIntProperty(SnpMR, s);
}

boolean SpaProperties::update_Status(String s){
    return updateStringProperty(Status, s);
}

boolean SpaProperties::update_PrimeCount(String s){
    return updateIntProperty(PrimeCount, s);
}

boolean SpaProperties::update_EC(String s){
    return updateIntProperty(EC, s);
}

boolean SpaProperties::update_HAMB(String s){
    return updateIntProperty(HAMB, s);
}

boolean SpaProperties::update_HCON(String s){
    return updateIntProperty(HCON, s);
}

boolean SpaProperties::update_Mode(String s){
    return updateStringProperty(Mode, s);
}

boolean SpaProperties::update_Ser1_Timer(String s){
    return updateIntProperty(Ser1_Timer, s);
}

boolean SpaProperties::update_Ser2_Timer(String s){
    return updateIntProperty(Ser2_Timer, s);
}

boolean SpaProperties::update_Ser3_Timer(String s){
    return updateIntProperty(Ser3_Timer, s);
}

boolean SpaProperties::update_HeatMode(String s){
    return updateIntProperty(HeatMode, s);
}

boolean SpaProperties::update_PumpIdleTimer(String s){
    return updateIntProperty(PumpIdleTimer, s);
}

boolean SpaProperties::update_PumpRunTimer(String s){
    return updateIntProperty(PumpRunTimer, s);
}

boolean SpaProperties::update_AdtPoolHys(String s){
    return updateIntProperty(AdtPoolHys, s);
}

boolean SpaProperties::update_AdtHeaterHys(String s){
    return updateIntProperty(AdtHeaterHys, s);
}

boolean SpaProperties::update_Power(String s){
    return updateIntProperty(Power, s);
}

boolean SpaProperties::update_Power_kWh(String s){
    return updateIntProperty(Power_kWh, s);
}

boolean SpaProperties::update_Power_Today(String s){
    return updateIntProperty(Power_Today, s);
}

boolean SpaProperties::update_Power_Yesterday(String s){
    return updateIntProperty(Power_Yesterday, s);
}

boolean SpaProperties::update_ThermalCutOut(String s){
    return updateIntProperty(ThermalCutOut, s);
}

boolean SpaProperties::update_Test_D1(String s){
    return updateIntProperty(Test_D1, s);
}

boolean SpaProperties::update_Test_D2(String s){
    return updateIntProperty(Test_D2, s);
}

boolean SpaProperties::update_Test_D3(String s){
    return updateIntProperty(Test_D3, s);
}

boolean SpaProperties::update_ElementHeatSourceOffset(String s){
    return updateIntProperty(ElementHeatSourceOffset, s);
}

boolean SpaProperties::update_Frequency(String s){
    return updateIntProperty(Frequency, s);
}

boolean SpaProperties::update_HPHeatSourceOffset_Heat(String s){
    return updateIntProperty(HPHeatSourceOffset_Heat, s);
}

boolean SpaProperties::update_HPHeatSourceOffset_Cool(String s){
    return updateIntProperty(HPHeatSourceOffset_Cool, s);
}

boolean SpaProperties::update_HeatSourceOffTime(String s){
    return updateIntProperty(HeatSourceOffTime, s);
}

boolean SpaProperties::update_Vari_Speed(String s){
    return updateIntProperty(Vari_Speed, s);
}

boolean SpaProperties::update_Vari_Percent(String s){
    return updateIntProperty(Vari_Percent, s);
}

boolean SpaProperties::update_Vari_Mode(String s){
    return updateIntProperty(Vari_Mode, s);
}

boolean SpaProperties::update_RB_TP_Pump1(String s){
    return updateIntProperty(RB_TP_Pump1, s);
}

boolean SpaProperties::update_RB_TP_Pump2(String s){
    return updateIntProperty(RB_TP_Pump2, s);
}

boolean SpaProperties::update_RB_TP_Pump3(String s){
    return updateIntProperty(RB_TP_Pump3, s);
}

boolean SpaProperties::update_RB_TP_Pump4(String s){
    return updateIntProperty(RB_TP_Pump4, s);
}

boolean SpaProperties::update_RB_TP_Pump5(String s){
    return updateIntProperty(RB_TP_Pump5, s);
}

boolean SpaProperties::update_RB_TP_Blower(String s){
    return updateIntProperty(RB_TP_Blower, s);
}

boolean SpaProperties::update_RB_TP_Light(String s){
    return updateIntProperty(RB_TP_Light, s);
}

boolean SpaProperties::update_RB_TP_Auto(String s) {
    return updateBool01Property(RB_TP_Auto, s);
}

boolean SpaProperties::update_RB_TP_Heater(String s) {
    return updateBool01Property(RB_TP_Heater, s);
}

boolean SpaProperties::update_RB_TP_Ozone(String s) {
    return updateBool01Property(RB_TP_Ozone, s);
}

boolean SpaProperties::update_RB_TP_Sleep(String s) {
    return updateBool01Property(RB_TP_Sleep, s);
}

boolean SpaProperties::update_WTMP(String s){
    return updateIntProperty(WTMP, s);
}

boolean SpaProperties::update_CleanCycle(String s) {
    return updateBool01Property(CleanCycle, s);
}

boolean SpaProperties::update_VARIValue(String s){
    return updateIntProperty(VARIValue, s);
}

boolean SpaProperties::update_LBRTValue(String s){
    return updateIntProperty(LBRTValue, s);
}

boolean SpaProperties::update_CurrClr(String s){
    return updateIntProperty(CurrClr, s);
}

boolean SpaProperties::update_ColorMode(String s){
    return updateIntProperty(ColorMode, s);
}

boolean SpaProperties::update_LSPDValue(String s){
    return updateIntProperty(LSPDValue, s);
}

boolean SpaProperties::update_FiltHrs(String s){
    return updateIntProperty(FiltSetHrs, s);
}

boolean SpaProperties::update_FiltBlockHrs(String s){
    return updateIntProperty(FiltBlockHrs, s);
}

boolean SpaProperties::update_STMP(String s){
    return updateIntProperty(STMP, s);
}

boolean SpaProperties::update_L_24HOURS(String s){
    return updateIntProperty(L_24HOURS, s);
}

boolean SpaProperties::update_PSAV_LVL(String s){
    return updateIntProperty(PSAV_LVL, s);
}

boolean SpaProperties::update_PSAV_BGN(String s){
    return updateIntProperty(PSAV_BGN, s);
}

boolean SpaProperties::update_PSAV_END(String s){
    return updateIntProperty(PSAV_END, s);
}

boolean SpaProperties::update_L_1SNZ_DAY(String s){
    return updateIntProperty(L_1SNZ_DAY, s);
}

boolean SpaProperties::update_L_2SNZ_DAY(String s){
    return updateIntProperty(L_2SNZ_DAY, s);
}

boolean SpaProperties::update_L_1SNZ_BGN(String s){
    return updateIntProperty(L_1SNZ_BGN, s);
}

boolean SpaProperties::update_L_2SNZ_BGN(String s){
    return updateIntProperty(L_2SNZ_BGN, s);
}

boolean SpaProperties::update_L_1SNZ_END(String s){
    return updateIntProperty(L_1SNZ_END, s);
}

boolean SpaProperties::update_L_2SNZ_END(String s){
    return updateIntProperty(L_2SNZ_END, s);
}

boolean SpaProperties::update_DefaultScrn(String s){
    return updateIntProperty(DefaultScrn, s);
}

boolean SpaProperties::update_TOUT(String s){
    return updateIntProperty(TOUT, s);
}

boolean SpaProperties::update_VPMP(String s) {
    return updateBool01Property(VPMP, s);
}

boolean SpaProperties::update_HIFI(String s) {
    return updateBool01Property(HIFI, s);
}

boolean SpaProperties::update_BRND(String s){
    return updateIntProperty(BRND, s);
}

boolean SpaProperties::update_PRME(String s){
    return updateIntProperty(PRME, s);
}

boolean SpaProperties::update_ELMT(String s){
    return updateIntProperty(ELMT, s);
}

boolean SpaProperties::update_TYPE(String s){
    return updateIntProperty(TYPE, s);
}

boolean SpaProperties::update_GAS(String s){
    return updateIntProperty(GAS, s);
}

boolean SpaProperties::update_WCLNTime(String s){
    return updateIntProperty(WCLNTime, s);
}

boolean SpaProperties::update_TemperatureUnits(String s) {
    return updateBool01Property(TemperatureUnits, s);
}

boolean SpaProperties::update_OzoneOff(String s) {
    return updateBool01Property(OzoneOff, s);
}

boolean SpaProperties::update_Ozone24(String s) {
    return updateBool01Property(Ozone24, s);
}

boolean SpaProperties::update_Circ24(String s) {
    return updateBool01Property(Circ24, s);
}

boolean SpaProperties::update_CJET(String s) {
    return updateBool01Property(CJET, s);
}

boolean SpaProperties::update_VELE(String s) {
    return updateBool01Property(VELE, s);
}

boolean SpaProperties::update_V_Max(String s){
    return updateIntProperty(V_Max, s);
}

boolean SpaProperties::update_V_Min(String s){
    return updateIntProperty(V_Min, s);
}

boolean SpaProperties::update_V_Max_24(String s){
    return updateIntProperty(V_Max_24, s);
}

boolean SpaProperties::update_V_Min_24(String s){
    return updateIntProperty(V_Min_24, s);
}

boolean SpaProperties::update_CurrentZero(String s){
    return updateIntProperty(CurrentZero, s);
}

boolean SpaProperties::update_CurrentAdjust(String s){
    return updateIntProperty(CurrentAdjust, s);
}

boolean SpaProperties::update_VoltageAdjust(String s){
    return updateIntProperty(VoltageAdjust, s);
}

boolean SpaProperties::update_Ser1(String s){
    return updateIntProperty(Ser1, s);
}

boolean SpaProperties::update_Ser2(String s){
    return updateIntProperty(Ser2, s);
}

boolean SpaProperties::update_Ser3(String s){
    return updateIntProperty(Ser3, s);
}

boolean SpaProperties::update_VMAX(String s){
    return updateIntProperty(VMAX, s);
}

boolean SpaProperties::update_AHYS(String s){
    return updateIntProperty(AHYS, s);
}

boolean SpaProperties::update_HUSE(String s){
    // HUSE is a bool; only accept 0/1.
    return updateBool01Property(HUSE, s);
}

boolean SpaProperties::update_HELE(String s) {
    return updateBool01Property(HELE, s);
}

boolean SpaProperties::update_HPMP(String s){
    return updateIntProperty(HPMP, s);
}

boolean SpaProperties::update_PMIN(String s){
    return updateIntProperty(PMIN, s);
}

boolean SpaProperties::update_PFLT(String s){
    return updateIntProperty(PFLT, s);
}

boolean SpaProperties::update_PHTR(String s){
    return updateIntProperty(PHTR, s);
}

boolean SpaProperties::update_PMAX(String s){
    return updateIntProperty(PMAX, s);
}

boolean SpaProperties::update_F1_HR(String s){
    return updateIntProperty(F1_HR, s);
}

boolean SpaProperties::update_F1_Time(String s){
    return updateIntProperty(F1_Time, s);
}

boolean SpaProperties::update_F1_ER(String s){
    return updateIntProperty(F1_ER, s);
}

boolean SpaProperties::update_F1_I(String s){
    return updateIntProperty(F1_I, s);
}

boolean SpaProperties::update_F1_V(String s){
    return updateIntProperty(F1_V, s);
}

boolean SpaProperties::update_F1_PT(String s){
    return updateIntProperty(F1_PT, s);
}

boolean SpaProperties::update_F1_HT(String s){
    return updateIntProperty(F1_HT, s);
}

boolean SpaProperties::update_F1_CT(String s){
    return updateIntProperty(F1_CT, s);
}

boolean SpaProperties::update_F1_ST(String s){
    return updateIntProperty(F1_ST, s);
}

boolean SpaProperties::update_F1_PU(String s){
    return updateIntProperty(F1_PU, s);
}

boolean SpaProperties::update_F1_VE(String s) {
    return updateBool01Property(F1_VE, s);
}

boolean SpaProperties::update_F2_HR(String s){
    return updateIntProperty(F2_HR, s);
}

boolean SpaProperties::update_F2_Time(String s){
    return updateIntProperty(F2_Time, s);
}

boolean SpaProperties::update_F2_ER(String s){
    return updateIntProperty(F2_ER, s);
}

boolean SpaProperties::update_F2_I(String s){
    return updateIntProperty(F2_I, s);
}

boolean SpaProperties::update_F2_V(String s){
    return updateIntProperty(F2_V, s);
}

boolean SpaProperties::update_F2_PT(String s){
    return updateIntProperty(F2_PT, s);
}

boolean SpaProperties::update_F2_HT(String s){
    return updateIntProperty(F2_HT, s);
}

boolean SpaProperties::update_F2_CT(String s){
    return updateIntProperty(F2_CT, s);
}

boolean SpaProperties::update_F2_ST(String s){
    return updateIntProperty(F2_ST, s);
}

boolean SpaProperties::update_F2_PU(String s){
    return updateIntProperty(F2_PU, s);
}

boolean SpaProperties::update_F2_VE(String s) {
    return updateBool01Property(F2_VE, s);
}

boolean SpaProperties::update_F3_HR(String s){
    return updateIntProperty(F3_HR, s);
}

boolean SpaProperties::update_F3_Time(String s){
    return updateIntProperty(F3_Time, s);
}

boolean SpaProperties::update_F3_ER(String s){
    return updateIntProperty(F3_ER, s);
}

boolean SpaProperties::update_F3_I(String s){
    return updateIntProperty(F3_I, s);
}

boolean SpaProperties::update_F3_V(String s){
    return updateIntProperty(F3_V, s);
}

boolean SpaProperties::update_F3_PT(String s){
    return updateIntProperty(F3_PT, s);
}

boolean SpaProperties::update_F3_HT(String s){
    return updateIntProperty(F3_HT, s);
}

boolean SpaProperties::update_F3_CT(String s){
    return updateIntProperty(F3_CT, s);
}

boolean SpaProperties::update_F3_ST(String s){
    return updateIntProperty(F3_ST, s);
}

boolean SpaProperties::update_F3_PU(String s){
    return updateIntProperty(F3_PU, s);
}

boolean SpaProperties::update_F3_VE(String s) {
    return updateBool01Property(F3_VE, s);
}

boolean SpaProperties::update_Outlet_Blower(String s){
    return updateIntProperty(Outlet_Blower, s);
}

boolean SpaProperties::update_HP_Present(String s){
    return updateIntProperty(HP_Present, s);
}

boolean SpaProperties::update_HP_Ambient(String s){
    return updateIntProperty(HP_Ambient, s);
}


boolean SpaProperties::update_HP_Condensor(String s){
    return updateIntProperty(HP_Condensor, s);
}

boolean SpaProperties::update_HP_Compressor_State(String s) {
    return updateBool01Property(HP_Compressor_State, s);
}

boolean SpaProperties::update_HP_Fan_State(String s) {
    return updateBool01Property(HP_Fan_State, s);
}

boolean SpaProperties::update_HP_4W_Valve(String s) {
    return updateBool01Property(HP_4W_Valve, s);
}

boolean SpaProperties::update_HP_Heater_State(String s) {
    return updateBool01Property(HP_Heater_State, s);
}


boolean SpaProperties::update_HP_State(String s){
    return updateIntProperty(HP_State, s);
}

boolean SpaProperties::update_HP_Mode(String s){
    return updateIntProperty(HP_Mode, s);
}

boolean SpaProperties::update_HP_Defrost_Timer(String s){
    return updateIntProperty(HP_Defrost_Timer, s);
}

boolean SpaProperties::update_HP_Comp_Run_Timer(String s){
    return updateIntProperty(HP_Comp_Run_Timer, s);
}

boolean SpaProperties::update_HP_Low_Temp_Timer(String s){
    return updateIntProperty(HP_Low_Temp_Timer, s);
}

boolean SpaProperties::update_HP_Heat_Accum_Timer(String s){
    return updateIntProperty(HP_Heat_Accum_Timer, s);
}

boolean SpaProperties::update_HP_Sequence_Timer(String s){
    return updateIntProperty(HP_Sequence_Timer, s);
}

boolean SpaProperties::update_HP_Warning(String s){
    return updateIntProperty(HP_Warning, s);
}

boolean SpaProperties::update_FrezTmr(String s){
    return updateIntProperty(FrezTmr, s);
}

boolean SpaProperties::update_DBGN(String s){
    return updateIntProperty(DBGN, s);
}

boolean SpaProperties::update_DEND(String s){
    return updateIntProperty(DEND, s);
}

boolean SpaProperties::update_DCMP(String s){
    return updateIntProperty(DCMP, s);
}

boolean SpaProperties::update_DMAX(String s){
    return updateIntProperty(DMAX, s);
}

boolean SpaProperties::update_DELE(String s){
    return updateIntProperty(DELE, s);
}

boolean SpaProperties::update_DPMP(String s){
    return updateIntProperty(DPMP, s);
}

boolean SpaProperties::update_Pump1InstallState(String s){
    return updateStringProperty(Pump1InstallState, s);
}

boolean SpaProperties::update_Pump2InstallState(String s){
    return updateStringProperty(Pump2InstallState, s);
}

boolean SpaProperties::update_Pump3InstallState(String s){
    return updateStringProperty(Pump3InstallState, s);
}

boolean SpaProperties::update_Pump4InstallState(String s){
    return updateStringProperty(Pump4InstallState, s);
}

boolean SpaProperties::update_Pump5InstallState(String s){
    return updateStringProperty(Pump5InstallState, s);
}

boolean SpaProperties::update_Pump1OkToRun(String s) {
    return updateBool01Property(Pump1OkToRun, s);
}

boolean SpaProperties::update_Pump2OkToRun(String s) {
    return updateBool01Property(Pump2OkToRun, s);
}

boolean SpaProperties::update_Pump3OkToRun(String s) {
    return updateBool01Property(Pump3OkToRun, s);
}

boolean SpaProperties::update_Pump4OkToRun(String s) {
    return updateBool01Property(Pump4OkToRun, s);
}

boolean SpaProperties::update_Pump5OkToRun(String s) {
    return updateBool01Property(Pump5OkToRun, s);
}

boolean SpaProperties::update_LockMode(String s) {
    // LockMode is tri-state: 0 = unlocked, 1 = partial, 2 = full.
    return updateTriStateProperty(LockMode, s);
}
