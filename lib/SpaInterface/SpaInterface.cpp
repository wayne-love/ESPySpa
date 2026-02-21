#include "SpaInterface.h"

#define BAUD_RATE 38400

SpaInterface::SpaInterface() : port(SPA_SERIAL) {
    SPA_SERIAL.setRxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.setTxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
    SPA_SERIAL.setTimeout(250);
}

SpaInterface::~SpaInterface() {}


void SpaInterface::setSpaPollFrequency(int updateFrequency) {
    _updateFrequency = updateFrequency;
}

String SpaInterface::flushSerialReadBuffer(bool returnData) {
    int x = 0;
    String flushedData;

    debugV("Flushing serial stream - %i bytes in the buffer", port.available());
    while (port.available() > 0 && x++ < 5120) {
        int byte = port.read();
        if (returnData) {
            flushedData += (char)byte; // Append to buffer
        }
        debugV("%02X,", byte); // Log each byte
    }

    debugD("Flushed serial stream - %i bytes remaining in the buffer", port.available());

    if (returnData && !flushedData.isEmpty()) {
        debugV("Flushed data (%i bytes): %s", flushedData.length(), flushedData.c_str());
    }

    return flushedData;
}


void SpaInterface::sendCommand(String cmd) {

    flushSerialReadBuffer();

    debugV("Sending - '%s'",cmd.c_str());
    port.print('\n');
    port.flush();
    delay(50); // **TODO** is this needed?
    port.printf("%s\n", cmd.c_str());
    port.flush();

    ulong timeout = millis() + 1000; // wait up to 1 sec for a response

    debugV("Start waiting for a response");
    while (port.available()==0 and millis()<timeout) {}
    debugV("Finish waiting");

    _resultRegistersDirty = true; // we're trying to write to the registers so we can assume that they will now be dirty
}

String SpaInterface::sendCommandReturnResult(String cmd) {
    sendCommand(cmd);
    String result = port.readStringUntil('\r');
    port.read(); // get rid of the trailing LF char
    debugV("Read - '%s'",result.c_str());
    return result;
}

bool SpaInterface::sendCommandCheckResult(String cmd, String expected){
    String result = sendCommandReturnResult(cmd);
    bool outcome = result == expected;
    debugD("Sent command '%s', expected '%s', got '%s'",cmd.c_str(),expected.c_str(),result.c_str());
    return outcome;
}

bool SpaInterface::setRB_TP_Pump1(int mode){
    debugD("setRB_TP_Pump1 - %i",mode);
    if (mode == getRB_TP_Pump1()) {
        debugD("No Pump1 change detected - current %i, new %i", getRB_TP_Pump1(), mode);
        return true;
    }

    if (sendCommandCheckResult("S22:"+String(mode),"S22-OK")) {
        update_RB_TP_Pump1(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump2(int mode){
    debugD("setRB_TP_Pump2 - %i",mode);
    if (mode == getRB_TP_Pump2()) {
        debugD("No Pump2 change detected - current %i, new %i", getRB_TP_Pump2(), mode);
        return true;
    }

    if (sendCommandCheckResult("S23:"+String(mode),"S23-OK")) {
        update_RB_TP_Pump2(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump3(int mode){
    debugD("setRB_TP_Pump3 - %i",mode);
    if (mode == getRB_TP_Pump3()) {
        debugD("No Pump3 change detected - current %i, new %i", getRB_TP_Pump3(), mode);
        return true;
    }

    if (sendCommandCheckResult("S24:"+String(mode),"S24-OK")) {
        update_RB_TP_Pump3(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump4(int mode){
    debugD("setRB_TP_Pump4 - %i",mode);
    if (mode == getRB_TP_Pump4()) {
        debugD("No Pump4 change detected - current %i, new %i", getRB_TP_Pump4(), mode);
        return true;
    }

    if (sendCommandCheckResult("S25:"+String(mode),"S25-OK")) {
        update_RB_TP_Pump4(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump5(int mode){
    debugD("setRB_TP_Pump5 - %i",mode);
    if (mode == getRB_TP_Pump5()) {
        debugD("No Pump5 change detected - current %i, new %i", getRB_TP_Pump5(), mode);
        return true;
    }

    if (sendCommandCheckResult("S26:"+String(mode),"S26-OK")) {
        update_RB_TP_Pump5(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Light(int mode){
    debugD("setRB_TP_Light - %i",mode);
    if (mode == getRB_TP_Light()) {
        debugD("No RB_TP_Light change detected - current %i, new %i", getRB_TP_Light(), mode);
        return true;
    }

    if (sendCommandCheckResult("W14","W14")) {
        update_RB_TP_Light(String(mode));
        return true;
    }
    return false;
}

bool SpaInterface::setHELE(int mode){
    debugD("setHELE - %i", mode);
    if (mode == getHELE()) {
        debugD("No HELE change detected - current %i, new %i", getHELE(), mode);
        return true;
    }

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

    if (temp==STMP.get()) {  // todo: does this ever evaluate to true?
        debugD("No STMP change detected - current %i, new %i", STMP.get(), temp);
        return true; // No change needed
    }

    if (temp % 2 != 0) {
        temp++;
    }

    String stemp = String(temp);

    if (sendCommandCheckResult("W40:" + stemp, stemp)) {
        STMP.update(temp);
        return true;
    }
    return false;
}

bool SpaInterface::validateSTMP(int value) {
    return value >= 50 && value <= 410;
}

bool SpaInterface::validateHPMP(int value) {
    return value >= 0 && value <= 3;
}

bool SpaInterface::validateColorMode(int value) {
    return value >= 0 && value <= 4;
}

bool SpaInterface::validate_SNZ_DAY(int value) {
    for (size_t i = 0; i < array_count(SNZ_DAY_Map); i++) {
        if (SNZ_DAY_Map[i].value == value) {
            return true;
        }
    }
    return false;
}

bool SpaInterface::validate_SNZ_TIME(int value) {
    if (value < 0) {
        return false;
    }

    int hour = value / 256;
    int minute = value % 256;

    return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

bool SpaInterface::setL_1SNZ_DAY(int mode){
    debugD("setL_1SNZ_DAY - %i",mode);
    if (mode == L_1SNZ_DAY.get()) {
        debugD("No L_1SNZ_DAY change detected - current %i, new %i", L_1SNZ_DAY.get(), mode);
        return true;
    }

    if (sendCommandCheckResult(String("W67:")+mode,String(mode))) {
        L_1SNZ_DAY.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setL_1SNZ_BGN(int mode){
    debugD("setL_1SNZ_BGN - %i",mode);
    if (mode == L_1SNZ_BGN.get()) {
        debugD("No L_1SNZ_BGN change detected - current %i, new %i", L_1SNZ_BGN.get(), mode);
        return true;
    }

    if (sendCommandCheckResult(String("W68:")+mode,String(mode))) {
        return true;
    }
    return false;
}

bool SpaInterface::setL_1SNZ_END(int mode){
    debugD("setL_1SNZ_END - %i",mode);
    if (mode == L_1SNZ_END.get()) {
        debugD("No L_1SNZ_END change detected - current %i, new %i", L_1SNZ_END.get(), mode);
        return true;
    }
    
    if (sendCommandCheckResult(String("W69:")+mode,String(mode))) {
        return true;
    }
    return false;
}

bool SpaInterface::setL_2SNZ_DAY(int mode){
    debugD("setL_2SNZ_DAY - %i",mode);
    if (mode == L_2SNZ_DAY.get()) {
        debugD("No L_2SNZ_DAY change detected - current %i, new %i", L_2SNZ_DAY.get(), mode);
        return true;
    }

    if (sendCommandCheckResult(String("W70:")+mode,String(mode))) {
        L_2SNZ_DAY.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setL_2SNZ_BGN(int mode){
    debugD("setL_2SNZ_BGN - %i",mode);
    if (mode == L_2SNZ_BGN.get()) {
        debugD("No L_2SNZ_BGN change detected - current %i, new %i", L_2SNZ_BGN.get(), mode);
        return true;
    }

    if (sendCommandCheckResult(String("W71:")+mode,String(mode))) {
        return true;
    }
    return false;
}

bool SpaInterface::setL_2SNZ_END(int mode){
    debugD("setL_2SNZ_END - %i",mode);
    if (mode == L_2SNZ_END.get()) {
        debugD("No L_2SNZ_END change detected - current %i, new %i", L_2SNZ_END.get(), mode);
        return true;
    }

    if (sendCommandCheckResult(String("W72:")+mode,String(mode))) {
        return true;
    }
    return false;
}

bool SpaInterface::setHPMP(int mode){
    // Internal writer for HPMP RWProperty.
    debugD("setHPMP - %i", mode);
    if (mode == HPMP.get()) {
        debugD("No HPMP change detected - current %i, new %i", HPMP.get(), mode);
        return true;
    }

    if (!validateHPMP(mode)) {
        debugD("Invalid HPMP mode %i", mode);
        return false;
    }

    String smode = String(mode);
    if (sendCommandCheckResult("W99:"+smode,smode)) {
        HPMP.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setColorMode(int mode){
    debugD("setColorMode - %i", mode);
    if (mode == ColorMode.get()) {
        debugD("No ColorMode change detected - current %i, new %i", ColorMode.get(), mode);
        return true;
    }

    if (!validateColorMode(mode)) {
        debugD("Invalid ColorMode %i", mode);
        return false;
    }

    String smode = String(mode);
    if (sendCommandCheckResult("S07:"+smode,smode)) {
        ColorMode.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setLBRTValue(int mode){
    debugD("setLBRTValue - %i", mode);
    if (mode == getLBRTValue()) {
        debugD("No LBRTValue change detected - current %i, new %i", getLBRTValue(), mode);
        return true;
    }

    String smode = String(mode);
    if (sendCommandCheckResult("S08:"+smode,smode)) {
        update_LBRTValue(smode);
        return true;
    }
    return false;
}

bool SpaInterface::setLSPDValue(int mode){
    debugD("setLSPDValue - %i", mode);
    if (mode == getLSPDValue()) {
        debugD("No LSPDValue change detected - current %i, new %i", getLSPDValue(), mode);
        return true;
    }

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
    if (mode == getCurrClr()) {
        debugD("No CurrClr change detected - current %i, new %i", getCurrClr(), mode);
        return true;
    }

    String smode = String(mode);
    if (sendCommandCheckResult("S10:"+smode,smode)) {
        update_CurrClr(smode);
        return true;
    }
    return false;
}

bool SpaInterface::setSpaDayOfWeek(int d){
    debugD("setSpaDayOfWeek - %i", d);
    if (d == getSpaDayOfWeek()) {
        debugD("No SpaDayOfWeek change detected - current %i, new %i", getSpaDayOfWeek(), d);
        return true;
    }

    String sd = String(d);
    if (sendCommandCheckResult("S06:"+sd,sd)) {
        update_SpaDayOfWeek(sd);
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
    delay(100);

    tmp = String(month(t));
    outcome = outcome && sendCommandCheckResult("S02:"+tmp,tmp);
    delay(100);

    tmp = String(day(t));
    outcome = outcome && sendCommandCheckResult("S03:"+tmp,tmp);
    delay(100);
    
    tmp = String(hour(t));
    outcome = outcome && sendCommandCheckResult("S04:"+tmp,tmp);
    delay(100);
    
    tmp = String(minute(t));
    outcome = outcome && sendCommandCheckResult("S05:"+tmp,tmp);
    delay(100);
    
    int weekDay = weekday(t); // day of the week (1-7), Sunday is day 1 (Arduino Time Library)
    // Convert to the format required by Spa: day of the week (0-6), Monday is day 0
    if (weekDay == 1) weekDay = 6;
    else weekDay -= 2;
    outcome = outcome && setSpaDayOfWeek(weekDay);
    
    return outcome;

}

bool SpaInterface::setOutlet_Blower(int mode){
    debugD("setOuput-Blower - %i", mode);
    if (mode == getOutlet_Blower()) {
        debugD("No Outlet_Blower change detected - current %i, new %i", getOutlet_Blower(), mode);
        return true;
    }

    String smode = String(mode);
    if (sendCommandCheckResult("S28:"+smode,"S28-OK")) {
        update_Outlet_Blower(smode);
        return true;
    }
    return false;
}

bool SpaInterface::setVARIValue(int mode){
    debugD("setVARIValue - %i", mode);
    if (mode == getVARIValue()) {
        debugD("No VARIValue change detected - current %i, new %i", getVARIValue(), mode);
        return true;
    }

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
    if (mode == getModeIndex(getMode())) {
        debugD("No Mode change detected - current %i, new %i", getModeIndex(getMode()), mode);
        return true;
    }
    
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

bool SpaInterface::setFiltBlockHrs(String duration){
    debugD("setFiltBlockHrs - %s", duration);
    for (int i = 0; i < FiltBlockHrsSelect.size(); i++) {
        if (FiltBlockHrsSelect[i] == duration) {
            if (sendCommandCheckResult("W90:"+FiltBlockHrsSelect[i],FiltBlockHrsSelect[i])) {
                update_FiltBlockHrs(FiltBlockHrsSelect[i]);
                return true;
            }
            return false;
        }
    }
    return false;
}

bool SpaInterface::setFiltHrs(String duration){
    debugD("setFiltHrs - %s", duration);
    int hrs = duration.toInt();
    if (hrs<1 or hrs>24) {
        debugE("FiltHrs out of range - %s", duration.c_str());
        return false;
    }
    if (sendCommandCheckResult("W60:" + String(hrs), String(hrs))) {
        update_FiltHrs(duration);
        return true;
    }
    return false;
}

bool SpaInterface::setLockMode(int mode){
    debugD("setLockMode - %i", mode);
    if (mode == getLockMode()) {
        debugD("No LockMode change detected - current %i, new %i", getLockMode(), mode);
        return true;
    }

    if (mode < 0 || mode > 2) {
        debugE("LockMode out of range - %i", mode);
        return false;
    }

    if (sendCommandCheckResult("S21:"+String(mode),String(mode))) {
        update_LockMode(String(mode));
        return true;
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
    int majorFirmwarwVersion = 0;

    if (_initialised) {
        uint spaceIndex = getSVER().indexOf(' ', 4);
        if (spaceIndex != -1) {
            majorFirmwarwVersion = getSVER().substring(4, spaceIndex).toInt(); // Skip the 'V' character
        }
        debugV("Firmware: %s, majorFirmwareVersion: %i", getSVER().c_str(), majorFirmwarwVersion);
    }

    // read the first field and validate the response
    statusResponseRaw[field] = port.readStringUntil(',');
    debugV("(%i,%s)",field,statusResponseRaw[field].c_str());
    if (field == 0 && !statusResponseRaw[field].startsWith("RF:")) { // If the first field is not "RF:" stop we don't have the start of the register
        debugE("Throwing exception - field: %i, value: %s", field, statusResponseRaw[field].c_str());
        return false;
    }
    statusResponseTmp = statusResponseRaw[field]+",";
    field++;

    bool lastByteWasColon = false;
    while (field < statusResponseMaxFields)
    {
        String registerData = "";
        bool isEndOfLine = false;
        bool isEndOfData = false;

        // This block is based on port.readStringUntil(',') but adds handling for ':' and '\n' characters
        char bytes[1];
        if (port.readBytes(bytes, 1) != 1) {
            bytes[0] = -1;
        }
        while(bytes[0] >= 0 && bytes[0] != ',') {
            if (lastByteWasColon) {
                registerData += ':'; // Add the colon to the buffer'
            }
            if (bytes[0] == ':' && registerData.length() > 0) {
                debugV("Read \":\", at end of field: %s, register number: %i, number: %i, total fields counted: %i, minimum fields: %i", statusResponseRaw[field-currentRegisterSize], field, registerCounter, currentRegisterSize, registerMinSize[registerCounter]);
                lastByteWasColon = true;
                break; // If we reach a colon and we have data in the buffer, we have reached the end of the current field
            } else {
                lastByteWasColon = false;
            }
            registerData += bytes[0]; // Append to buffer
            if (bytes[0] == '\n') {
                isEndOfLine = true;
                if (registerCounter >= 11 || (majorFirmwarwVersion < 3 && registerCounter >= 10)) {
                    debugV("Read \"\\n\", at end of final register: %s, register number: %i, number: %i, total fields counted: %i, minimum fields: %i", statusResponseRaw[field-currentRegisterSize], field, registerCounter, currentRegisterSize, registerMinSize[registerCounter]);
                    isEndOfData = true;
                    break; // If we reach the last register we have finished reading...
                }
            }
            if (port.readBytes(bytes, 1) != 1) {
                bytes[0] = -1;
            }
        }

        statusResponseRaw[field] = registerData;
        debugV("(%i,%s)",field,statusResponseRaw[field].c_str());

        // if we have reached an end of line, we are at the end of the current register
        if (isEndOfLine) {
            debugV("Completed reading register: %s, number: %i, total fields counted: %i, minimum fields: %i", statusResponseRaw[field-currentRegisterSize], registerCounter, currentRegisterSize, registerMinSize[registerCounter]);
            if (registerMinSize[registerCounter] > currentRegisterSize) {
                debugE("Throwing exception - not enough fields in register: %s number: %i, total fields counted: %i, minimum fields: %i", statusResponseRaw[field-currentRegisterSize], registerCounter, currentRegisterSize, registerMinSize[registerCounter]);
                registerError++; // Instead of returning false, I want to read the complete response so it is available in the webinterface for debugging
            }
            registerCounter++;
            currentRegisterSize = 0;
        } else {
            currentRegisterSize++;
        }

        if (isEndOfData) {
            debugD("Reached end of data");
            statusResponseTmp += statusResponseRaw[field];
            break;
        }

        if (bytes[0] == -1) {
            debugD("Reached end of stream");
            statusResponseTmp += statusResponseRaw[field];
            break;
        }

        statusResponseTmp = statusResponseTmp + statusResponseRaw[field]+",";

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

            if (registerCounter == 1 && R3 > 0 && currentRegisterSize == 7) {
                uint spaceIndex = statusResponseRaw[R3+6].indexOf(' ', 4);
                if (spaceIndex != -1) {
                    majorFirmwarwVersion = statusResponseRaw[R3+6].substring(4, spaceIndex).toInt(); // Skip the 'V' character
                }
                debugV("Firmware: %s, majorFirmwarwVersion: %i", statusResponseRaw[R3+6].c_str(), majorFirmwarwVersion);
            }
        }

        field++;
    }

    //Flush the remaining data from the buffer as the last field is meaningless
    statusResponseTmp = statusResponseTmp + flushSerialReadBuffer(true);

    statusResponse.update_Value(statusResponseTmp);

    if ((majorFirmwarwVersion > 2 && registerCounter < 12) || (majorFirmwarwVersion < 3 && registerCounter < 11)) {
        debugE("Throwing exception - not enough registers, we only read: %i", registerCounter);
        return false;
    }

    if (registerError > 0) {
        debugE("Throwing exception - not enough fields in %i registers", registerError);
        return false;
    }

    if ((majorFirmwarwVersion > 2 && field < statusResponseMinFields) || (majorFirmwarwVersion < 3 && field < statusResponseV2MinFields)) {
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
        debugV("Waiting...");
        _lastWaitMessage = millis();
    }

    if (_resultRegistersDirty) {
        _nextUpdateDue = millis() + 500;  // if we need to read the registers, pause a bit to see if there are more commands coming.
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
    MainsCurrent.update(statusResponseRaw[R2+1].toInt());
    update_MainsVoltage(statusResponseRaw[R2+2]);
    update_CaseTemperature(statusResponseRaw[R2+3]);
    update_PortCurrent(statusResponseRaw[R2+4]);
    update_SpaDayOfWeek(statusResponseRaw[R2+5]);
    update_SpaTime(statusResponseRaw[R2+11], statusResponseRaw[R2+10], statusResponseRaw[R2+9], statusResponseRaw[R2+6], statusResponseRaw[R2+7], statusResponseRaw[R2+8]);
    update_HeaterTemperature(statusResponseRaw[R2+12]);
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
    #pragma endregion
    #pragma region R3
    update_CLMT(statusResponseRaw[R3+1]);
    update_PHSE(statusResponseRaw[R3+2]);
    update_LLM1(statusResponseRaw[R3+3]);
    update_LLM2(statusResponseRaw[R3+4]);
    update_LLM3(statusResponseRaw[R3+5]);
    update_SVER(statusResponseRaw[R3+6]);
    update_Model(statusResponseRaw[R3+7]); 
    update_SerialNo1(statusResponseRaw[R3+8]);
    update_SerialNo2(statusResponseRaw[R3+9]); 
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
    update_Status(statusResponseRaw[R3+20]);
    update_PrimeCount(statusResponseRaw[R3+21]);
    update_EC(statusResponseRaw[R3+22]);
    update_HAMB(statusResponseRaw[R3+23]);
    update_HCON(statusResponseRaw[R3+24]);
    // update_HV_2(statusResponseRaw[R3+25]);
    #pragma endregion
    #pragma region R4
    update_Mode(statusResponseRaw[R4+1]);
    update_Ser1_Timer(statusResponseRaw[R4+2]);
    update_Ser2_Timer(statusResponseRaw[R4+3]);
    update_Ser3_Timer(statusResponseRaw[R4+4]);
    update_HeatMode(statusResponseRaw[R4+5]);
    update_PumpIdleTimer(statusResponseRaw[R4+6]);
    update_PumpRunTimer(statusResponseRaw[R4+7]);
    update_AdtPoolHys(statusResponseRaw[R4+8]);
    update_AdtHeaterHys(statusResponseRaw[R4+9]);
    update_Power(statusResponseRaw[R4+10]);
    update_Power_kWh(statusResponseRaw[R4+11]);
    update_Power_Today(statusResponseRaw[R4+12]);
    update_Power_Yesterday(statusResponseRaw[R4+13]);
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
    #pragma endregion
    #pragma region R5
    //R5
    // Unknown encoding - TouchPad2.updateValue();
    // Unknown encoding - TouchPad1.updateValue();
    //RB_TP_Blower.updateValue(statusResponseRaw[R5 + 5]);
    update_RB_TP_Sleep(statusResponseRaw[R5 + 10]);
    update_RB_TP_Ozone(statusResponseRaw[R5 + 11]);
    update_RB_TP_Heater(statusResponseRaw[R5 + 12]);
    update_RB_TP_Auto(statusResponseRaw[R5 + 13]);
    update_RB_TP_Light(statusResponseRaw[R5 + 14]);
    update_WTMP(statusResponseRaw[R5 + 15]);
    update_CleanCycle(statusResponseRaw[R5 + 16]);
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
    ColorMode.update(statusResponseRaw[R6 + 4].toInt());
    update_LSPDValue(statusResponseRaw[R6 + 5]);
    update_FiltHrs(statusResponseRaw[R6 + 6]);
    update_FiltBlockHrs(statusResponseRaw[R6 + 7]);
    STMP.update(statusResponseRaw[R6 + 8].toInt());
    update_L_24HOURS(statusResponseRaw[R6 + 9]);
    update_PSAV_LVL(statusResponseRaw[R6 + 10]);
    update_PSAV_BGN(statusResponseRaw[R6 + 11]);
    update_PSAV_END(statusResponseRaw[R6 + 12]);
    L_1SNZ_DAY.update(statusResponseRaw[R6 + 13].toInt());
    L_2SNZ_DAY.update(statusResponseRaw[R6 + 14].toInt());
    L_1SNZ_BGN.update(statusResponseRaw[R6 + 15].toInt());
    L_2SNZ_BGN.update(statusResponseRaw[R6 + 16].toInt());
    L_1SNZ_END.update(statusResponseRaw[R6 + 17].toInt());
    L_2SNZ_END.update(statusResponseRaw[R6 + 18].toInt());
    update_DefaultScrn(statusResponseRaw[R6 + 19]);
    update_TOUT(statusResponseRaw[R6 + 20]);
    update_VPMP(statusResponseRaw[R6 + 21]);
    update_HIFI(statusResponseRaw[R6 + 22]);
    update_BRND(statusResponseRaw[R6 + 23]);
    // Note: We only have 23 registers in V2 firmware
    if (R6 > 23) {
        update_PRME(statusResponseRaw[R6 + 24]);
        update_ELMT(statusResponseRaw[R6 + 25]);
        update_TYPE(statusResponseRaw[R6 + 26]);
        update_GAS(statusResponseRaw[R6 + 27]);
    }
    #pragma endregion
    #pragma region R7
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
    update_HELE(statusResponseRaw[R7 + 25]);
    HPMP.update(statusResponseRaw[R7 + 26].toInt());
    update_PMIN(statusResponseRaw[R7 + 27]);
    update_PFLT(statusResponseRaw[R7 + 28]);
    update_PHTR(statusResponseRaw[R7 + 29]);
    update_PMAX(statusResponseRaw[R7 + 30]);
    #pragma endregion
    #pragma region R9
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
    #pragma endregion
    #pragma region RA
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
    #pragma endregion
    #pragma region RB
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
    #pragma endregion
    #pragma region RC
    //Outlet_Heater.updateValue(statusResponseRaw[]);
    //Outlet_Circ.updateValue(statusResponseRaw[]);
    //Outlet_Sanitise.updateValue(statusResponseRaw[]);
    //Outlet_Pump1.updateValue(statusResponseRaw[]);
    //Outlet_Pump2.updateValue(statusResponseRaw[]);
    //Outlet_Pump4.updateValue(statusResponseRaw[]);
    //Outlet_Pump5.updateValue(statusResponseRaw[]);
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
    #pragma endregion

    // There is no RG register in V2 firmware
    if (RG < 0) return;

    #pragma region RG
    update_Pump1InstallState(statusResponseRaw[RG + 7]);
    update_Pump2InstallState(statusResponseRaw[RG + 8]);
    update_Pump3InstallState(statusResponseRaw[RG + 9]);
    update_Pump4InstallState(statusResponseRaw[RG + 10]);
    update_Pump5InstallState(statusResponseRaw[RG + 11]);
    update_Pump1OkToRun(statusResponseRaw[RG + 1]);
    update_Pump2OkToRun(statusResponseRaw[RG + 2]);
    update_Pump3OkToRun(statusResponseRaw[RG + 3]);
    update_Pump4OkToRun(statusResponseRaw[RG + 4]);
    update_Pump5OkToRun(statusResponseRaw[RG + 5]);
    update_LockMode(statusResponseRaw[RG + 12]);
    #pragma endregion

};
