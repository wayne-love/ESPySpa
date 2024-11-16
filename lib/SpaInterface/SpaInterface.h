#ifndef SPAINTERFACE_H
#define SPAINTERFACE_H

#include <Arduino.h>
#include <functional>
#include <stdexcept>
#include <RemoteDebug.h>
#include "SpaProperties.h"

extern RemoteDebug Debug;
#define FAILEDREADFREQUENCY 1000 //(ms) Frequency to retry on a failed read of the status registers.

class SpaInterface : public SpaProperties {
    private:

        /// @brief How often to pole the spa for updates in seconds.
        int _updateFrequency = 60;

        /// @brief Number of fields that we can expect to read.

        int statusResponseMinFields = 288;

        static const int statusResponseMaxFields = 295;

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

        /// @brief Does the status response array contain valid information?
        bool validStatusResponse = false;

        /// @brief Serial stream to interface to SpanNet hardware.
        Stream &port;

        /// @brief Read from serial interface, expect it to contain return from RF command
        /// @return true if successful read, false if there was a corrupted read
        bool readStatus();

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

        void flushSerialReadBuffer();


        /// @brief Stores millis time at which next update should occur
        unsigned long _nextUpdateDue = 0;

        /// @brief False until first successful read of the registers.
        bool _initialised = false;

        /// @brief If the result registers have been modified locally, need to do a fress pull from the controller
        bool _resultRegistersDirty = true;


        void (*updateCallback)() = nullptr;

        u_long _lastWaitMessage = millis();

        bool sendValue(const char* name, String smode);
        bool sendValue(const char* name, int mode);
        bool sendValue(const char* name, time_t t);


    public:
        /// @brief Init SpaInterface.
        SpaInterface();

        ~SpaInterface();

        /// @brief configure how often the spa is polled in seconds.
        /// @param updateFrequency
        void setUpdateFrequency(int updateFrequency);

        /// @brief Complete RF command response in a single string
        Property<String> statusResponse = Property<String>("statusResponse");

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
};


#endif  //SPAINTERFACE_H
