#ifndef SPAINTERFACE_H
#define SPAINTERFACE_H

#include <Arduino.h>
#include <functional>
#include <stdexcept>
#include <RemoteDebug.h>
#include "SpaProperties.h"

extern RemoteDebug Debug;
#define FAILEDREADFREQUENCY 1000 //(ms) Frequency to retry on a failed read of the status registers.
#define V2FIRMWARE_STRING "SW V2" // String to identify V2 firmware

class SpaInterface : public SpaProperties {
    private:

        /// @brief How often to pole the spa for updates in seconds.
        int _updateFrequency = 60;

        /// @brief Number of fields that we can expect to read.
        static const int statusResponseV2MinFields = 253;
        static const int statusResponseMinFields = 275;
        static const int statusResponseMaxFields = 300;

        /// @brief Each field of the RF cmd response as seperate elements.
        String statusResponseRaw[statusResponseMaxFields];

        int R2=-1;
        int R3=-1;
        int R4=-1;
        int R5=-1;
        int R6=-1;
        int R7=-1;
        int R9=-1;
        int RA=-1;
        int RB=-1;
        int RC=-1;
        int RE=-1;
        int RG=-1;

        // Register minimum sizes aligned with data read in updateMeasures()
        const std::array <int, 12> registerMinSize = {
          29, //R2
          25, //R3
          23, //R4
          22, //R5
          23, //R6
          30, //R7
          12, //R9
          12, //RA
          12, //RB
          10, //RC
          30, //RE
          12  //RG
          };

        /// @brief Does the status response array contain valid information?
        bool validStatusResponse = false;

        /// @brief Serial stream to interface to SpanNet hardware.
        Stream &port;

        /// @brief Read from serial interface, expect it to contain return from RF command
        /// @return true if successful read, false if there was a corrupted read
        bool readStatus();

        void updateMeasures();



        /// @brief Sends command to SpaNet controller.  Result must be read by some other method.
        /// Used for the 'RF' command so that we can do a optomised read of the return array.
        /// @param cmd - cmd to be executed.
        void sendCommand(String cmd);

        
        /// @brief Sends a command to the SpanNet controller and returns the result string
        /// @param cmd - cmd to be executed
        /// @return String - result string
        String sendCommandReturnResult(String cmd);

        /// @brief Sends the command and checks the result against the expected outcome
        /// @param cmd command to send
        /// @param expected expected string response
        /// @return result
        bool sendCommandCheckResult(String cmd, String expected);

        /// @brief Updates the attributes by sending the RF command and parsing the result.
        void updateStatus();

        void flushSerialReadBuffer() { flushSerialReadBuffer(false); };
        String flushSerialReadBuffer(bool returnData);


        /// @brief Stores millis time at which next update should occur
        unsigned long _nextUpdateDue = 0;

        /// @brief False until first successful read of the registers.
        bool _initialised = false;

        /// @brief If the result registers have been modified locally, need to do a fress pull from the controller
        bool _resultRegistersDirty = true;

   
        void (*updateCallback)() = nullptr;

        u_long _lastWaitMessage = millis();



    public:
        /// @brief Init SpaInterface.
        SpaInterface();

        ~SpaInterface();

        /// @brief configure how often the spa is polled in seconds.
        /// @param SpaPollFrequency
        void setSpaPollFrequency(int updateFrequency);

        /// @brief Complete RF command response in a single string
        Property<String> statusResponse;

        /// @brief To be called by loop function of main sketch.  Does regular updates, etc.
        void loop();

        /// @brief Have we sucessfuly read the registers from the SpaNet controller.
        /// @return 
        bool isInitialised();

        /// @brief Set the function to be called when properties have been updated.
        /// @param f 
        void setUpdateCallback(void (*f)());

        /// @brief Clear the call back function.
        void clearUpdateCallback();

        /// @brief Set the desired water temperature
        /// @param temp Between 5 and 40 in 0.5 increments
        /// @return Returns True if succesful
        bool setSTMP(int temp);

        /// @brief Set snooze day ({128,127,96,31} -> {"Off","Everyday","Weekends","Weekdays"};)
        /// @param mode
        /// @return Returns True if succesful
        bool setL_1SNZ_DAY(int mode);

        /// @brief Set snooze time (provide an integer that uses this calculation HH:mm > HH*265+mm. e.g. 13:47 = 13*256+47 = 3375)
        /// @param mode
        /// @return Returns True if succesful
        bool setL_1SNZ_BGN(int mode);
        bool setL_1SNZ_END(int mode);

        /// @brief Set snooze day ({128,127,96,31} -> {"Off","Everyday","Weekends","Weekdays"};)
        /// @param mode
        /// @return Returns True if succesful
        bool setL_2SNZ_DAY(int mode);

        /// @brief Set snooze time (provide an integer that uses this calculation HH:mm > HH*265+mm. e.g. 13:47 = 13*256+47 = 3375)
        /// @param mode
        /// @return Returns True if succesful
        bool setL_2SNZ_BGN(int mode);
        bool setL_2SNZ_END(int mode);

        /// @brief Set Heat pump operating mode (0 --> 3, {auto, heat, cool, off})
        /// @param mode 
        /// @return Returns True if succesful
        bool setHPMP(int mode);
        bool setHPMP(String mode);

        /// @brief Set light mode (0 = white, 1 = colour, 2 = step, 3 = fade, 4 = party)
        /// @param mode
        /// @return Returns True if succesful
        bool setColorMode(int mode);
        bool setColorMode(String mode);

        /// @brief Set light brightness (min 1, max 5)
        /// @param mode
        /// @return Returns True if succesful
        bool setLBRTValue(int mode);

        /// @brief Set light effect speed (min 1, max 5)
        /// @param mode
        /// @return Returns True if succesful
        bool setLSPDValue(int mode);
        bool setLSPDValue(String mode);

        /// @brief Set light colour (min 0, max 31)
        /// @param mode
        /// @return Returns True if succesful
        bool setCurrClr(int mode);

        /// @brief Set the operating mode for pump 1
        /// @param mode 0 = off, 1 = on, 4 = auto (if supported)
        /// @return True if successful
        bool setRB_TP_Pump1(int mode);

        /// @brief Set the operating mode for pump 2
        /// @param mode 0 = off, 1 = on, 4 = auto (if supported)
        /// @return True if successful
        bool setRB_TP_Pump2(int mode);

        /// @brief Set the operating mode for pump 3
        /// @param mode 0 = off, 1 = on, 4 = auto (if supported)
        /// @return True if successful
        bool setRB_TP_Pump3(int mode);

        /// @brief Set the operating mode for pump 4
        /// @param mode 0 = off, 1 = on, 4 = auto (if supported)
        /// @return True if successful
        bool setRB_TP_Pump4(int mode);

        /// @brief Set the operating mode for pump 5
        /// @param mode 0 = off, 1 = on, 4 = auto (if supported)
        /// @return True if successful
        bool setRB_TP_Pump5(int mode);

        bool setRB_TP_Light(int mode);

        /// @brief Set aux element operating mode
        /// @param mode 0 = off, 1 = on
        /// @return True if successful
        bool setHELE(int mode);

        /// @brief Sets the day of week on the spa
        /// @param d Day of week (0 = Monday - 6 = Sunday)
        /// @return True if successful
        bool setSpaDayOfWeek(int d);

        /// @brief Sets the clock on the spa
        /// @param t Time
        /// @return True if successful
        bool setSpaTime(time_t t);

        /// @brief Controls the air blower
        /// @param mode 0 = Varible, 1 = Ramp, 2 = Off
        /// @return True if successful
        bool setOutlet_Blower(int mode);

        /// @brief Set the speed of the air blower
        /// @param mode 1 = low, 5 = high
        /// @return True if successful
        bool setVARIValue(int mode);

        /// @brief Set Spa mode (0 --> 4, {"NORM","ECON","AWAY","WEEK"};)
        /// @param mode
        /// @return Returns True if succesful
        bool setMode(int mode);
        bool setMode(String mode);

        /// @brief Set filtration block duration (1,2,3,4,6,8,12,24 hours)
        /// @param duration 
        /// @return 
        bool setFiltBlockHrs(String duration);

        /// @brief Set filtration hours (1 to 24 hours)
        /// @param duration
        bool setFiltHrs(String duration);
};


// Define the function pointer type for getPumpInstallState functions
typedef String (SpaInterface::*GetPumpStateInstallFunction)();

// Declare the array of function pointers for each pump's install state as static
static GetPumpStateInstallFunction pumpInstallStateFunctions[] = {
  &SpaInterface::getPump1InstallState,
  &SpaInterface::getPump2InstallState,
  &SpaInterface::getPump3InstallState,
  &SpaInterface::getPump4InstallState,
  &SpaInterface::getPump5InstallState
};

// Define the function pointer type for getPumpState functions
typedef int (SpaInterface::*GetPumpStateFunction)();

// Declare the array of function pointers for each pump's state as static
static GetPumpStateFunction pumpStateFunctions[] = {
  &SpaInterface::getRB_TP_Pump1,
  &SpaInterface::getRB_TP_Pump2,
  &SpaInterface::getRB_TP_Pump3,
  &SpaInterface::getRB_TP_Pump4,
  &SpaInterface::getRB_TP_Pump5
};

// Define the function pointer type for getPumpState functions
typedef bool (SpaInterface::*SetPumpFunction)(int);

// Declare the array of function pointers for each pump's state as static
static SetPumpFunction setPumpFunctions[] = {
  &SpaInterface::setRB_TP_Pump1,
  &SpaInterface::setRB_TP_Pump2,
  &SpaInterface::setRB_TP_Pump3,
  &SpaInterface::setRB_TP_Pump4,
  &SpaInterface::setRB_TP_Pump5
};

#endif
