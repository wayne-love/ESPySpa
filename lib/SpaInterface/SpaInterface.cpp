#include "SpaInterface.h"

#define BAUD_RATE 38400

SpaInterface::SpaInterface() : port(SPA_SERIAL) {
    SPA_SERIAL.setRxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.setTxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
    SPA_SERIAL.setTimeout(250);
}

SpaInterface::~SpaInterface() {}


void SpaInterface::setUpdateFrequency(int updateFrequency) {
    _updateFrequency = updateFrequency;
}

String SpaInterface::flushSerialReadBuffer(bool returnData) {
    int x = 0;
    String flushedData;

    debugD("Flushing serial stream - %i bytes in the buffer", port.available());
    while (port.available() > 0 && x++ < 5120) {
        int byte = port.read();
        if (returnData) {
            flushedData += (char)byte; // Append to buffer
        }
        debugV("%02X,", byte); // Log each byte
    }

    debugD("Flushed serial stream - %i bytes remaining in the buffer", port.available());

    if (returnData && !flushedData.isEmpty()) {
        debugD("Flushed data (%i bytes): %s", flushedData.length(), flushedData.c_str());
    }

    return flushedData;
}


void SpaInterface::sendCommand(String cmd) {

    flushSerialReadBuffer();

    debugD("Sending - %s",cmd.c_str());
    port.print('\n');
    port.flush();
    delay(50); // **TODO** is this needed?
    port.printf("%s\n", cmd.c_str());
    port.flush();

    ulong timeout = millis() + 1000; // wait up to 1 sec for a response

    debugD("Start waiting for a response");
    while (port.available()==0 and millis()<timeout) {}
    debugD("Finish waiting");

    _resultRegistersDirty = true; // we're trying to write to the registers so we can assume that they will now be dirty
}

String SpaInterface::sendCommandReturnResult(String cmd) {
    sendCommand(cmd);
    String result = port.readStringUntil('\r');
    port.read(); // get rid of the trailing LF char
    debugV("Read - %s",result.c_str());
    return result;
}

bool SpaInterface::sendCommandCheckResult(String cmd, String expected){
    String result = sendCommandReturnResult(cmd);
    bool outcome = result == expected;
    if (!outcome) debugW("Sent comment %s, expected %s, got %s",cmd.c_str(),expected.c_str(),result.c_str());
    return outcome;
}

bool SpaInterface::setRB_TP_Pump1(int mode){
    debugD("setRB_TP_Pump1 - %i",mode);

    if (sendCommandCheckResult("S22:"+String(mode),"S22-OK")) {
        update_RB_TP_Pump1(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump2(int mode){
    debugD("setRB_TP_Pump2 - %i",mode);

    if (sendCommandCheckResult("S23:"+String(mode),"S23-OK")) {
        update_RB_TP_Pump2(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump3(int mode){
    debugD("setRB_TP_Pump3 - %i",mode);

    if (sendCommandCheckResult("S24:"+String(mode),"S24-OK")) {
        update_RB_TP_Pump3(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump4(int mode){
    debugD("setRB_TP_Pump4 - %i",mode);

    if (sendCommandCheckResult("S25:"+String(mode),"S25-OK")) {
        update_RB_TP_Pump4(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump5(int mode){
    debugD("setRB_TP_Pump5 - %i",mode);

    if (sendCommandCheckResult("S26:"+String(mode),"S26-OK")) {
        update_RB_TP_Pump5(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Light(int mode){
    debugD("setRB_TP_Light - %i",mode);
    if (mode != getRB_TP_Light()) {
        if (sendCommandCheckResult("W14","W14")) {
            update_RB_TP_Light(String(mode));
            return true;
        }
        return false;
    }
    return true;
}

bool SpaInterface::setHELE(int mode){
    debugD("setHELE - %i", mode);

    if (sendCommandCheckResult("W98:"+String(mode),String(mode))) {
        update_HELE(String(mode));
        return true;
    }
    return false;
}


/// @brief Set the water temperature set point * 10 (380 = 38.0)
/// @param temp 
/// @return 
bool SpaInterface::setSTMP(int temp){
    debugD("setSTMP - %i", temp);
    String stemp = String(temp);

    if (sendCommandCheckResult("W40:" + stemp,stemp)) {
        update_STMP(stemp);
        return true;
    }
    return false;
}

bool SpaInterface::setL_1SNZ_DAY(int mode){
    debugD("setL_1SNZ_DAY - %i",mode);
    if (sendCommandCheckResult(String("W67:")+mode,String(mode))) {
        update_L_1SNZ_DAY(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setL_1SNZ_BGN(int mode){
    debugD("setL_1SNZ_BGN - %i",mode);
    if (sendCommandCheckResult(String("W68:")+mode,String(mode))) {
        update_L_1SNZ_BGN(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setL_1SNZ_END(int mode){
    debugD("setL_1SNZ_END - %i",mode);
    if (sendCommandCheckResult(String("W69:")+mode,String(mode))) {
        update_L_1SNZ_END(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setL_2SNZ_DAY(int mode){
    debugD("setL_2SNZ_DAY - %i",mode);
    if (sendCommandCheckResult(String("W70:")+mode,String(mode))) {
        update_L_2SNZ_DAY(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setL_2SNZ_BGN(int mode){
    debugD("setL_1SNZ_BGN - %i",mode);
    if (sendCommandCheckResult(String("W71:")+mode,String(mode))) {
        update_L_1SNZ_BGN(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setL_2SNZ_END(int mode){
    debugD("setL_1SNZ_END - %i",mode);
    if (sendCommandCheckResult(String("W72:")+mode,String(mode))) {
        update_L_1SNZ_END(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setHPMP(int mode){
    debugD("setHPMP - %i", mode);

    String smode = String(mode);

    if (sendCommandCheckResult("W99:"+smode,smode)) {
        update_HPMP(smode);
        return true;
    }
    return false;
}

bool SpaInterface::setHPMP(String mode){
    debugD("setHPMP - %s", mode.c_str());

    for (uint x=0; x<HPMPStrings.size(); x++) {
        if (HPMPStrings[x] == mode) {
            return setHPMP(x);
        }
    }
    return false;
}

bool SpaInterface::setColorMode(int mode){
    debugD("setColorMode - %i", mode);

    String smode = String(mode);

    if (sendCommandCheckResult("S07:"+smode,smode)) {
        update_ColorMode(smode);
        return true;
    }
    return false;
}

bool SpaInterface::setColorMode(String mode){
    debugD("setColorMode - %s", mode.c_str());
    for (uint x=0; x<colorModeStrings.size(); x++) {
        if (colorModeStrings[x] == mode) {
            return setColorMode(x);
        }
    }
    return false;
}

bool SpaInterface::setLBRTValue(int mode){
    debugD("setLBRTValue - %i", mode);

    String smode = String(mode);

    if (sendCommandCheckResult("S08:"+smode,smode)) {
        update_LBRTValue(smode);
        return true;
    }
    return false;
}

bool SpaInterface::setLSPDValue(int mode){
    debugD("setLSPDValue - %i", mode);

    String smode = String(mode);

    if (sendCommandCheckResult("S09:"+smode,smode)) {
        update_LSPDValue(smode);
        return true;
    }
    return false;
}

bool SpaInterface::setLSPDValue(String mode){
    debugD("setLSPDValue - %s", mode.c_str());
    int x = atoi(mode.c_str());
    if (x > 0 && x < 6) {
        return setLSPDValue(x);
    }
    return false;
}

bool SpaInterface::setCurrClr(int mode){
    debugD("setCurrClr - %i", mode);

    String smode = String(mode);

    if (sendCommandCheckResult("S10:"+smode,smode)) {
        update_CurrClr(smode);
        return true;
    }
    return false;
}

bool SpaInterface::setSpaTime(time_t t){
    debugD("setSpaTime");

    String tmp;
    bool outcome;

    tmp = String(year(t));
    outcome = sendCommandCheckResult("S01:"+tmp, tmp);

    tmp = String(month(t));
    outcome = outcome && sendCommandCheckResult("S02:"+tmp,tmp);

    tmp = String(day(t));
    outcome = outcome && sendCommandCheckResult("S03:"+tmp,tmp);
    
    tmp = String(hour(t));
    outcome = outcome && sendCommandCheckResult("S04:"+tmp,tmp);
    
    tmp = String(minute(t));
    outcome = outcome && sendCommandCheckResult("S05:"+tmp,tmp);
    
    tmp = String(second(t));
    outcome = outcome && sendCommandCheckResult("S06:"+tmp,tmp);
    
    return outcome;

}

bool SpaInterface::setOutlet_Blower(int mode){
    debugD("setOuput-Blower - %i", mode);

    String smode = String(mode);

    if (sendCommandCheckResult("S28:"+smode,"S28-OK")) {
        update_Outlet_Blower(smode);
        return true;
    }
    return false;
}

bool SpaInterface::setVARIValue(int mode){
    debugD("setVARIValue - %i", mode);
    if (mode > 0 && mode < 6) {
        String smode = String(mode);

        if (sendCommandCheckResult("S13:"+smode,smode+"  S13")) {
            update_VARIValue(smode);
            return true;
        }
    }
    return false;
}

bool SpaInterface::setMode(int mode){
    debugD("setMode - %i", mode);

    String smode = String(mode);

    if (sendCommandCheckResult("W66:"+smode,smode)) {
        update_Mode(spaModeStrings[mode]);
        return true;
    }
    return false;
}

bool SpaInterface::setMode(String mode){
    debugD("setMode - %s", mode.c_str());
    for (uint x=0; x<spaModeStrings.size(); x++) {
        if (spaModeStrings[x] == mode) {
            return setMode(x);
        }
    }
    return false;
}


bool SpaInterface::readStatus() {

    // We could just do a port.readString but this will always impose a
    // 250ms (or whatever the timeout is) delay penality.  This in turn,
    // along with the other unavoidable delays can cause the status of
    // properties to bounce in certain UI's (apple devices, home assistant, etc)

    debugD("Reading registers -");

    int field = 0;
    int registerCounter = 0;
    int currentRegisterSize = 0;
    int registerError = 0;
    validStatusResponse = false;
    String statusResponseTmp = "";

    while (field < statusResponseMaxFields)
    {
        statusResponseRaw[field] = port.readStringUntil(',');
        debugV("(%i,%s)",field,statusResponseRaw[field].c_str());

        statusResponseTmp = statusResponseTmp + statusResponseRaw[field]+",";

        if (statusResponseRaw[field].isEmpty()) { // If we get a empty field then we've had a bad read.
            debugE("Throwing exception - null string");
            return false;
        }
        if (field == 0 && !statusResponseRaw[field].startsWith("RF:")) { // If the first field is not "RF:" stop we don't have the start of the register
            debugE("Throwing exception - field: %i, value: %s", field, statusResponseRaw[field].c_str());
            return false;
        }
        // if we have reached a colon we are at the end of the current register
        // OR
        // if we are in register 11 (the last register) and have reached the minimum size we should stop
        if (statusResponseRaw[field][0] == ':' || (registerCounter == 11 && currentRegisterSize >= registerMinSize[registerCounter])) {
            debugV("Completed reading register: %s, number: %i, total fields counted: %i, minimum fields: %i", statusResponseRaw[field-currentRegisterSize+1], registerCounter, currentRegisterSize, registerMinSize[registerCounter]);
            if (registerMinSize[registerCounter] > currentRegisterSize) {
                debugE("Throwing exception - not enough fields in register: %s number: %i, total fields counted: %i, minimum fields: %i", statusResponseRaw[field-currentRegisterSize+1], registerCounter, currentRegisterSize, registerMinSize[registerCounter]);
                registerError++; // Instead of returning false, I want to read the complete response so it is available in the webinterface for debugging
            }
            registerCounter++;
            currentRegisterSize = 0;
        }
        // If we reach the last register we have finished reading...
        if (registerCounter >= 12) break;

        if (!_initialised) { // We only have to set these on the first read, they never change after that.
            if (statusResponseRaw[field] == "R2") R2 = field;
            else if (statusResponseRaw[field] == "R3") R3 = field;
            else if (statusResponseRaw[field] == "R4") R4 = field;
            else if (statusResponseRaw[field] == "R5") R5 = field;
            else if (statusResponseRaw[field] == "R6") R6 = field;
            else if (statusResponseRaw[field] == "R7") R7 = field;
            else if (statusResponseRaw[field] == "R9") R9 = field;
            else if (statusResponseRaw[field] == "RA") RA = field;
            else if (statusResponseRaw[field] == "RB") RB = field;
            else if (statusResponseRaw[field] == "RC") RC = field;
            else if (statusResponseRaw[field] == "RE") RE = field;
            else if (statusResponseRaw[field] == "RG") RG = field;
        }


        field++;
        currentRegisterSize++;
    }

    //Flush the remaining data from the buffer as the last field is meaningless
    statusResponseTmp = statusResponseTmp + flushSerialReadBuffer(true);

    statusResponse.update_Value(statusResponseTmp);

    if (registerCounter < 12) {
        debugE("Throwing exception - not enough registers, we only read: %i", registerCounter);
        return false;
    }

    if (registerError > 0) {
        debugE("Throwing exception - not enough fields in %i registers", registerError);
        return false;
    }

    if (field < statusResponseMinFields) {
        debugE("Throwing exception - %i fields read expecting at least %i",field, statusResponseMinFields);
        return false;
    }

    updateMeasures();
    _resultRegistersDirty = false;
    validStatusResponse = true;

    debugD("Reading registers - finish");
    return true;
}

bool SpaInterface::isInitialised() { 
    return _initialised; 
}


void SpaInterface::updateStatus() {

    flushSerialReadBuffer();

    debugD("Update status called");
    sendCommand("RF");

    _nextUpdateDue = millis() + FAILEDREADFREQUENCY;    
    if (readStatus()) {
        debugD("readStatus returned true");
        _nextUpdateDue = millis() + (_updateFrequency * 1000);
        _initialised = true;
        if (updateCallback != nullptr) { updateCallback(); }
    }
}


void SpaInterface::loop(){
    if ( _lastWaitMessage + 1000 < millis()) {
        debugD("Waiting...");
        _lastWaitMessage = millis();
    }

    if (_resultRegistersDirty) {
        _nextUpdateDue = millis() + 200;  // if we need to read the registers, pause a bit to see if there are more commands coming.
        _resultRegistersDirty = false;
    }

    if (millis()>_nextUpdateDue) {
        updateStatus();    
    }
}


void SpaInterface::setUpdateCallback(void (*f)()) {
    updateCallback = f;
}


void SpaInterface::clearUpdateCallback() {
    updateCallback = nullptr;
}


void SpaInterface::updateMeasures() {
    #pragma region R2
    update_MainsCurrent(statusResponseRaw[R2+1]);
    update_MainsVoltage(statusResponseRaw[R2+2]);
    update_CaseTemperature(statusResponseRaw[R2+3]);
/*
    update_PortCurrent(statusResponseRaw[R2+4]);
*/
    update_SpaTime(statusResponseRaw[R2+11], statusResponseRaw[R2+10], statusResponseRaw[R2+9], statusResponseRaw[R2+6], statusResponseRaw[R2+7], statusResponseRaw[R2+8]);
    update_HeaterTemperature(statusResponseRaw[R2+12]);
/*
    update_PoolTemperature(statusResponseRaw[R2+13]);
    update_WaterPresent(statusResponseRaw[R2+14]);
    update_AwakeMinutesRemaining(statusResponseRaw[R2+16]);
    update_FiltPumpRunTimeTotal(statusResponseRaw[R2+17]);
    update_FiltPumpReqMins(statusResponseRaw[R2+18]);
    update_LoadTimeOut(statusResponseRaw[R2+19]);
    update_HourMeter(statusResponseRaw[R2+20]);
    update_Relay1(statusResponseRaw[R2+21]);
    update_Relay2(statusResponseRaw[R2+22]);
    update_Relay3(statusResponseRaw[R2+23]);
    update_Relay4(statusResponseRaw[R2+24]);
    update_Relay5(statusResponseRaw[R2+25]);
    update_Relay6(statusResponseRaw[R2+26]);
    update_Relay7(statusResponseRaw[R2+27]);
    update_Relay8(statusResponseRaw[R2+28]);
    update_Relay9(statusResponseRaw[R2+29]); 
*/
    #pragma endregion
    #pragma region R3
/*
    update_CLMT(statusResponseRaw[R3+1]);
    update_PHSE(statusResponseRaw[R3+2]);
    update_LLM1(statusResponseRaw[R3+3]);
    update_LLM2(statusResponseRaw[R3+4]);
    update_LLM3(statusResponseRaw[R3+5]);
*/
    update_SVER(statusResponseRaw[R3+6]);
    update_Model(statusResponseRaw[R3+7]); 
    update_SerialNo1(statusResponseRaw[R3+8]);
    update_SerialNo2(statusResponseRaw[R3+9]); 
/*
    update_D1(statusResponseRaw[R3+10]);
    update_D2(statusResponseRaw[R3+11]);
    update_D3(statusResponseRaw[R3+12]);
    update_D4(statusResponseRaw[R3+13]);
    update_D5(statusResponseRaw[R3+14]);
    update_D6(statusResponseRaw[R3+15]);
    update_Pump(statusResponseRaw[R3+16]);
    update_LS(statusResponseRaw[R3+17]);
    update_HV(statusResponseRaw[R3+18]);
    update_SnpMR(statusResponseRaw[R3+19]);
*/
    update_Status(statusResponseRaw[R3+20]);
/*
    update_PrimeCount(statusResponseRaw[R3+21]);
    update_EC(statusResponseRaw[R3+22]);
    update_HAMB(statusResponseRaw[R3+23]);
    update_HCON(statusResponseRaw[R3+24]);
    // update_HV_2(statusResponseRaw[R3+25]);
*/
    #pragma endregion
    #pragma region R4
    update_Mode(statusResponseRaw[R4+1]);
/*
    update_Ser1_Timer(statusResponseRaw[R4+2]);
    update_Ser2_Timer(statusResponseRaw[R4+3]);
    update_Ser3_Timer(statusResponseRaw[R4+4]);
    update_HeatMode(statusResponseRaw[R4+5]);
    update_PumpIdleTimer(statusResponseRaw[R4+6]);
    update_PumpRunTimer(statusResponseRaw[R4+7]);
    update_AdtPoolHys(statusResponseRaw[R4+8]);
    update_AdtHeaterHys(statusResponseRaw[R4+9]);
*/
    update_Power(statusResponseRaw[R4+10]);
    update_Power_kWh(statusResponseRaw[R4+11]);
    update_Power_Today(statusResponseRaw[R4+12]);
    update_Power_Yesterday(statusResponseRaw[R4+13]);
/*
    update_ThermalCutOut(statusResponseRaw[R4+14]);
    update_Test_D1(statusResponseRaw[R4+15]);
    update_Test_D2(statusResponseRaw[R4+16]);
    update_Test_D3(statusResponseRaw[R4+17]);
    update_ElementHeatSourceOffset(statusResponseRaw[R4+18]);
    update_Frequency(statusResponseRaw[R4+19]);
    update_HPHeatSourceOffset_Heat(statusResponseRaw[R4+20]);
    update_HPHeatSourceOffset_Cool(statusResponseRaw[R4+21]);
    update_HeatSourceOffTime(statusResponseRaw[R4+22]);
    update_Vari_Speed(statusResponseRaw[R4+24]);
    update_Vari_Percent(statusResponseRaw[R4+25]);
    update_Vari_Mode(statusResponseRaw[R4+23]);
*/
    #pragma endregion
    #pragma region R5
    //R5
    // Unknown encoding - TouchPad2.updateValue();
    // Unknown encoding - TouchPad1.updateValue();
    //RB_TP_Blower.updateValue(statusResponseRaw[R5 + 5]);
/*
    update_RB_TP_Sleep(statusResponseRaw[R5 + 10]);
*/
    update_RB_TP_Ozone(statusResponseRaw[R5 + 11]);
    update_RB_TP_Heater(statusResponseRaw[R5 + 12]);
/*
    update_RB_TP_Auto(statusResponseRaw[R5 + 13]);
*/
    update_RB_TP_Light(statusResponseRaw[R5 + 14]);
    update_WTMP(statusResponseRaw[R5 + 15]);
/*
    update_CleanCycle(statusResponseRaw[R5 + 16]);
*/
    update_RB_TP_Pump1(statusResponseRaw[R5 + 18]);
    update_RB_TP_Pump2(statusResponseRaw[R5 + 19]);
    update_RB_TP_Pump3(statusResponseRaw[R5 + 20]);
    update_RB_TP_Pump4(statusResponseRaw[R5 + 21]);
    update_RB_TP_Pump5(statusResponseRaw[R5 + 22]);
    #pragma endregion
    #pragma region R6
    update_VARIValue(statusResponseRaw[R6 + 1]);
    update_LBRTValue(statusResponseRaw[R6 + 2]);
    update_CurrClr(statusResponseRaw[R6 + 3]);
    update_ColorMode(statusResponseRaw[R6 + 4]);
    update_LSPDValue(statusResponseRaw[R6 + 5]);
/*
    update_FiltSetHrs(statusResponseRaw[R6 + 6]);
    update_FiltBlockHrs(statusResponseRaw[R6 + 7]);
*/
    update_STMP(statusResponseRaw[R6 + 8]);
/*
    update_L_24HOURS(statusResponseRaw[R6 + 9]);
    update_PSAV_LVL(statusResponseRaw[R6 + 10]);
    update_PSAV_BGN(statusResponseRaw[R6 + 11]);
    update_PSAV_END(statusResponseRaw[R6 + 12]);
*/
    update_L_1SNZ_DAY(statusResponseRaw[R6 + 13]);
    update_L_2SNZ_DAY(statusResponseRaw[R6 + 14]);
    update_L_1SNZ_BGN(statusResponseRaw[R6 + 15]);
    update_L_2SNZ_BGN(statusResponseRaw[R6 + 16]);
    update_L_1SNZ_END(statusResponseRaw[R6 + 17]);
    update_L_2SNZ_END(statusResponseRaw[R6 + 18]);
/*
    update_DefaultScrn(statusResponseRaw[R6 + 19]);
    update_TOUT(statusResponseRaw[R6 + 20]);
    update_VPMP(statusResponseRaw[R6 + 21]);
    update_HIFI(statusResponseRaw[R6 + 22]);
    update_BRND(statusResponseRaw[R6 + 23]);
    update_PRME(statusResponseRaw[R6 + 24]);
    update_ELMT(statusResponseRaw[R6 + 25]);
    update_TYPE(statusResponseRaw[R6 + 26]);
    update_GAS(statusResponseRaw[R6 + 27]);
*/
    #pragma endregion
    #pragma region R7
/*
    update_WCLNTime(statusResponseRaw[R7 + 1]);
    // The following 2 may be reversed
    update_TemperatureUnits(statusResponseRaw[R7 + 3]);
    update_OzoneOff(statusResponseRaw[R7 + 2]);
    update_Ozone24(statusResponseRaw[R7 + 4]);
    update_Circ24(statusResponseRaw[R7 + 6]);
    update_CJET(statusResponseRaw[R7 + 5]);
    // 0 = off, 1 = step, 2 = variable
    update_VELE(statusResponseRaw[R7 + 7]);
    //update_StartDD(statusResponseRaw[R7 + 8]);
    //update_StartMM(statusResponseRaw[R7 + 9]);
    //update_StartYY(statusResponseRaw[R7 + 10]);
    update_V_Max(statusResponseRaw[R7 + 11]);
    update_V_Min(statusResponseRaw[R7 + 12]);
    update_V_Max_24(statusResponseRaw[R7 + 13]);
    update_V_Min_24(statusResponseRaw[R7 + 14]);
    update_CurrentZero(statusResponseRaw[R7 + 15]);
    update_CurrentAdjust(statusResponseRaw[R7 + 16]);
    update_VoltageAdjust(statusResponseRaw[R7 + 17]);
    // 168 is unknown
    update_Ser1(statusResponseRaw[R7 + 19]);
    update_Ser2(statusResponseRaw[R7 + 20]);
    update_Ser3(statusResponseRaw[R7 + 21]);
    update_VMAX(statusResponseRaw[R7 + 22]);
    update_AHYS(statusResponseRaw[R7 + 23]);
    update_HUSE(statusResponseRaw[R7 + 24]);
*/
    update_HELE(statusResponseRaw[R7 + 25]);
    update_HPMP(statusResponseRaw[R7 + 26]);
/*
    update_PMIN(statusResponseRaw[R7 + 27]);
    update_PFLT(statusResponseRaw[R7 + 28]);
    update_PHTR(statusResponseRaw[R7 + 29]);
    update_PMAX(statusResponseRaw[R7 + 30]);
*/
    #pragma endregion
    #pragma region R9
/*
    update_F1_HR(statusResponseRaw[R9 + 2]);
    update_F1_Time(statusResponseRaw[R9 + 3]);
    update_F1_ER(statusResponseRaw[R9 + 4]);
    update_F1_I(statusResponseRaw[R9 + 5]);
    update_F1_V(statusResponseRaw[R9 + 6]);
    update_F1_PT(statusResponseRaw[R9 + 7]);
    update_F1_HT(statusResponseRaw[R9 + 8]);
    update_F1_CT(statusResponseRaw[R9 + 9]);
    update_F1_PU(statusResponseRaw[R9 + 10]);
    update_F1_VE(statusResponseRaw[R9 + 11]);
    update_F1_ST(statusResponseRaw[R9 + 12]);
*/
    #pragma endregion
    #pragma region RA
/*
    update_F2_HR(statusResponseRaw[RA + 2]);
    update_F2_Time(statusResponseRaw[RA + 3]);
    update_F2_ER(statusResponseRaw[RA + 4]);
    update_F2_I(statusResponseRaw[RA + 5]);
    update_F2_V(statusResponseRaw[RA + 6]);
    update_F2_PT(statusResponseRaw[RA + 7]);
    update_F2_HT(statusResponseRaw[RA + 8]);
    update_F2_CT(statusResponseRaw[RA + 9]);
    update_F2_PU(statusResponseRaw[RA + 10]);
    update_F2_VE(statusResponseRaw[RA + 11]);
    update_F2_ST(statusResponseRaw[RA + 12]);
*/
    #pragma endregion
    #pragma region RB
/*
    update_F3_HR(statusResponseRaw[RB + 2]);
    update_F3_Time(statusResponseRaw[RB + 3]);
    update_F3_ER(statusResponseRaw[RB + 4]);
    update_F3_I(statusResponseRaw[RB + 5]);
    update_F3_V(statusResponseRaw[RB + 6]);
    update_F3_PT(statusResponseRaw[RB + 7]);
    update_F3_HT(statusResponseRaw[RB + 8]);
    update_F3_CT(statusResponseRaw[RB + 9]);
    update_F3_PU(statusResponseRaw[RB + 10]);
    update_F3_VE(statusResponseRaw[RB + 11]);
    update_F3_ST(statusResponseRaw[RB + 12]);
*/
    #pragma endregion
    #pragma region RC
/*
    //Outlet_Heater.updateValue(statusResponseRaw[]);
    //Outlet_Circ.updateValue(statusResponseRaw[]);
    //Outlet_Sanitise.updateValue(statusResponseRaw[]);
    //Outlet_Pump1.updateValue(statusResponseRaw[]);
    //Outlet_Pump2.updateValue(statusResponseRaw[]);
    //Outlet_Pump4.updateValue(statusResponseRaw[]);
    //Outlet_Pump5.updateValue(statusResponseRaw[]);
*/
    update_Outlet_Blower(statusResponseRaw[RC + 10]);
    #pragma endregion
    #pragma region RE
    update_HP_Present(statusResponseRaw[RE + 1]);
    //HP_FlowSwitch.updateValue(statusResponseRaw[]);
    //HP_HighSwitch.updateValue(statusResponseRaw[]);
    //HP_LowSwitch.updateValue(statusResponseRaw[]);
    //HP_CompCutOut.updateValue(statusResponseRaw[]);
    //HP_ExCutOut.updateValue(statusResponseRaw[]);
    //HP_D1.updateValue(statusResponseRaw[]);
    //HP_D2.updateValue(statusResponseRaw[]);
    //HP_D3.updateValue(statusResponseRaw[]);
    update_HP_Ambient(statusResponseRaw[RE + 10]);
    update_HP_Condensor(statusResponseRaw[RE + 11]);
/*
    update_HP_Compressor_State(statusResponseRaw[RE + 12]);
    update_HP_Fan_State(statusResponseRaw[RE + 13]);
    update_HP_4W_Valve(statusResponseRaw[RE + 14]);
    update_HP_Heater_State(statusResponseRaw[RE + 15]);
    update_HP_State(statusResponseRaw[RE + 16]);
    update_HP_Mode(statusResponseRaw[RE + 17]);
    update_HP_Defrost_Timer(statusResponseRaw[RE + 18]);
    update_HP_Comp_Run_Timer(statusResponseRaw[RE + 19]);
    update_HP_Low_Temp_Timer(statusResponseRaw[RE + 20]);
    update_HP_Heat_Accum_Timer(statusResponseRaw[RE + 21]);
    update_HP_Sequence_Timer(statusResponseRaw[RE + 22]);
    update_HP_Warning(statusResponseRaw[RE + 23]);
    update_FrezTmr(statusResponseRaw[RE + 24]);
    update_DBGN(statusResponseRaw[RE + 25]);
    update_DEND(statusResponseRaw[RE + 26]);
    update_DCMP(statusResponseRaw[RE + 27]);
    update_DMAX(statusResponseRaw[RE + 28]);
    update_DELE(statusResponseRaw[RE + 29]);
    update_DPMP(statusResponseRaw[RE + 30]);
    //CMAX.updateValue(statusResponseRaw[]);
    //HP_Compressor.updateValue(statusResponseRaw[]);
    //HP_Pump_State.updateValue(statusResponseRaw[]);
    //HP_Status.updateValue(statusResponseRaw[]);
*/
    #pragma endregion
    #pragma region RG
    update_Pump1InstallState(statusResponseRaw[RG + 7]);
    update_Pump2InstallState(statusResponseRaw[RG + 8]);
    update_Pump3InstallState(statusResponseRaw[RG + 9]);
    update_Pump4InstallState(statusResponseRaw[RG + 10]);
    update_Pump5InstallState(statusResponseRaw[RG + 11]);
/*
    update_Pump1OkToRun(statusResponseRaw[RG + 1]);
    update_Pump2OkToRun(statusResponseRaw[RG + 2]);
    update_Pump3OkToRun(statusResponseRaw[RG + 3]);
    update_Pump4OkToRun(statusResponseRaw[RG + 4]);
    update_Pump5OkToRun(statusResponseRaw[RG + 5]);
    update_LockMode(statusResponseRaw[RG + 12]);
*/
    #pragma endregion

};