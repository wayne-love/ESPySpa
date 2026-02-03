#include "SpaProperties.h"


static boolean updateIntProperty(Property<int>& prop, const String& s) {
    // Accept only base-10 integers with no trailing characters.
    const char* raw = s.c_str();
    char* end = nullptr;
    long value = strtol(raw, &end, 10);
    if (raw == end || *end != '\0') {
        return false;
    }

    prop.update_Value(static_cast<int>(value));
    return true;
}

static boolean updateBool01Property(Property<bool>& prop, const String& s) {
    // Accept only "0" or "1" to avoid accidental truthy values.
    if (s!="0" && s!="1") {
        return false;
    }

    prop.update_Value(s == "1");
    return true;
}

static boolean updateStringProperty(Property<String>& prop, const String& s) {
    // Store raw string values without validation.
    prop.update_Value(s);
    return true;
}

static boolean updateTriStateProperty(Property<int>& prop, const String& s) {
    // Accept only "0", "1", or "2" for explicit tri-state fields.
    if (s!="0" && s!="1" && s!="2") {
        return false;
    }

    prop.update_Value(s.toInt());
    return true;
}

boolean SpaProperties::update_SpaDayOfWeek(const String& s){
    return updateIntProperty(SpaDayOfWeek, s);
}

boolean SpaProperties::update_SpaTime(const String& year, const String& month, const String& day, const String& hour, const String& minute, const String& second){

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

boolean SpaProperties::update_MainsVoltage(const String& s){
    return updateIntProperty(MainsVoltage, s);
}

boolean SpaProperties::update_CaseTemperature(const String& s){
    return updateIntProperty(CaseTemperature, s);
}

boolean SpaProperties::update_PortCurrent(const String& s){
    return updateIntProperty(PortCurrent, s);
}

boolean SpaProperties::update_HeaterTemperature(const String& s){
    return updateIntProperty(HeaterTemperature, s);
}

boolean SpaProperties::update_PoolTemperature(const String& s){
    return updateIntProperty(PoolTemperature, s);
}

boolean SpaProperties::update_WaterPresent(const String& s) {
    return updateBool01Property(WaterPresent, s);
}

boolean SpaProperties::update_AwakeMinutesRemaining(const String& s){
    return updateIntProperty(AwakeMinutesRemaining, s);
}

boolean SpaProperties::update_FiltPumpRunTimeTotal(const String& s){
    return updateIntProperty(FiltPumpRunTimeTotal, s);
}

boolean SpaProperties::update_FiltPumpReqMins(const String& s){
    return updateIntProperty(FiltPumpReqMins, s);
}

boolean SpaProperties::update_LoadTimeOut(const String& s){
    return updateIntProperty(LoadTimeOut, s);
}

boolean SpaProperties::update_HourMeter(const String& s){
    return updateIntProperty(HourMeter, s);
}

boolean SpaProperties::update_Relay1(const String& s){
    return updateIntProperty(Relay1, s);
}

boolean SpaProperties::update_Relay2(const String& s){
    return updateIntProperty(Relay2, s);
}

boolean SpaProperties::update_Relay3(const String& s){
    return updateIntProperty(Relay3, s);
}

boolean SpaProperties::update_Relay4(const String& s){
    return updateIntProperty(Relay4, s);
}

boolean SpaProperties::update_Relay5(const String& s){
    return updateIntProperty(Relay5, s);
}

boolean SpaProperties::update_Relay6(const String& s){
    return updateIntProperty(Relay6, s);
}

boolean SpaProperties::update_Relay7(const String& s){
    return updateIntProperty(Relay7, s);
}

boolean SpaProperties::update_Relay8(const String& s){
    return updateIntProperty(Relay8, s);
}

boolean SpaProperties::update_Relay9(const String& s){
    return updateIntProperty(Relay9, s);
}

boolean SpaProperties::update_CLMT(const String& s){
    return updateIntProperty(CLMT, s);
}

boolean SpaProperties::update_PHSE(const String& s){
    return updateIntProperty(PHSE, s);
}

boolean SpaProperties::update_LLM1(const String& s){
    return updateIntProperty(LLM1, s);
}

boolean SpaProperties::update_LLM2(const String& s){
    return updateIntProperty(LLM2, s);
}

boolean SpaProperties::update_LLM3(const String& s){
    return updateIntProperty(LLM3, s);
}

boolean SpaProperties::update_SVER(const String& s){
    return updateStringProperty(SVER, s);
}

boolean SpaProperties::update_Model(const String& s){
    return updateStringProperty(Model, s);
}

boolean SpaProperties::update_SerialNo1(const String& s){
    return updateStringProperty(SerialNo1, s);
}

boolean SpaProperties::update_SerialNo2(const String& s){
    return updateStringProperty(SerialNo2, s);
}

boolean SpaProperties::update_D1(const String& s) {
    return updateBool01Property(D1, s);
}

boolean SpaProperties::update_D2(const String& s) {
    return updateBool01Property(D2, s);
}

boolean SpaProperties::update_D3(const String& s) {
    return updateBool01Property(D3, s);
}

boolean SpaProperties::update_D4(const String& s) {
    return updateBool01Property(D4, s);
}

boolean SpaProperties::update_D5(const String& s) {
    return updateBool01Property(D5, s);
}

boolean SpaProperties::update_D6(const String& s) {
    return updateBool01Property(D6, s);
}

boolean SpaProperties::update_Pump(const String& s){
    return updateStringProperty(Pump, s);
}

boolean SpaProperties::update_LS(const String& s){
    return updateIntProperty(LS, s);
}

boolean SpaProperties::update_HV(const String& s) {
    return updateBool01Property(HV, s);
}

boolean SpaProperties::update_SnpMR(const String& s){
    return updateIntProperty(SnpMR, s);
}

boolean SpaProperties::update_Status(const String& s){
    return updateStringProperty(Status, s);
}

boolean SpaProperties::update_PrimeCount(const String& s){
    return updateIntProperty(PrimeCount, s);
}

boolean SpaProperties::update_EC(const String& s){
    return updateIntProperty(EC, s);
}

boolean SpaProperties::update_HAMB(const String& s){
    return updateIntProperty(HAMB, s);
}

boolean SpaProperties::update_HCON(const String& s){
    return updateIntProperty(HCON, s);
}

boolean SpaProperties::update_Mode(const String& s){
    return updateStringProperty(Mode, s);
}

boolean SpaProperties::update_Ser1_Timer(const String& s){
    return updateIntProperty(Ser1_Timer, s);
}

boolean SpaProperties::update_Ser2_Timer(const String& s){
    return updateIntProperty(Ser2_Timer, s);
}

boolean SpaProperties::update_Ser3_Timer(const String& s){
    return updateIntProperty(Ser3_Timer, s);
}

boolean SpaProperties::update_HeatMode(const String& s){
    return updateIntProperty(HeatMode, s);
}

boolean SpaProperties::update_PumpIdleTimer(const String& s){
    return updateIntProperty(PumpIdleTimer, s);
}

boolean SpaProperties::update_PumpRunTimer(const String& s){
    return updateIntProperty(PumpRunTimer, s);
}

boolean SpaProperties::update_AdtPoolHys(const String& s){
    return updateIntProperty(AdtPoolHys, s);
}

boolean SpaProperties::update_AdtHeaterHys(const String& s){
    return updateIntProperty(AdtHeaterHys, s);
}

boolean SpaProperties::update_Power(const String& s){
    return updateIntProperty(Power, s);
}

boolean SpaProperties::update_Power_kWh(const String& s){
    return updateIntProperty(Power_kWh, s);
}

boolean SpaProperties::update_Power_Today(const String& s){
    return updateIntProperty(Power_Today, s);
}

boolean SpaProperties::update_Power_Yesterday(const String& s){
    return updateIntProperty(Power_Yesterday, s);
}

boolean SpaProperties::update_ThermalCutOut(const String& s){
    return updateIntProperty(ThermalCutOut, s);
}

boolean SpaProperties::update_Test_D1(const String& s){
    return updateIntProperty(Test_D1, s);
}

boolean SpaProperties::update_Test_D2(const String& s){
    return updateIntProperty(Test_D2, s);
}

boolean SpaProperties::update_Test_D3(const String& s){
    return updateIntProperty(Test_D3, s);
}

boolean SpaProperties::update_ElementHeatSourceOffset(const String& s){
    return updateIntProperty(ElementHeatSourceOffset, s);
}

boolean SpaProperties::update_Frequency(const String& s){
    return updateIntProperty(Frequency, s);
}

boolean SpaProperties::update_HPHeatSourceOffset_Heat(const String& s){
    return updateIntProperty(HPHeatSourceOffset_Heat, s);
}

boolean SpaProperties::update_HPHeatSourceOffset_Cool(const String& s){
    return updateIntProperty(HPHeatSourceOffset_Cool, s);
}

boolean SpaProperties::update_HeatSourceOffTime(const String& s){
    return updateIntProperty(HeatSourceOffTime, s);
}

boolean SpaProperties::update_Vari_Speed(const String& s){
    return updateIntProperty(Vari_Speed, s);
}

boolean SpaProperties::update_Vari_Percent(const String& s){
    return updateIntProperty(Vari_Percent, s);
}

boolean SpaProperties::update_Vari_Mode(const String& s){
    return updateIntProperty(Vari_Mode, s);
}

boolean SpaProperties::update_RB_TP_Pump1(const String& s){
    return updateIntProperty(RB_TP_Pump1, s);
}

boolean SpaProperties::update_RB_TP_Pump2(const String& s){
    return updateIntProperty(RB_TP_Pump2, s);
}

boolean SpaProperties::update_RB_TP_Pump3(const String& s){
    return updateIntProperty(RB_TP_Pump3, s);
}

boolean SpaProperties::update_RB_TP_Pump4(const String& s){
    return updateIntProperty(RB_TP_Pump4, s);
}

boolean SpaProperties::update_RB_TP_Pump5(const String& s){
    return updateIntProperty(RB_TP_Pump5, s);
}

boolean SpaProperties::update_RB_TP_Blower(const String& s){
    return updateIntProperty(RB_TP_Blower, s);
}

boolean SpaProperties::update_RB_TP_Light(const String& s){
    return updateIntProperty(RB_TP_Light, s);
}

boolean SpaProperties::update_RB_TP_Auto(const String& s) {
    return updateBool01Property(RB_TP_Auto, s);
}

boolean SpaProperties::update_RB_TP_Heater(const String& s) {
    return updateBool01Property(RB_TP_Heater, s);
}

boolean SpaProperties::update_RB_TP_Ozone(const String& s) {
    return updateBool01Property(RB_TP_Ozone, s);
}

boolean SpaProperties::update_RB_TP_Sleep(const String& s) {
    return updateBool01Property(RB_TP_Sleep, s);
}

boolean SpaProperties::update_WTMP(const String& s){
    return updateIntProperty(WTMP, s);
}

boolean SpaProperties::update_CleanCycle(const String& s) {
    return updateBool01Property(CleanCycle, s);
}

boolean SpaProperties::update_VARIValue(const String& s){
    return updateIntProperty(VARIValue, s);
}

boolean SpaProperties::update_LBRTValue(const String& s){
    return updateIntProperty(LBRTValue, s);
}

boolean SpaProperties::update_CurrClr(const String& s){
    return updateIntProperty(CurrClr, s);
}

boolean SpaProperties::update_ColorMode(const String& s){
    return updateIntProperty(ColorMode, s);
}

boolean SpaProperties::update_LSPDValue(const String& s){
    return updateIntProperty(LSPDValue, s);
}

boolean SpaProperties::update_FiltHrs(const String& s){
    return updateIntProperty(FiltSetHrs, s);
}

boolean SpaProperties::update_FiltBlockHrs(const String& s){
    return updateIntProperty(FiltBlockHrs, s);
}

boolean SpaProperties::update_L_24HOURS(const String& s){
    return updateIntProperty(L_24HOURS, s);
}

boolean SpaProperties::update_PSAV_LVL(const String& s){
    return updateIntProperty(PSAV_LVL, s);
}

boolean SpaProperties::update_PSAV_BGN(const String& s){
    return updateIntProperty(PSAV_BGN, s);
}

boolean SpaProperties::update_PSAV_END(const String& s){
    return updateIntProperty(PSAV_END, s);
}

boolean SpaProperties::update_L_2SNZ_DAY(const String& s){
    return updateIntProperty(L_2SNZ_DAY, s);
}

boolean SpaProperties::update_L_1SNZ_BGN(const String& s){
    return updateIntProperty(L_1SNZ_BGN, s);
}

boolean SpaProperties::update_L_2SNZ_BGN(const String& s){
    return updateIntProperty(L_2SNZ_BGN, s);
}

boolean SpaProperties::update_L_1SNZ_END(const String& s){
    return updateIntProperty(L_1SNZ_END, s);
}

boolean SpaProperties::update_L_2SNZ_END(const String& s){
    return updateIntProperty(L_2SNZ_END, s);
}

boolean SpaProperties::update_DefaultScrn(const String& s){
    return updateIntProperty(DefaultScrn, s);
}

boolean SpaProperties::update_TOUT(const String& s){
    return updateIntProperty(TOUT, s);
}

boolean SpaProperties::update_VPMP(const String& s) {
    return updateBool01Property(VPMP, s);
}

boolean SpaProperties::update_HIFI(const String& s) {
    return updateBool01Property(HIFI, s);
}

boolean SpaProperties::update_BRND(const String& s){
    return updateIntProperty(BRND, s);
}

boolean SpaProperties::update_PRME(const String& s){
    return updateIntProperty(PRME, s);
}

boolean SpaProperties::update_ELMT(const String& s){
    return updateIntProperty(ELMT, s);
}

boolean SpaProperties::update_TYPE(const String& s){
    return updateIntProperty(TYPE, s);
}

boolean SpaProperties::update_GAS(const String& s){
    return updateIntProperty(GAS, s);
}

boolean SpaProperties::update_WCLNTime(const String& s){
    return updateIntProperty(WCLNTime, s);
}

boolean SpaProperties::update_TemperatureUnits(const String& s) {
    return updateBool01Property(TemperatureUnits, s);
}

boolean SpaProperties::update_OzoneOff(const String& s) {
    return updateBool01Property(OzoneOff, s);
}

boolean SpaProperties::update_Ozone24(const String& s) {
    return updateBool01Property(Ozone24, s);
}

boolean SpaProperties::update_Circ24(const String& s) {
    return updateBool01Property(Circ24, s);
}

boolean SpaProperties::update_CJET(const String& s) {
    return updateBool01Property(CJET, s);
}

boolean SpaProperties::update_VELE(const String& s) {
    return updateBool01Property(VELE, s);
}

boolean SpaProperties::update_V_Max(const String& s){
    return updateIntProperty(V_Max, s);
}

boolean SpaProperties::update_V_Min(const String& s){
    return updateIntProperty(V_Min, s);
}

boolean SpaProperties::update_V_Max_24(const String& s){
    return updateIntProperty(V_Max_24, s);
}

boolean SpaProperties::update_V_Min_24(const String& s){
    return updateIntProperty(V_Min_24, s);
}

boolean SpaProperties::update_CurrentZero(const String& s){
    return updateIntProperty(CurrentZero, s);
}

boolean SpaProperties::update_CurrentAdjust(const String& s){
    return updateIntProperty(CurrentAdjust, s);
}

boolean SpaProperties::update_VoltageAdjust(const String& s){
    return updateIntProperty(VoltageAdjust, s);
}

boolean SpaProperties::update_Ser1(const String& s){
    return updateIntProperty(Ser1, s);
}

boolean SpaProperties::update_Ser2(const String& s){
    return updateIntProperty(Ser2, s);
}

boolean SpaProperties::update_Ser3(const String& s){
    return updateIntProperty(Ser3, s);
}

boolean SpaProperties::update_VMAX(const String& s){
    return updateIntProperty(VMAX, s);
}

boolean SpaProperties::update_AHYS(const String& s){
    return updateIntProperty(AHYS, s);
}

boolean SpaProperties::update_HUSE(const String& s){
    // HUSE is a bool; only accept 0/1.
    return updateBool01Property(HUSE, s);
}

boolean SpaProperties::update_HELE(const String& s) {
    return updateBool01Property(HELE, s);
}

boolean SpaProperties::update_HPMP(const String& s){
    return updateIntProperty(HPMP, s);
}

boolean SpaProperties::update_PMIN(const String& s){
    return updateIntProperty(PMIN, s);
}

boolean SpaProperties::update_PFLT(const String& s){
    return updateIntProperty(PFLT, s);
}

boolean SpaProperties::update_PHTR(const String& s){
    return updateIntProperty(PHTR, s);
}

boolean SpaProperties::update_PMAX(const String& s){
    return updateIntProperty(PMAX, s);
}

boolean SpaProperties::update_F1_HR(const String& s){
    return updateIntProperty(F1_HR, s);
}

boolean SpaProperties::update_F1_Time(const String& s){
    return updateIntProperty(F1_Time, s);
}

boolean SpaProperties::update_F1_ER(const String& s){
    return updateIntProperty(F1_ER, s);
}

boolean SpaProperties::update_F1_I(const String& s){
    return updateIntProperty(F1_I, s);
}

boolean SpaProperties::update_F1_V(const String& s){
    return updateIntProperty(F1_V, s);
}

boolean SpaProperties::update_F1_PT(const String& s){
    return updateIntProperty(F1_PT, s);
}

boolean SpaProperties::update_F1_HT(const String& s){
    return updateIntProperty(F1_HT, s);
}

boolean SpaProperties::update_F1_CT(const String& s){
    return updateIntProperty(F1_CT, s);
}

boolean SpaProperties::update_F1_ST(const String& s){
    return updateIntProperty(F1_ST, s);
}

boolean SpaProperties::update_F1_PU(const String& s){
    return updateIntProperty(F1_PU, s);
}

boolean SpaProperties::update_F1_VE(const String& s) {
    return updateBool01Property(F1_VE, s);
}

boolean SpaProperties::update_F2_HR(const String& s){
    return updateIntProperty(F2_HR, s);
}

boolean SpaProperties::update_F2_Time(const String& s){
    return updateIntProperty(F2_Time, s);
}

boolean SpaProperties::update_F2_ER(const String& s){
    return updateIntProperty(F2_ER, s);
}

boolean SpaProperties::update_F2_I(const String& s){
    return updateIntProperty(F2_I, s);
}

boolean SpaProperties::update_F2_V(const String& s){
    return updateIntProperty(F2_V, s);
}

boolean SpaProperties::update_F2_PT(const String& s){
    return updateIntProperty(F2_PT, s);
}

boolean SpaProperties::update_F2_HT(const String& s){
    return updateIntProperty(F2_HT, s);
}

boolean SpaProperties::update_F2_CT(const String& s){
    return updateIntProperty(F2_CT, s);
}

boolean SpaProperties::update_F2_ST(const String& s){
    return updateIntProperty(F2_ST, s);
}

boolean SpaProperties::update_F2_PU(const String& s){
    return updateIntProperty(F2_PU, s);
}

boolean SpaProperties::update_F2_VE(const String& s) {
    return updateBool01Property(F2_VE, s);
}

boolean SpaProperties::update_F3_HR(const String& s){
    return updateIntProperty(F3_HR, s);
}

boolean SpaProperties::update_F3_Time(const String& s){
    return updateIntProperty(F3_Time, s);
}

boolean SpaProperties::update_F3_ER(const String& s){
    return updateIntProperty(F3_ER, s);
}

boolean SpaProperties::update_F3_I(const String& s){
    return updateIntProperty(F3_I, s);
}

boolean SpaProperties::update_F3_V(const String& s){
    return updateIntProperty(F3_V, s);
}

boolean SpaProperties::update_F3_PT(const String& s){
    return updateIntProperty(F3_PT, s);
}

boolean SpaProperties::update_F3_HT(const String& s){
    return updateIntProperty(F3_HT, s);
}

boolean SpaProperties::update_F3_CT(const String& s){
    return updateIntProperty(F3_CT, s);
}

boolean SpaProperties::update_F3_ST(const String& s){
    return updateIntProperty(F3_ST, s);
}

boolean SpaProperties::update_F3_PU(const String& s){
    return updateIntProperty(F3_PU, s);
}

boolean SpaProperties::update_F3_VE(const String& s) {
    return updateBool01Property(F3_VE, s);
}

boolean SpaProperties::update_Outlet_Blower(const String& s){
    return updateIntProperty(Outlet_Blower, s);
}

boolean SpaProperties::update_HP_Present(const String& s){
    return updateIntProperty(HP_Present, s);
}

boolean SpaProperties::update_HP_Ambient(const String& s){
    return updateIntProperty(HP_Ambient, s);
}


boolean SpaProperties::update_HP_Condensor(const String& s){
    return updateIntProperty(HP_Condensor, s);
}

boolean SpaProperties::update_HP_Compressor_State(const String& s) {
    return updateBool01Property(HP_Compressor_State, s);
}

boolean SpaProperties::update_HP_Fan_State(const String& s) {
    return updateBool01Property(HP_Fan_State, s);
}

boolean SpaProperties::update_HP_4W_Valve(const String& s) {
    return updateBool01Property(HP_4W_Valve, s);
}

boolean SpaProperties::update_HP_Heater_State(const String& s) {
    return updateBool01Property(HP_Heater_State, s);
}


boolean SpaProperties::update_HP_State(const String& s){
    return updateIntProperty(HP_State, s);
}

boolean SpaProperties::update_HP_Mode(const String& s){
    return updateIntProperty(HP_Mode, s);
}

boolean SpaProperties::update_HP_Defrost_Timer(const String& s){
    return updateIntProperty(HP_Defrost_Timer, s);
}

boolean SpaProperties::update_HP_Comp_Run_Timer(const String& s){
    return updateIntProperty(HP_Comp_Run_Timer, s);
}

boolean SpaProperties::update_HP_Low_Temp_Timer(const String& s){
    return updateIntProperty(HP_Low_Temp_Timer, s);
}

boolean SpaProperties::update_HP_Heat_Accum_Timer(const String& s){
    return updateIntProperty(HP_Heat_Accum_Timer, s);
}

boolean SpaProperties::update_HP_Sequence_Timer(const String& s){
    return updateIntProperty(HP_Sequence_Timer, s);
}

boolean SpaProperties::update_HP_Warning(const String& s){
    return updateIntProperty(HP_Warning, s);
}

boolean SpaProperties::update_FrezTmr(const String& s){
    return updateIntProperty(FrezTmr, s);
}

boolean SpaProperties::update_DBGN(const String& s){
    return updateIntProperty(DBGN, s);
}

boolean SpaProperties::update_DEND(const String& s){
    return updateIntProperty(DEND, s);
}

boolean SpaProperties::update_DCMP(const String& s){
    return updateIntProperty(DCMP, s);
}

boolean SpaProperties::update_DMAX(const String& s){
    return updateIntProperty(DMAX, s);
}

boolean SpaProperties::update_DELE(const String& s){
    return updateIntProperty(DELE, s);
}

boolean SpaProperties::update_DPMP(const String& s){
    return updateIntProperty(DPMP, s);
}

boolean SpaProperties::update_Pump1InstallState(const String& s){
    return updateStringProperty(Pump1InstallState, s);
}

boolean SpaProperties::update_Pump2InstallState(const String& s){
    return updateStringProperty(Pump2InstallState, s);
}

boolean SpaProperties::update_Pump3InstallState(const String& s){
    return updateStringProperty(Pump3InstallState, s);
}

boolean SpaProperties::update_Pump4InstallState(const String& s){
    return updateStringProperty(Pump4InstallState, s);
}

boolean SpaProperties::update_Pump5InstallState(const String& s){
    return updateStringProperty(Pump5InstallState, s);
}

boolean SpaProperties::update_Pump1OkToRun(const String& s) {
    return updateBool01Property(Pump1OkToRun, s);
}

boolean SpaProperties::update_Pump2OkToRun(const String& s) {
    return updateBool01Property(Pump2OkToRun, s);
}

boolean SpaProperties::update_Pump3OkToRun(const String& s) {
    return updateBool01Property(Pump3OkToRun, s);
}

boolean SpaProperties::update_Pump4OkToRun(const String& s) {
    return updateBool01Property(Pump4OkToRun, s);
}

boolean SpaProperties::update_Pump5OkToRun(const String& s) {
    return updateBool01Property(Pump5OkToRun, s);
}

boolean SpaProperties::update_LockMode(const String& s) {
    // LockMode is tri-state: 0 = unlocked, 1 = partial, 2 = full.
    return updateTriStateProperty(LockMode, s);
}

