#include "SpaInterface.h"

#define BAUD_RATE 38400

SpaInterface::SpaInterface() : port(SPA_SERIAL) {
    SPA_SERIAL.setRxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.setTxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
    SPA_SERIAL.setTimeout(250);

    SpaInterface* mySpa = this;
    Property<int>::setSendCallback([mySpa](const char* name, int value) {
        return mySpa->sendValue(name, value);
    });
    Property<int>::setSendCallback([mySpa](const char* name, String value) {
        return mySpa->sendValue(name, value);
    });
    Property<String>::setSendCallback([mySpa](const char* name, String value) {
        return mySpa->sendValue(name, value);
    });
    Property<bool>::setSendCallback([mySpa](const char* name, int value) {
        return mySpa->sendValue(name, value);
    });
    Property<time_t>::setSendCallback([mySpa](const char* name, time_t value) {
        return mySpa->sendValue(name, value);
    });
}

SpaInterface::~SpaInterface() {}


void SpaInterface::setUpdateFrequency(int updateFrequency) {
    _updateFrequency = updateFrequency;
}

void SpaInterface::flushSerialReadBuffer() {
    int x = 0;

    debugD("Flushing serial stream - %i bytes in the buffer", port.available());
    while (port.available() > 0 && x++<5120) {
        int bytes = port.read();
        debugV("%i,",bytes);
    }
    debugD("Flushed serial stream - %i bytes in the buffer", port.available());
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

bool SpaInterface::sendValue(const char* name, int mode) {
    debugD("sendValue int name: %s, mode: %i", name, mode);

    if (strcmp(name, "RB_TP_Pump1") == 0) {
        if (sendCommandCheckResult("S22:"+String(mode),"S22-OK")) {
            RB_TP_Pump1.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "RB_TP_Pump2") == 0) {
        if (sendCommandCheckResult("S23:"+String(mode),"S23-OK")) {
            RB_TP_Pump2.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "setRB_TP_Pump3") == 0) {
        if (sendCommandCheckResult("S24:"+String(mode),"S24-OK")) {
            RB_TP_Pump3.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "RB_TP_Pump4") == 0) {
        if (sendCommandCheckResult("S25:"+String(mode),"S25-OK")) {
            RB_TP_Pump4.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "RB_TP_Pump5") == 0) {
        if (sendCommandCheckResult("S26:"+String(mode),"S26-OK")) {
            RB_TP_Pump5.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "RB_TP_Light") == 0) {
        if (sendCommandCheckResult("W14","W14")) {
            RB_TP_Light.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "HELE") == 0) {
        if (sendCommandCheckResult("W98:"+String(mode),String(mode))) {
            HELE.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "STMP") == 0) {
        if (sendCommandCheckResult("W40:" + String(mode),String(mode))) {
            STMP.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "L_1SNZ_DAY") == 0) {
        if (sendCommandCheckResult(String("W67:")+mode,String(mode))) {
            L_1SNZ_DAY.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "L_1SNZ_BGN") == 0) {
        if (sendCommandCheckResult(String("W68:")+mode,String(mode))) {
            L_1SNZ_BGN.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "L_1SNZ_END") == 0) {
        if (sendCommandCheckResult(String("W69:")+mode,String(mode))) {
            L_1SNZ_END.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "L_2SNZ_DAY") == 0) {
        if (sendCommandCheckResult(String("W70:")+mode,String(mode))) {
            L_2SNZ_DAY.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "L_2SNZ_BGN") == 0) {
        if (sendCommandCheckResult(String("W71:")+mode,String(mode))) {
            L_1SNZ_BGN.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "L_2SNZ_END") == 0) {
        if (sendCommandCheckResult(String("W72:")+mode,String(mode))) {
            L_1SNZ_END.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "HPMP") == 0) {
        if (sendCommandCheckResult("W99:"+String(mode),String(mode))) {
            HPMP.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "ColorMode") == 0) {
        if (sendCommandCheckResult("S07:"+String(mode),String(mode))) {
            ColorMode.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "LBRTValue") == 0) {
        if (sendCommandCheckResult("S08:"+String(mode),String(mode))) {
            LBRTValue.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "LSPDValue") == 0) {
        if (sendCommandCheckResult("S09:"+String(mode),String(mode))) {
            LSPDValue.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "CurrClr") == 0) {
        if (sendCommandCheckResult("S10:"+String(mode),String(mode))) {
            CurrClr.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "Outlet_Blower") == 0) {
        if (sendCommandCheckResult("S28:"+String(mode),"S28-OK")) {
            Outlet_Blower.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "VARIValue") == 0) {
        debugD("setVARIValue - %i", mode);
        if (sendCommandCheckResult("S13:"+String(mode),String(mode)+"  S13")) {
            VARIValue.updateValue(mode);
            return true;
        }
        return false;
    }
    else if (strcmp(name, "Mode") == 0) {
        if (sendCommandCheckResult("W66:"+String(mode),String(mode))) {
            Mode.updateValue(spaModeStrings[mode]);
            return true;
        }
        return false;
    }

    return false;
}

bool SpaInterface::sendValue(const char* name, String smode) {
    debugD("sendValue String name: %s, smode: %s", name, smode.c_str());

    if (strcmp(name, "HPMP") == 0) {
        debugD("setHPMP - %s", smode.c_str());

        for (int x=0; x<HPMPStrings.size(); x++) {
            if (HPMPStrings[x] == smode) {
                if (HPMP.getValue() != x) {
                    return sendValue(name, x);
                } else {
                    return true;
                }
            }
        }
        return false;
    }
    else if (strcmp(name, "ColorMode") == 0) {
        debugD("setColorMode - %s", smode.c_str());
        for (int x=0; x<colorModeStrings.size(); x++) {
            if (colorModeStrings[x] == smode) {
                if (ColorMode.getValue() != x) {
                    return sendValue(name, x);
                } else {
                    return true;
                }
            }
        }
        return false;
    }
    else if (strcmp(name, "setLSPDValue") == 0) {
        debugD("setLSPDValue - %s", smode.c_str());
        int x = atoi(smode.c_str());
        if (x > 0 && x < 6) {
            if (LSPDValue.getValue() != x) {
                return sendValue(name, x);
            } else {
                true;
            }
        }
        return false;
    }
    else if (strcmp(name, "Mode") == 0) {
        debugD("setMode - %s", smode.c_str());
        for (int x=0; x<spaModeStrings.size(); x++) {
            if (spaModeStrings[x] == smode) {
                if (Mode.getValue() != smode) {
                    return sendValue(name, x);
                } else {
                    true;
                }
            }
        }
        return false;
    }
    return false;
}

bool SpaInterface::sendValue(const char* name, time_t t) {
    debugD("sendValue time_t name: %s", name);
    if (strcmp(name, "SpaTime") == 0) {
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
        if (statusResponseRaw[field][0] == ':') registerCounter++;
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
    }

    //Flush the remaining data from the buffer as the last field is meaningless
    flushSerialReadBuffer();

    statusResponse.updateValue(statusResponseTmp);

    if (field < statusResponseMinFields) {
        debugE("Throwing exception - %i fields read expecting at least %i",field, statusResponseMinFields);
        return false;
    }

    updateMeasures(statusResponseRaw, R2, R3, R4, R5, R6, R7, R9, RA, RB, RC, RE, RG);
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
