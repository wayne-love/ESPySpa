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

    if (sendCommandCheckResult("S22:" + String(mode),"S22-OK")) {
        pump(1).currentState = mode;
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump2(int mode){
    debugD("setRB_TP_Pump2 - %i",mode);

    if (sendCommandCheckResult("S23:" + String(mode),"S23-OK")) {
        pump(2).currentState = mode;
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump3(int mode){
    debugD("setRB_TP_Pump3 - %i",mode);

    if (sendCommandCheckResult("S24:" + String(mode),"S24-OK")) {
        pump(3).currentState = mode;
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump4(int mode){
    debugD("setRB_TP_Pump4 - %i",mode);

    if (sendCommandCheckResult("S25:" + String(mode),"S25-OK")) {
        pump(4).currentState = mode;
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump5(int mode){
    debugD("setRB_TP_Pump5 - %i",mode);

    if (sendCommandCheckResult("S26:" + String(mode),"S26-OK")) {
        pump(5).currentState = mode;
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Light(int mode){
    debugD("setRB_TP_Light - %i",mode);
    if (mode != RB_TP_Light) {
        if (sendCommandCheckResult("W14","W14")) {
            RB_TP_Light = mode;
            return true;
        }
        return false;
    }
    return true;
}

bool SpaInterface::setHELE(int mode){
    debugD("setHELE - %i", mode);

    if (sendCommandCheckResult("W98:" + String(mode), String(mode))) {
        HELE = mode;
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

    if (sendCommandCheckResult("W40:" + stemp, stemp)) {
        STMP = temp;
        return true;
    }
    return false;
}

bool SpaInterface::setL_1SNZ_DAY(int mode){
    debugD("setL_1SNZ_DAY - %i",mode);
    if (sendCommandCheckResult("W67:" + String(mode),String(mode))) {
        L_1SNZ_DAY = mode;
        return true;
    }
    return false;
}

bool SpaInterface::setL_1SNZ_BGN(int mode){
    debugD("setL_1SNZ_BGN - %i",mode);
    if (sendCommandCheckResult("W68:" + String(mode),String(mode))) {
        L_1SNZ_BGN = mode;
        return true;
    }
    return false;
}

bool SpaInterface::setL_1SNZ_END(int mode){
    debugD("setL_1SNZ_END - %i",mode);
    if (sendCommandCheckResult("W69:" + String(mode),String(mode))) {
        L_1SNZ_END = mode;
        return true;
    }
    return false;
}

bool SpaInterface::setL_2SNZ_DAY(int mode){
    debugD("setL_2SNZ_DAY - %i",mode);
    if (sendCommandCheckResult("W70:" + String(mode),String(mode))) {
        L_2SNZ_DAY = mode;
        return true;
    }
    return false;
}

bool SpaInterface::setL_2SNZ_BGN(int mode){
    debugD("setL_1SNZ_BGN - %i",mode);
    if (sendCommandCheckResult("W71:" + String(mode),String(mode))) {
        L_2SNZ_BGN = mode; 
        return true;
    }
    return false;
}

bool SpaInterface::setL_2SNZ_END(int mode){
    debugD("setL_1SNZ_END - %i",mode);
    if (sendCommandCheckResult("W72:" + String(mode),String(mode))) {
        L_2SNZ_END = mode;
        return true;
    }
    return false;
}

bool SpaInterface::setHPMP(int mode){
    debugD("setHPMP - %i", mode);

    String smode = String(mode);

    if (sendCommandCheckResult("W99:"+smode,smode)) {
        HPMP = mode;
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
        ColorMode = mode;
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
        LBRTValue = mode;
        return true;
    }
    return false;
}

bool SpaInterface::setLSPDValue(int mode){
    debugD("setLSPDValue - %i", mode);

    String smode = String(mode);

    if (sendCommandCheckResult("S09:"+smode,smode)) {
        LSPDValue = mode;
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
        CurrClr = mode;
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
        Outlet_Blower = smode;
        return true;
    }
    return false;
}

bool SpaInterface::setVARIValue(int mode){
    debugD("setVARIValue - %i", mode);
    if (mode > 0 && mode < 6) {
        String smode = String(mode);

        if (sendCommandCheckResult("S13:"+smode,smode+"  S13")) {
            VARIValue = smode;
            return true;
        }
    }
    return false;
}

bool SpaInterface::setMode(int mode){
    debugD("setMode - %i", mode);

    String smode = String(mode);

    if (sendCommandCheckResult("W66:"+smode,smode)) {
        Mode = spaModeStrings[mode];
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
    MainsCurrent = statusResponseRaw[R2+1];
    MainsVoltage = statusResponseRaw[R2+2];
    CaseTemperature = statusResponseRaw[R2+3];
    PortCurrent = statusResponseRaw[R2+4];
    update_SpaTime(statusResponseRaw[R2+11], statusResponseRaw[R2+10], statusResponseRaw[R2+9], statusResponseRaw[R2+6], statusResponseRaw[R2+7], statusResponseRaw[R2+8]);
    HeaterTemperature = statusResponseRaw[R2+12];
    PoolTemperature = statusResponseRaw[R2+13];
    WaterPresent = statusResponseRaw[R2+14];
    AwakeMinutesRemaining = statusResponseRaw[R2+16];
    FiltPumpRunTimeTotal = statusResponseRaw[R2+17];
    FiltPumpReqMins = statusResponseRaw[R2+18];
    LoadTimeOut = statusResponseRaw[R2+19];
    HourMeter = statusResponseRaw[R2+20];
    Relay1 = statusResponseRaw[R2+21];
    Relay2 = statusResponseRaw[R2+22];
    Relay3 = statusResponseRaw[R2+23];
    Relay4 = statusResponseRaw[R2+24];
    Relay5 = statusResponseRaw[R2+25];
    Relay6 = statusResponseRaw[R2+26];
    Relay7 = statusResponseRaw[R2+27];
    Relay8 = statusResponseRaw[R2+28];
    Relay9 = statusResponseRaw[R2+29];
    #pragma endregion
    #pragma region R3
    CLMT = statusResponseRaw[R3+1];
    PHSE = statusResponseRaw[R3+2];
    LLM1 = statusResponseRaw[R3+3]; 
    LLM2 = statusResponseRaw[R3+4];
    LLM3 = statusResponseRaw[R3+5];
    SVER = statusResponseRaw[R3+6];
    Model = statusResponseRaw[R3+7];
    SerialNo1 = statusResponseRaw[R3+8];
    SerialNo2 = statusResponseRaw[R3+9];
    D1 = statusResponseRaw[R3+10];
    D2 = statusResponseRaw[R3+11];
    D3 = statusResponseRaw[R3+12];
    D4 = statusResponseRaw[R3+13];
    D5 = statusResponseRaw[R3+14];
    D6 = statusResponseRaw[R3+15];
    Pump = statusResponseRaw[R3+16];
    LS = statusResponseRaw[R3+17];
    HV = statusResponseRaw[R3+18];
    SnpMR = statusResponseRaw[R3+19];
    Status = statusResponseRaw[R3+20];
    PrimeCount = statusResponseRaw[R3+21];
    EC = statusResponseRaw[R3+22];
    HAMB = statusResponseRaw[R3+23];
    HCON = statusResponseRaw[R3+24];
    // update_HV_2(statusResponseRaw[R3+25]);
    #pragma endregion
    #pragma region R4
    Mode = statusResponseRaw[R4+1];
    Ser1_Timer = statusResponseRaw[R4+2];
    Ser2_Timer = statusResponseRaw[R4+3];
    Ser3_Timer = statusResponseRaw[R4+4];
    HeatMode = statusResponseRaw[R4+5];
    PumpIdleTimer = statusResponseRaw[R4+6];
    PumpRunTimer = statusResponseRaw[R4+7];
    AdtPoolHys = statusResponseRaw[R4+8];
    AdtHeaterHys = statusResponseRaw[R4+9];
    Power = statusResponseRaw[R4+10];
    Power_kWh = statusResponseRaw[R4+11];
    Power_Today = statusResponseRaw[R4+12];
    Power_Yesterday = statusResponseRaw[R4+13];
    ThermalCutOut = statusResponseRaw[R4+14];
    Test_D1 = statusResponseRaw[R4+15];
    Test_D2 = statusResponseRaw[R4+16];
    Test_D3 = statusResponseRaw[R4+17];
    ElementHeatSourceOffset = statusResponseRaw[R4+18];
    Frequency = statusResponseRaw[R4+19];
    HPHeatSourceOffset_Heat = statusResponseRaw[R4+20];
    HPHeatSourceOffset_Cool = statusResponseRaw[R4+21];
    HeatSourceOffTime = statusResponseRaw[R4+22];
    Vari_Speed = statusResponseRaw[R4+24];
    Vari_Percent = statusResponseRaw[R4+25];
    Vari_Mode = statusResponseRaw[R4+23];
    #pragma endregion
    #pragma region R5
    //R5
    // Unknown encoding - TouchPad2 = statusResponseRaw[R5 + x];
    // Unknown encoding - TouchPad1 = statusResponseRaw[R5 + x]; 
    //RB_TP_Blower = statusResponseRaw[R5 + 5];
    RB_TP_Sleep = statusResponseRaw[R5 + 10];
    RB_TP_Ozone = statusResponseRaw[R5 + 11];
    RB_TP_Heater = statusResponseRaw[R5 + 12];
    RB_TP_Auto = statusResponseRaw[R5 + 13];
    RB_TP_Light = statusResponseRaw[R5 + 14];
    WTMP = statusResponseRaw[R5 + 15];
    CleanCycle = statusResponseRaw[R5 + 16];
    pump(1).currentState = statusResponseRaw[R5 + 18];
    pump(2).currentState = statusResponseRaw[R5 + 19];
    pump(3).currentState = statusResponseRaw[R5 + 20];
    pump(4).currentState = statusResponseRaw[R5 + 21];
    pump(5).currentState = statusResponseRaw[R5 + 22];
    #pragma endregion
    #pragma region R6
    VARIValue = statusResponseRaw[R6 + 1];
    LBRTValue = statusResponseRaw[R6 + 2];
    CurrClr = statusResponseRaw[R6 + 3];
    ColorMode = statusResponseRaw[R6 + 4];
    LSPDValue = statusResponseRaw[R6 + 5];
    FiltSetHrs = statusResponseRaw[R6 + 6];
    FiltBlockHrs = statusResponseRaw[R6 + 7];
    STMP = statusResponseRaw[R6 + 8];
    L_24HOURS = statusResponseRaw[R6 + 9];
    PSAV_LVL = statusResponseRaw[R6 + 10];
    PSAV_BGN = statusResponseRaw[R6 + 11];
    PSAV_END = statusResponseRaw[R6 + 12];
    L_1SNZ_DAY = statusResponseRaw[R6 + 13];
    L_2SNZ_DAY = statusResponseRaw[R6 + 14];
    L_1SNZ_BGN = statusResponseRaw[R6 + 15];
    L_2SNZ_BGN = statusResponseRaw[R6 + 16];
    L_1SNZ_END = statusResponseRaw[R6 + 17];
    L_2SNZ_END = statusResponseRaw[R6 + 18];
    DefaultScrn = statusResponseRaw[R6 + 19];
    TOUT = statusResponseRaw[R6 + 20];
    VPMP = statusResponseRaw[R6 + 21];
    HIFI = statusResponseRaw[R6 + 22];
    BRND = statusResponseRaw[R6 + 23];
    PRME = statusResponseRaw[R6 + 24];
    ELMT = statusResponseRaw[R6 + 25];
    TYPE = statusResponseRaw[R6 + 26];
    GAS = statusResponseRaw[R6 + 27];
    #pragma endregion
    #pragma region R7
    WCLNTime = statusResponseRaw[R7 + 1];
    // The following 2 may be reversed
    TemperatureUnits = statusResponseRaw[R7 + 3];
    OzoneOff = statusResponseRaw[R7 + 2];
    Ozone24 = statusResponseRaw[R7 + 4];
    Circ24 = statusResponseRaw[R7 + 6];
    CJET = statusResponseRaw[R7 + 5];
    // 0 = off, 1 = step, 2 = variable
    VELE = statusResponseRaw[R7 + 7];
    //StartDD = statusResponseRaw[R7 + 8];
    //StartMM = statusResponseRaw[R7 + 9];
    //StartYY = statusResponseRaw[R7 + 10];
    V_Max = statusResponseRaw[R7 + 11];
    V_Min = statusResponseRaw[R7 + 12];
    V_Max_24 = statusResponseRaw[R7 + 13];
    V_Min_24 = statusResponseRaw[R7 + 14];
    CurrentZero = statusResponseRaw[R7 + 15];
    CurrentAdjust = statusResponseRaw[R7 + 16];
    VoltageAdjust = statusResponseRaw[R7 + 17];
    // 168 is unknown
    Ser1 = statusResponseRaw[R7 + 19];
    Ser2 = statusResponseRaw[R7 + 20];
    Ser3 = statusResponseRaw[R7 + 21];
    VMAX = statusResponseRaw[R7 + 22];
    AHYS = statusResponseRaw[R7 + 23];
    HUSE = statusResponseRaw[R7 + 24];
    HELE = statusResponseRaw[R7 + 25];
    HPMP = statusResponseRaw[R7 + 26];
    PMIN = statusResponseRaw[R7 + 27];
    PFLT = statusResponseRaw[R7 + 28];
    PHTR = statusResponseRaw[R7 + 29];
    PMAX = statusResponseRaw[R7 + 30];
    #pragma endregion
    #pragma region R9
    F1_HR = statusResponseRaw[R9 + 2];
    F1_Time = statusResponseRaw[R9 + 3];
    F1_ER = statusResponseRaw[R9 + 4];
    F1_I = statusResponseRaw[R9 + 5];
    F1_V = statusResponseRaw[R9 + 6];
    F1_PT = statusResponseRaw[R9 + 7];
    F1_HT = statusResponseRaw[R9 + 8];
    F1_CT = statusResponseRaw[R9 + 9];
    F1_PU = statusResponseRaw[R9 + 10];
    F1_VE = statusResponseRaw[R9 + 11];
    F1_ST = statusResponseRaw[R9 + 12];
    #pragma endregion
    #pragma region RA
    F2_HR = statusResponseRaw[RA + 2];
    F2_Time = statusResponseRaw[RA + 3];
    F2_ER = statusResponseRaw[RA + 4];
    F2_I = statusResponseRaw[RA + 5];
    F2_V = statusResponseRaw[RA + 6];
    F2_PT = statusResponseRaw[RA + 7];
    F2_HT = statusResponseRaw[RA + 8];
    F2_CT = statusResponseRaw[RA + 9];
    F2_PU = statusResponseRaw[RA + 10];
    F2_VE = statusResponseRaw[RA + 11];
    F2_ST = statusResponseRaw[RA + 12];
    #pragma endregion
    #pragma region RB
    F3_HR = statusResponseRaw[RB + 2];
    F3_Time = statusResponseRaw[RB + 3];
    F3_ER = statusResponseRaw[RB + 4];
    F3_I = statusResponseRaw[RB + 5];
    F3_V = statusResponseRaw[RB + 6];
    F3_PT = statusResponseRaw[RB + 7];
    F3_HT = statusResponseRaw[RB + 8];
    F3_CT = statusResponseRaw[RB + 9];
    F3_PU = statusResponseRaw[RB + 10];
    F3_VE = statusResponseRaw[RB + 11];
    F3_ST = statusResponseRaw[RB + 12];
    #pragma endregion
    #pragma region RC
    //Outlet_Heater = statusResponseRaw[];
    //Outlet_Circ = statusResponseRaw[];
    //Outlet_Sanitise = statusResponseRaw[];
    //Outlet_Pump1 = statusResponseRaw[];
    //Outlet_Pump2 = statusResponseRaw[];
    //Outlet_Pump4 = statusResponseRaw[];
    //Outlet_Pump5 = statusResponseRaw[];
    Outlet_Blower = statusResponseRaw[RC + 10];
    #pragma endregion
    #pragma region RE
    HP_Present = statusResponseRaw[RE + 1];
    //HP_FlowSwitch = statusResponseRaw[];
    //HP_HighSwitch = statusResponseRaw[];
    //HP_LowSwitch = statusResponseRaw[];
    //HP_CompCutOut = statusResponseRaw[];
    //HP_ExCutOut = statusResponseRaw[];
    //HP_D1 = statusResponseRaw[];
    //HP_D2 = statusResponseRaw[];
    //HP_D3 = statusResponseRaw[];
    HP_Ambient = statusResponseRaw[RE + 10];
    HP_Condensor = statusResponseRaw[RE + 11];
    HP_Compressor_State = statusResponseRaw[RE + 12];
    HP_Fan_State = statusResponseRaw[RE + 13];
    HP_4W_Valve = statusResponseRaw[RE + 14];
    HP_Heater_State = statusResponseRaw[RE + 15];
    HP_State = statusResponseRaw[RE + 16];
    HP_Mode = statusResponseRaw[RE + 17];
    HP_Defrost_Timer = statusResponseRaw[RE + 18];
    HP_Comp_Run_Timer = statusResponseRaw[RE + 19];
    HP_Low_Temp_Timer = statusResponseRaw[RE + 20];
    HP_Heat_Accum_Timer = statusResponseRaw[RE + 21];
    HP_Sequence_Timer = statusResponseRaw[RE + 22];
    HP_Warning = statusResponseRaw[RE + 23];
    FrezTmr = statusResponseRaw[RE + 24];
    DBGN = statusResponseRaw[RE + 25];
    DEND = statusResponseRaw[RE + 26];
    DCMP = statusResponseRaw[RE + 27];
    DMAX = statusResponseRaw[RE + 28];
    DELE = statusResponseRaw[RE + 29];
    DPMP = statusResponseRaw[RE + 30];
    //CMAX = statusResponseRaw[];
    //HP_Compressor = statusResponseRaw[];
    //HP_Pump_State = statusResponseRaw[];
    //HP_Status = statusResponseRaw[];
    #pragma endregion
    #pragma region RG
    pump(1).installState = statusResponseRaw[RG + 7];
    pump(2).installState = statusResponseRaw[RG + 8];
    pump(3).installState = statusResponseRaw[RG + 9];
    pump(4).installState = statusResponseRaw[RG + 10];
    pump(5).installState = statusResponseRaw[RG + 11];
    pump(1).okToRun = statusResponseRaw[RG + 1];
    pump(2).okToRun = statusResponseRaw[RG + 2];
    pump(3).okToRun = statusResponseRaw[RG + 3];
    pump(4).okToRun = statusResponseRaw[RG + 4];
    pump(5).okToRun = statusResponseRaw[RG + 5];
    LockMode = statusResponseRaw[RG + 12];
    #pragma endregion

};