#include "SpaInterface.h"

#define BAUD_RATE 38400

SpaInterface* SpaInterface::_instance = nullptr;

SpaInterface::SpaInterface() : port(SPA_SERIAL) {
    SPA_SERIAL.setRxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.setTxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
    SPA_SERIAL.setTimeout(250);

    _instance = this;
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

void SpaInterface::_processDebugCommand() {
    if (_instance == nullptr) return;

    String cmd = Debug.getLastCommand();
    if (!cmd.startsWith("ss ") && !cmd.startsWith("SS ")) return;

    String payload = cmd.substring(3);
    debugI("TX: %s", payload.c_str());

    _instance->flushSerialReadBuffer();
    _instance->port.print('\n');
    _instance->port.flush();
    delay(50);
    _instance->port.printf("%s\n", payload.c_str());
    _instance->port.flush();

    // Wait up to 2s for first byte, then collect until 500ms gap
    String response = "";
    unsigned long start = millis();
    while (!_instance->port.available() && millis() - start < 2000) {}
    if (_instance->port.available()) {
        unsigned long lastByte = millis();
        while (millis() - lastByte < 500) {
            while (_instance->port.available()) {
                response += (char)_instance->port.read();
                lastByte = millis();
            }
        }
    }

    if (response.length() > 0) {
        debugI("RX:\n%s", response.c_str());
    } else {
        debugI("RX: (no response)");
    }

    _instance->_resultRegistersDirty = true;
}

bool SpaInterface::setRB_TP_Pump1(int mode){
    debugD("setRB_TP_Pump1 - %i", mode);
    if (mode < 0 || mode > 4) {
        throw std::out_of_range("RB_TP_Pump1 value out of range (0..4)");
    }
    if (mode == RB_TP_Pump1.get()) {
        debugD("No Pump1 change detected - current %i, new %i", RB_TP_Pump1.get(), mode);
        return true;
    }

    if (sendCommandCheckResult("S22:"+String(mode),"S22-OK")) {
        RB_TP_Pump1.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump2(int mode){
    debugD("setRB_TP_Pump2 - %i", mode);
    if (mode < 0 || mode > 4) {
        throw std::out_of_range("RB_TP_Pump2 value out of range (0..4)");
    }
    if (mode == RB_TP_Pump2.get()) {
        debugD("No Pump2 change detected - current %i, new %i", RB_TP_Pump2.get(), mode);
        return true;
    }

    if (sendCommandCheckResult("S23:"+String(mode),"S23-OK")) {
        RB_TP_Pump2.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump3(int mode){
    debugD("setRB_TP_Pump3 - %i", mode);
    if (mode < 0 || mode > 4) {
        throw std::out_of_range("RB_TP_Pump3 value out of range (0..4)");
    }
    if (mode == RB_TP_Pump3.get()) {
        debugD("No Pump3 change detected - current %i, new %i", RB_TP_Pump3.get(), mode);
        return true;
    }

    if (sendCommandCheckResult("S24:"+String(mode),"S24-OK")) {
        RB_TP_Pump3.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump4(int mode){
    debugD("setRB_TP_Pump4 - %i", mode);
    if (mode < 0 || mode > 4) {
        throw std::out_of_range("RB_TP_Pump4 value out of range (0..4)");
    }
    if (mode == RB_TP_Pump4.get()) {
        debugD("No Pump4 change detected - current %i, new %i", RB_TP_Pump4.get(), mode);
        return true;
    }

    if (sendCommandCheckResult("S25:"+String(mode),"S25-OK")) {
        RB_TP_Pump4.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setRB_TP_Pump5(int mode){
    debugD("setRB_TP_Pump5 - %i", mode);
    if (mode < 0 || mode > 4) {
        throw std::out_of_range("RB_TP_Pump5 value out of range (0..4)");
    }
    if (mode == RB_TP_Pump5.get()) {
        debugD("No Pump5 change detected - current %i, new %i", RB_TP_Pump5.get(), mode);
        return true;
    }

    if (sendCommandCheckResult("S26:"+String(mode),"S26-OK")) {
        RB_TP_Pump5.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setStatusResponse(String s) {
    return true;
}

bool SpaInterface::setRB_TP_Light(int mode){
    debugD("setRB_TP_Light - %i", mode);
    if (mode < 0 || mode > 1) throw std::out_of_range("RB_TP_Light value out of range (0..1)");
    if (mode == RB_TP_Light.get()) {
        debugD("No RB_TP_Light change detected - current %i, new %i", RB_TP_Light.get(), mode);
        return true;
    }
    if (sendCommandCheckResult("W14", "W14")) {
        RB_TP_Light.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setHELE(bool mode){
    debugD("setHELE - %i", mode);
    if (mode == HELE.get()) {
        debugD("No HELE change detected - current %i, new %i", HELE.get(), mode);
        return true;
    }

    int v = mode ? 1 : 0;
    if (sendCommandCheckResult("W98:"+String(v),String(v))) {
        HELE.update(mode);
        return true;
    }
    return false;
}


bool SpaInterface::sendKey(SpaKey key) {
    String cmd;
    String expected;
    switch (key) {
        case SpaKey::Up:     cmd = "W08"; expected = "W8";  break;
        case SpaKey::Ok:     cmd = "W09"; expected = "W9";  break;
        case SpaKey::Down:   cmd = "W10"; expected = "W10"; break;
        case SpaKey::Invert: cmd = "W11"; expected = "W11"; break;
        default: return false;
    }
    return sendCommandCheckResult(cmd, expected);
}

/// @brief Set the water temperature set point * 10 (380 = 38.0)
/// @param temp
/// @return
bool SpaInterface::setSTMP(int temp){
    debugD("setSTMP - %i", temp);

    if (temp < 50 || temp > 410) {
        throw std::out_of_range("STMP value out of range (50..410)");
    }

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

bool SpaInterface::setL_1SNZ_DAY(int mode){
    debugD("setL_1SNZ_DAY - %i",mode);
    bool validMode = false;
    for (size_t i = 0; i < array_count(SNZ_DAY_Map); i++) {
        if (SNZ_DAY_Map[i].value == mode) {
            validMode = true;
            break;
        }
    }
    if (!validMode) {
        throw std::out_of_range("L_1SNZ_DAY value out of range");
    }

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
    if (mode < 0) {
        throw std::out_of_range("L_1SNZ_BGN value out of range");
    }
    int hour = mode / 256;
    int minute = mode % 256;
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        throw std::out_of_range("L_1SNZ_BGN value out of range");
    }

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
    if (mode < 0) {
        throw std::out_of_range("L_1SNZ_END value out of range");
    }
    int hour = mode / 256;
    int minute = mode % 256;
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        throw std::out_of_range("L_1SNZ_END value out of range");
    }

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
    bool validMode = false;
    for (size_t i = 0; i < array_count(SNZ_DAY_Map); i++) {
        if (SNZ_DAY_Map[i].value == mode) {
            validMode = true;
            break;
        }
    }
    if (!validMode) {
        throw std::out_of_range("L_2SNZ_DAY value out of range");
    }

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
    if (mode < 0) {
        throw std::out_of_range("L_2SNZ_BGN value out of range");
    }
    int hour = mode / 256;
    int minute = mode % 256;
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        throw std::out_of_range("L_2SNZ_BGN value out of range");
    }

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
    if (mode < 0) {
        throw std::out_of_range("L_2SNZ_END value out of range");
    }
    int hour = mode / 256;
    int minute = mode % 256;
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        throw std::out_of_range("L_2SNZ_END value out of range");
    }

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
    if (mode < 0 || mode > 3) {
        throw std::out_of_range("HPMP value out of range (0..3)");
    }

    if (mode == HPMP.get()) {
        debugD("No HPMP change detected - current %i, new %i", HPMP.get(), mode);
        return true;
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
    if (mode < 0 || mode > 4) {
        throw std::out_of_range("ColorMode value out of range (0..4)");
    }

    if (mode == ColorMode.get()) {
        debugD("No ColorMode change detected - current %i, new %i", ColorMode.get(), mode);
        return true;
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
    if (mode < 1 || mode > 5) {
        throw std::out_of_range("LBRTValue value out of range (1..5)");
    }

    if (mode == LBRTValue.get()) {
        debugD("No LBRTValue change detected - current %i, new %i", LBRTValue.get(), mode);
        return true;
    }

    String smode = String(mode);
    if (sendCommandCheckResult("S08:"+smode,smode)) {
        LBRTValue.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setLSPDValue(int mode){
    debugD("setLSPDValue - %i", mode);
    if (mode < 1 || mode > 5) {
        throw std::out_of_range("LSPDValue value out of range (1..5)");
    }

    if (mode == LSPDValue.get()) {
        debugD("No LSPDValue change detected - current %i, new %i", LSPDValue.get(), mode);
        return true;
    }

    String smode = String(mode);
    if (sendCommandCheckResult("S09:"+smode,smode)) {
        LSPDValue.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setCurrClr(int mode){
    debugD("setCurrClr - %i", mode);
    if (mode < 0 || mode > 31) {
        throw std::out_of_range("CurrClr value out of range (0..31)");
    }

    if (mode == CurrClr.get()) {
        debugD("No CurrClr change detected - current %i, new %i", CurrClr.get(), mode);
        return true;
    }

    String smode = String(mode);
    if (sendCommandCheckResult("S10:"+smode,smode)) {
        CurrClr.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setSpaDayOfWeek(int d){
    debugD("setSpaDayOfWeek - %i", d);
    if (d < 0 || d > 6) {
        throw std::out_of_range("SpaDayOfWeek value out of range (0..6)");
    }
    if (d == SpaDayOfWeek.get()) {
        debugD("No SpaDayOfWeek change detected - current %i, new %i", SpaDayOfWeek.get(), d);
        return true;
    }

    String sd = String(d);
    if (sendCommandCheckResult("S06:"+sd,sd)) {
        SpaDayOfWeek.update(d);
        return true;
    }
    return false;
}

bool SpaInterface::setSpaTime(time_t t){
    debugD("setSpaTime");

    String tmp;
    bool outcome;

    tmp = String(year(t) % 100);
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

    if (outcome) SpaTime.update(t);
    return outcome;
}

bool SpaInterface::setOutlet_Blower(int mode){
    debugD("setOutlet_Blower - %i", mode);
    if (mode < 0 || mode > 2) {
        throw std::out_of_range("Outlet_Blower value out of range (0..2)");
    }
    if (mode == Outlet_Blower.get()) {
        debugD("No Outlet_Blower change detected - current %i, new %i", Outlet_Blower.get(), mode);
        return true;
    }

    String smode = String(mode);
    if (sendCommandCheckResult("S28:"+smode,"S28-OK")) {
        Outlet_Blower.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setVARIValue(int mode){
    debugD("setVARIValue - %i", mode);
    if (mode < 1 || mode > 5) {
        throw std::out_of_range("VARIValue out of range (1..5)");
    }
    if (mode == VARIValue.get()) {
        debugD("No VARIValue change detected - current %i, new %i", VARIValue.get(), mode);
        return true;
    }

    String smode = String(mode);
    if (sendCommandCheckResult("S13:"+smode,smode+"  S13")) {
        VARIValue.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setMode(int mode){
    debugD("setMode - %i", mode);
    if (mode < 0 || mode > 3) {
        throw std::out_of_range("Mode value out of range (0..3)");
    }
    if (mode == Mode.get()) {
        debugD("No Mode change detected - current %i, new %i", Mode.get(), mode);
        return true;
    }

    String smode = String(mode);
    if (sendCommandCheckResult("W66:"+smode,smode)) {
        Mode.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setFiltBlockHrs(int mode){
    debugD("setFiltBlockHrs - %i", mode);
    static const int valid[] = {1, 2, 3, 4, 6, 8, 12, 24};
    bool isValid = false;
    for (int v : valid) {
        if (mode == v) { isValid = true; break; }
    }
    if (!isValid) {
        throw std::out_of_range("FiltBlockHrs value not in valid set (1,2,3,4,6,8,12,24)");
    }
    if (mode == FiltBlockHrs.get()) {
        debugD("No FiltBlockHrs change detected - current %i, new %i", FiltBlockHrs.get(), mode);
        return true;
    }
    if (sendCommandCheckResult("W90:"+String(mode), String(mode))) {
        FiltBlockHrs.update(mode);
        return true;
    }
    return false;
}

bool SpaInterface::setFiltHrs(int hrs){
    debugD("setFiltHrs - %i", hrs);
    if (hrs < 1 || hrs > 24) {
        throw std::out_of_range("FiltHrs value out of range (1..24)");
    }
    if (hrs == FiltHrs.get()) {
        debugD("No FiltHrs change detected - current %i, new %i", FiltHrs.get(), hrs);
        return true;
    }
    if (sendCommandCheckResult("W60:" + String(hrs), String(hrs))) {
        FiltHrs.update(hrs);
        return true;
    }
    return false;
}

bool SpaInterface::setLockMode(int mode){
    debugD("setLockMode - %i", mode);
    if (mode < 0 || mode > 2) {
        throw std::out_of_range("LockMode value out of range (0..2)");
    }
    if (mode == LockMode.get()) {
        debugD("No LockMode change detected - current %i, new %i", LockMode.get(), mode);
        return true;
    }
    if (sendCommandCheckResult("S21:"+String(mode), String(mode))) {
        LockMode.update(mode);
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
        uint spaceIndex = SVER.get().indexOf(' ', 4);
        if (spaceIndex != -1) {
            majorFirmwarwVersion = SVER.get().substring(4, spaceIndex).toInt(); // Skip the 'V' character
        }
        debugV("Firmware: %s, majorFirmwareVersion: %i", SVER.get().c_str(), majorFirmwarwVersion);
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

    statusResponse.update(statusResponseTmp);

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
    if (!_debugInitialised) {
        Debug.setHelpProjectsCmds("ss <cmd> - Send raw command to spa serial and print response");
        Debug.setCallBackProjectCmds(&SpaInterface::_processDebugCommand);
        _debugInitialised = true;
    }

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
    MainsVoltage.update(statusResponseRaw[R2+2].toInt());
    CaseTemperature.update(statusResponseRaw[R2+3].toInt());
    PortCurrent.update(statusResponseRaw[R2+4].toInt());
    SpaDayOfWeek.update(statusResponseRaw[R2+5].toInt());
    {
        tmElements_t tm;
        tm.Year   = CalendarYrToTm(statusResponseRaw[R2+11].toInt());
        tm.Month  = statusResponseRaw[R2+10].toInt();
        tm.Day    = statusResponseRaw[R2+9].toInt();
        tm.Hour   = statusResponseRaw[R2+6].toInt();
        tm.Minute = statusResponseRaw[R2+7].toInt();
        tm.Second = statusResponseRaw[R2+8].toInt();
        SpaTime.update(makeTime(tm));
    }
    HeaterTemperature.update(statusResponseRaw[R2+12].toInt());
    PoolTemperature.update(statusResponseRaw[R2+13].toInt());
    WaterPresent.update(statusResponseRaw[R2+14] == "1");
    AwakeMinutesRemaining.update(statusResponseRaw[R2+16].toInt());
    FiltPumpRunTimeTotal.update(statusResponseRaw[R2+17].toInt());
    FiltPumpReqMins.update(statusResponseRaw[R2+18].toInt());
    LoadTimeOut.update(statusResponseRaw[R2+19].toInt());
    HourMeter.update(statusResponseRaw[R2+20].toInt());
    Relay1.update(statusResponseRaw[R2+21].toInt());
    Relay2.update(statusResponseRaw[R2+22].toInt());
    Relay3.update(statusResponseRaw[R2+23].toInt());
    Relay4.update(statusResponseRaw[R2+24].toInt());
    Relay5.update(statusResponseRaw[R2+25].toInt());
    Relay6.update(statusResponseRaw[R2+26].toInt());
    Relay7.update(statusResponseRaw[R2+27].toInt());
    Relay8.update(statusResponseRaw[R2+28].toInt());
    Relay9.update(statusResponseRaw[R2+29].toInt());
    #pragma endregion
    #pragma region R3
    CLMT.update(statusResponseRaw[R3+1].toInt());
    PHSE.update(statusResponseRaw[R3+2].toInt());
    LLM1.update(statusResponseRaw[R3+3].toInt());
    LLM2.update(statusResponseRaw[R3+4].toInt());
    LLM3.update(statusResponseRaw[R3+5].toInt());
    SVER.update(statusResponseRaw[R3+6]);
    Model.update(statusResponseRaw[R3+7]);
    SerialNo1.update(statusResponseRaw[R3+8]);
    SerialNo2.update(statusResponseRaw[R3+9]);
    D1.update(statusResponseRaw[R3+10] == "1");
    D2.update(statusResponseRaw[R3+11] == "1");
    D3.update(statusResponseRaw[R3+12] == "1");
    D4.update(statusResponseRaw[R3+13] == "1");
    D5.update(statusResponseRaw[R3+14] == "1");
    D6.update(statusResponseRaw[R3+15] == "1");
    Pump.update(statusResponseRaw[R3+16]);
    LS.update(statusResponseRaw[R3+17].toInt());
    HV.update(statusResponseRaw[R3+18] == "1");
    SnpMR.update(statusResponseRaw[R3+19].toInt());
    Status.update(statusResponseRaw[R3+20]);
    PrimeCount.update(statusResponseRaw[R3+21].toInt());
    EC.update(statusResponseRaw[R3+22].toInt());
    HAMB.update(statusResponseRaw[R3+23].toInt());
    HCON.update(statusResponseRaw[R3+24].toInt());
    // update_HV_2(statusResponseRaw[R3+25]);
    #pragma endregion
    #pragma region R4
    try { Mode.updateFromLabel(statusResponseRaw[R4+1].c_str()); } catch (const std::exception& ex) { debugE("Mode update failed: %s", ex.what()); }
    Ser1_Timer.update(statusResponseRaw[R4+2].toInt());
    Ser2_Timer.update(statusResponseRaw[R4+3].toInt());
    Ser3_Timer.update(statusResponseRaw[R4+4].toInt());
    HeatMode.update(statusResponseRaw[R4+5].toInt());
    PumpIdleTimer.update(statusResponseRaw[R4+6].toInt());
    PumpRunTimer.update(statusResponseRaw[R4+7].toInt());
    AdtPoolHys.update(statusResponseRaw[R4+8].toInt());
    AdtHeaterHys.update(statusResponseRaw[R4+9].toInt());
    Power.update(statusResponseRaw[R4+10].toInt());
    Power_kWh.update(statusResponseRaw[R4+11].toInt());
    Power_Today.update(statusResponseRaw[R4+12].toInt());
    Power_Yesterday.update(statusResponseRaw[R4+13].toInt());
    ThermalCutOut.update(statusResponseRaw[R4+14].toInt());
    Test_D1.update(statusResponseRaw[R4+15].toInt());
    Test_D2.update(statusResponseRaw[R4+16].toInt());
    Test_D3.update(statusResponseRaw[R4+17].toInt());
    ElementHeatSourceOffset.update(statusResponseRaw[R4+18].toInt());
    Frequency.update(statusResponseRaw[R4+19].toInt());
    HPHeatSourceOffset_Heat.update(statusResponseRaw[R4+20].toInt());
    HPHeatSourceOffset_Cool.update(statusResponseRaw[R4+21].toInt());
    HeatSourceOffTime.update(statusResponseRaw[R4+22].toInt());
    Vari_Speed.update(statusResponseRaw[R4+24].toInt());
    Vari_Percent.update(statusResponseRaw[R4+25].toInt());
    Vari_Mode.update(statusResponseRaw[R4+23].toInt());
    #pragma endregion
    #pragma region R5
    //R5
    // Unknown encoding - TouchPad2.updateValue();
    // Unknown encoding - TouchPad1.updateValue();
    //RB_TP_Blower.updateValue(statusResponseRaw[R5 + 5]);
    RB_TP_Sleep.update(statusResponseRaw[R5 + 10] == "1");
    RB_TP_Ozone.update(statusResponseRaw[R5 + 11] == "1");
    RB_TP_Heater.update(statusResponseRaw[R5 + 12] == "1");
    RB_TP_Auto.update(statusResponseRaw[R5 + 13] == "1");
    RB_TP_Light.update(statusResponseRaw[R5 + 14].toInt());
    WTMP.update(statusResponseRaw[R5 + 15].toInt());
    CleanCycle.update(statusResponseRaw[R5 + 16] == "1");
    RB_TP_Pump1.update(statusResponseRaw[R5 + 18].toInt());
    RB_TP_Pump2.update(statusResponseRaw[R5 + 19].toInt());
    RB_TP_Pump3.update(statusResponseRaw[R5 + 20].toInt());
    RB_TP_Pump4.update(statusResponseRaw[R5 + 21].toInt());
    RB_TP_Pump5.update(statusResponseRaw[R5 + 22].toInt());
    #pragma endregion
    #pragma region R6
    VARIValue.update(statusResponseRaw[R6 + 1].toInt());
    LBRTValue.update(statusResponseRaw[R6 + 2].toInt());
    CurrClr.update(statusResponseRaw[R6 + 3].toInt());
    ColorMode.update(statusResponseRaw[R6 + 4].toInt());
    LSPDValue.update(statusResponseRaw[R6 + 5].toInt());
    FiltHrs.update(statusResponseRaw[R6 + 6].toInt());
    FiltBlockHrs.update(statusResponseRaw[R6 + 7].toInt());
    STMP.update(statusResponseRaw[R6 + 8].toInt());
    L_24HOURS.update(statusResponseRaw[R6 + 9].toInt());
    PSAV_LVL.update(statusResponseRaw[R6 + 10].toInt());
    PSAV_BGN.update(statusResponseRaw[R6 + 11].toInt());
    PSAV_END.update(statusResponseRaw[R6 + 12].toInt());
    L_1SNZ_DAY.update(statusResponseRaw[R6 + 13].toInt());
    L_2SNZ_DAY.update(statusResponseRaw[R6 + 14].toInt());
    L_1SNZ_BGN.update(statusResponseRaw[R6 + 15].toInt());
    L_2SNZ_BGN.update(statusResponseRaw[R6 + 16].toInt());
    L_1SNZ_END.update(statusResponseRaw[R6 + 17].toInt());
    L_2SNZ_END.update(statusResponseRaw[R6 + 18].toInt());
    DefaultScrn.update(statusResponseRaw[R6 + 19].toInt());
    TOUT.update(statusResponseRaw[R6 + 20].toInt());
    VPMP.update(statusResponseRaw[R6 + 21] == "1");
    HIFI.update(statusResponseRaw[R6 + 22] == "1");
    BRND.update(statusResponseRaw[R6 + 23].toInt());
    // Note: We only have 23 registers in V2 firmware
    if (R6 > 23) {
        PRME.update(statusResponseRaw[R6 + 24].toInt());
        ELMT.update(statusResponseRaw[R6 + 25].toInt());
        TYPE.update(statusResponseRaw[R6 + 26].toInt());
        GAS.update(statusResponseRaw[R6 + 27].toInt());
    }
    #pragma endregion
    #pragma region R7
    WCLNTime.update(statusResponseRaw[R7 + 1].toInt());
    // The following 2 may be reversed
    TemperatureUnits.update(statusResponseRaw[R7 + 3] == "1");
    OzoneOff.update(statusResponseRaw[R7 + 2] == "1");
    Ozone24.update(statusResponseRaw[R7 + 4] == "1");
    Circ24.update(statusResponseRaw[R7 + 6] == "1");
    CJET.update(statusResponseRaw[R7 + 5] == "1");
    // 0 = off, 1 = step, 2 = variable
    VELE.update(statusResponseRaw[R7 + 7] == "1");
    //update_StartDD(statusResponseRaw[R7 + 8]);
    //update_StartMM(statusResponseRaw[R7 + 9]);
    //update_StartYY(statusResponseRaw[R7 + 10]);
    V_Max.update(statusResponseRaw[R7 + 11].toInt());
    V_Min.update(statusResponseRaw[R7 + 12].toInt());
    V_Max_24.update(statusResponseRaw[R7 + 13].toInt());
    V_Min_24.update(statusResponseRaw[R7 + 14].toInt());
    CurrentZero.update(statusResponseRaw[R7 + 15].toInt());
    CurrentAdjust.update(statusResponseRaw[R7 + 16].toInt());
    VoltageAdjust.update(statusResponseRaw[R7 + 17].toInt());
    // 168 is unknown
    Ser1.update(statusResponseRaw[R7 + 19].toInt());
    Ser2.update(statusResponseRaw[R7 + 20].toInt());
    Ser3.update(statusResponseRaw[R7 + 21].toInt());
    VMAX.update(statusResponseRaw[R7 + 22].toInt());
    AHYS.update(statusResponseRaw[R7 + 23].toInt());
    HUSE.update(statusResponseRaw[R7 + 24] == "1");
    HELE.update(statusResponseRaw[R7 + 25] == "1");
    HPMP.update(statusResponseRaw[R7 + 26].toInt());
    PMIN.update(statusResponseRaw[R7 + 27].toInt());
    PFLT.update(statusResponseRaw[R7 + 28].toInt());
    PHTR.update(statusResponseRaw[R7 + 29].toInt());
    PMAX.update(statusResponseRaw[R7 + 30].toInt());
    #pragma endregion
    #pragma region R9
    F1_HR.update(statusResponseRaw[R9 + 2].toInt());
    F1_Time.update(statusResponseRaw[R9 + 3].toInt());
    F1_ER.update(statusResponseRaw[R9 + 4].toInt());
    F1_I.update(statusResponseRaw[R9 + 5].toInt());
    F1_V.update(statusResponseRaw[R9 + 6].toInt());
    F1_PT.update(statusResponseRaw[R9 + 7].toInt());
    F1_HT.update(statusResponseRaw[R9 + 8].toInt());
    F1_CT.update(statusResponseRaw[R9 + 9].toInt());
    F1_PU.update(statusResponseRaw[R9 + 10].toInt());
    F1_VE.update(statusResponseRaw[R9 + 11] == "1");
    F1_ST.update(statusResponseRaw[R9 + 12].toInt());
    #pragma endregion
    #pragma region RA
    F2_HR.update(statusResponseRaw[RA + 2].toInt());
    F2_Time.update(statusResponseRaw[RA + 3].toInt());
    F2_ER.update(statusResponseRaw[RA + 4].toInt());
    F2_I.update(statusResponseRaw[RA + 5].toInt());
    F2_V.update(statusResponseRaw[RA + 6].toInt());
    F2_PT.update(statusResponseRaw[RA + 7].toInt());
    F2_HT.update(statusResponseRaw[RA + 8].toInt());
    F2_CT.update(statusResponseRaw[RA + 9].toInt());
    F2_PU.update(statusResponseRaw[RA + 10].toInt());
    F2_VE.update(statusResponseRaw[RA + 11] == "1");
    F2_ST.update(statusResponseRaw[RA + 12].toInt());
    #pragma endregion
    #pragma region RB
    F3_HR.update(statusResponseRaw[RB + 2].toInt());
    F3_Time.update(statusResponseRaw[RB + 3].toInt());
    F3_ER.update(statusResponseRaw[RB + 4].toInt());
    F3_I.update(statusResponseRaw[RB + 5].toInt());
    F3_V.update(statusResponseRaw[RB + 6].toInt());
    F3_PT.update(statusResponseRaw[RB + 7].toInt());
    F3_HT.update(statusResponseRaw[RB + 8].toInt());
    F3_CT.update(statusResponseRaw[RB + 9].toInt());
    F3_PU.update(statusResponseRaw[RB + 10].toInt());
    F3_VE.update(statusResponseRaw[RB + 11] == "1");
    F3_ST.update(statusResponseRaw[RB + 12].toInt());
    #pragma endregion
    #pragma region RC
    //Outlet_Heater.updateValue(statusResponseRaw[]);
    //Outlet_Circ.updateValue(statusResponseRaw[]);
    //Outlet_Sanitise.updateValue(statusResponseRaw[]);
    //Outlet_Pump1.updateValue(statusResponseRaw[]);
    //Outlet_Pump2.updateValue(statusResponseRaw[]);
    //Outlet_Pump4.updateValue(statusResponseRaw[]);
    //Outlet_Pump5.updateValue(statusResponseRaw[]);
    Outlet_Blower.update(statusResponseRaw[RC + 10].toInt());
    #pragma endregion
    #pragma region RE
    HP_Present.update(statusResponseRaw[RE + 1].toInt());
    //HP_FlowSwitch.updateValue(statusResponseRaw[]);
    //HP_HighSwitch.updateValue(statusResponseRaw[]);
    //HP_LowSwitch.updateValue(statusResponseRaw[]);
    //HP_CompCutOut.updateValue(statusResponseRaw[]);
    //HP_ExCutOut.updateValue(statusResponseRaw[]);
    //HP_D1.updateValue(statusResponseRaw[]);
    //HP_D2.updateValue(statusResponseRaw[]);
    //HP_D3.updateValue(statusResponseRaw[]);
    HP_Ambient.update(statusResponseRaw[RE + 10].toInt());
    HP_Condensor.update(statusResponseRaw[RE + 11].toInt());
    HP_Compressor_State.update(statusResponseRaw[RE + 12] == "1");
    HP_Fan_State.update(statusResponseRaw[RE + 13] == "1");
    HP_4W_Valve.update(statusResponseRaw[RE + 14] == "1");
    HP_Heater_State.update(statusResponseRaw[RE + 15] == "1");
    HP_State.update(statusResponseRaw[RE + 16].toInt());
    HP_Mode.update(statusResponseRaw[RE + 17].toInt());
    HP_Defrost_Timer.update(statusResponseRaw[RE + 18].toInt());
    HP_Comp_Run_Timer.update(statusResponseRaw[RE + 19].toInt());
    HP_Low_Temp_Timer.update(statusResponseRaw[RE + 20].toInt());
    HP_Heat_Accum_Timer.update(statusResponseRaw[RE + 21].toInt());
    HP_Sequence_Timer.update(statusResponseRaw[RE + 22].toInt());
    HP_Warning.update(statusResponseRaw[RE + 23].toInt());
    FrezTmr.update(statusResponseRaw[RE + 24].toInt());
    DBGN.update(statusResponseRaw[RE + 25].toInt());
    DEND.update(statusResponseRaw[RE + 26].toInt());
    DCMP.update(statusResponseRaw[RE + 27].toInt());
    DMAX.update(statusResponseRaw[RE + 28].toInt());
    DELE.update(statusResponseRaw[RE + 29].toInt());
    DPMP.update(statusResponseRaw[RE + 30].toInt());
    //CMAX.updateValue(statusResponseRaw[]);
    //HP_Compressor.updateValue(statusResponseRaw[]);
    //HP_Pump_State.updateValue(statusResponseRaw[]);
    //HP_Status.updateValue(statusResponseRaw[]);
    #pragma endregion

    // There is no RG register in V2 firmware
    if (RG < 0) return;

    #pragma region RG
    Pump1InstallState.update(statusResponseRaw[RG + 7]);
    Pump2InstallState.update(statusResponseRaw[RG + 8]);
    Pump3InstallState.update(statusResponseRaw[RG + 9]);
    Pump4InstallState.update(statusResponseRaw[RG + 10]);
    Pump5InstallState.update(statusResponseRaw[RG + 11]);
    Pump1OkToRun.update(statusResponseRaw[RG + 1] == "1");
    Pump2OkToRun.update(statusResponseRaw[RG + 2] == "1");
    Pump3OkToRun.update(statusResponseRaw[RG + 3] == "1");
    Pump4OkToRun.update(statusResponseRaw[RG + 4] == "1");
    Pump5OkToRun.update(statusResponseRaw[RG + 5] == "1");
    LockMode.update(statusResponseRaw[RG + 12].toInt());
    #pragma endregion

};
