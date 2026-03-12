#ifndef SPAINTERFACE_H
#define SPAINTERFACE_H

#include <Arduino.h>
#include <functional>
#include <stdexcept>
#include <RemoteDebug.h>
#include <time.h>
#include <TimeLib.h>


extern RemoteDebug Debug;
#define FAILEDREADFREQUENCY 1000 //(ms) Frequency to retry on a failed read of the status registers.
#define V2FIRMWARE_STRING "SW V2" // String to identify V2 firmware
template <typename T, size_t N>
constexpr size_t array_count(const T (&)[N]) { return N; }

class SpaInterface {
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

        /// @brief Set the desired water temperature
        /// @param temp Between 5 and 40 in 0.5 increments
        /// @return Returns True if succesful
        bool setSTMP(int temp);
        
        /// @brief Internal writer used by `HPMP` RWProperty.
        /// @details Sends `W99:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..3).
        bool setHPMP(int mode);
        /// @brief Internal writer used by `ColorMode` RWProperty.
        /// @details Sends `S07:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..4).
        bool setColorMode(int mode);
        /// @brief Internal writer used by `LBRTValue` RWProperty.
        /// @details Sends `S08:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (1..5).
        bool setLBRTValue(int mode);
        /// @brief Internal writer used by `LSPDValue` RWProperty.
        /// @details Sends `S09:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (1..5).
        bool setLSPDValue(int mode);
        /// @brief Internal writer used by `CurrClr` RWProperty.
        /// @details Sends `S10:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..31).
        bool setCurrClr(int mode);

        /// @brief Set snooze day ({128,127,96,31} -> {"Off","Everyday","Weekends","Weekdays"};)
        /// @param mode
        /// @return Returns True if succesful
        bool setL_1SNZ_DAY(int mode);

        /// @brief Set snooze day ({128,127,96,31} -> {"Off","Everyday","Weekends","Weekdays"};)
        /// @param mode
        /// @return Returns True if succesful
        bool setL_2SNZ_DAY(int mode);

        bool setL_1SNZ_BGN(int mode);
        bool setL_1SNZ_END(int mode);
        bool setL_2SNZ_BGN(int mode);
        bool setL_2SNZ_END(int mode);

        /// @brief Internal writer used by `RB_TP_Pump1` RWProperty.
        /// @details Sends `S22:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..4).
        bool setRB_TP_Pump1(int mode);
        /// @brief Internal writer used by `RB_TP_Pump2` RWProperty.
        /// @details Sends `S23:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..4).
        bool setRB_TP_Pump2(int mode);
        /// @brief Internal writer used by `RB_TP_Pump3` RWProperty.
        /// @details Sends `S24:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..4).
        bool setRB_TP_Pump3(int mode);
        /// @brief Internal writer used by `RB_TP_Pump4` RWProperty.
        /// @details Sends `S25:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..4).
        bool setRB_TP_Pump4(int mode);
        /// @brief Internal writer used by `RB_TP_Pump5` RWProperty.
        /// @details Sends `S26:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..4).
        bool setRB_TP_Pump5(int mode);
        /// @brief Internal writer used by `HELE` RWProperty.
        /// @details Sends `W98:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        bool setHELE(bool mode);
        /// @brief Internal writer used by `SpaDayOfWeek` RWProperty.
        /// @details Sends `S06:<d>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..6).
        bool setSpaDayOfWeek(int d);
        /// @brief Internal writer used by `Outlet_Blower` RWProperty.
        /// @details Sends `S28:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..2).
        bool setOutlet_Blower(int mode);
        /// @brief Internal writer used by `Mode` RWProperty.
        /// @details Sends `W66:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..3).
        bool setMode(int mode);
        /// @brief Internal writer used by `VARIValue` RWProperty.
        /// @details Sends `S13:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (1..5).
        bool setVARIValue(int mode);
        /// @brief Internal writer used by `FiltBlockHrs` RWProperty.
        /// @details Sends `W90:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for values not in the valid set (1,2,3,4,6,8,12,24).
        bool setFiltBlockHrs(int mode);
        /// @brief Internal writer used by `FiltHrs` RWProperty.
        /// @details Sends `W60:<hrs>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (1..24).
        bool setFiltHrs(int hrs);
        /// @brief Internal writer used by `LockMode` RWProperty.
        /// @details Sends `S21:<mode>` to the controller and only updates
        /// the cached property value when the command succeeds.
        /// Throws std::out_of_range for invalid values (0..2).
        bool setLockMode(int mode);
        /// @brief Internal writer used by `SpaTime` RWProperty.
        /// @details Sends S01..S05 + S06 (via setSpaDayOfWeek) to the controller.
        bool setSpaTime(time_t t);
        /// @brief Internal writer used by `RB_TP_Light` RWProperty.
        /// @details Sends `W14` to toggle the light; updates cached value to `mode`.
        bool setRB_TP_Light(int mode);
        /// @brief Internal writer used by `statusResponse` RWProperty.
        /// @details No-op — statusResponse is populated internally by readStatus().
        /// Allows external injection of a raw response string (e.g. for testing).
        bool setStatusResponse(String s);

    public:
        /// @brief Init SpaInterface.
        SpaInterface();

        ~SpaInterface();

        // Read-only value holder synced from the spa; external code can only read.
        template <typename T>
        class ROProperty {
        public:
            // Optional label/value mapping for human-readable access.
            struct LabelValue {
                const char* label;
                T value;
            };

            ROProperty() = default;
            // Provide a label/value map; array size is deduced.
            template <size_t N>
            ROProperty(const LabelValue (&map)[N])
                : _map(map), _mapSize(N) {}

            T get() const { return _value; }
            operator T() const { return _value; }
            void setCallback(void (*c)(T)) { _callback = c; }
            void clearCallback() { _callback = nullptr; }
            // Returns the matching label for the current value, or fallback if not found.
            const char* getLabel(const char* fallback = "Unknown") const {
                if (!_map || _mapSize == 0) {
                    return fallback;
                }
                for (size_t i = 0; i < _mapSize; i++) {
                    if (_map[i].value == _value) {
                        return _map[i].label;
                    }
                }
                return fallback;
            }
            // Expose label/value map for building UI dropdowns.
            const LabelValue* getLabelMap(size_t& count) const {
                count = _mapSize;
                return _map;
            }
            size_t getLabelCount() const { return _mapSize; }
            const char* getLabelAt(size_t index, const char* fallback = "Unknown") const {
                if (!_map || index >= _mapSize) {
                    return fallback;
                }
                return _map[index].label;
            }

        protected:
            T _value{};
            bool _hasValue = false;
            const LabelValue* _map = nullptr;
            size_t _mapSize = 0;
            void (*_callback)(T) = nullptr;

            // Called by SpaInterface when a fresh value is received from the spa.
            void update(T newValue) {
                T oldValue = _value;
                _value = newValue;
                _hasValue = true;
                if (_callback && oldValue != newValue) {
                    _callback(_value);
                }
            }
            // Reverse-lookup by label string; throws std::invalid_argument if the label
            // is not found in the map or if no map is configured.
            void updateFromLabel(const char* label) {
                if (!label || !this->_map || this->_mapSize == 0) {
                    throw std::invalid_argument("updateFromLabel: no label map configured");
                }
                for (size_t i = 0; i < this->_mapSize; i++) {
                    if (strcmp(this->_map[i].label, label) == 0) {
                        update(this->_map[i].value);
                        return;
                    }
                }
                throw std::invalid_argument((String("updateFromLabel: unknown label '") + label + "'").c_str());
            }

            friend class SpaInterface;
        };

        // Read/write property that sends commands before updating the cached value.
        template <typename T>
        class RWProperty : public ROProperty<T> {
        public:
            using WriteFunction = bool (SpaInterface::*)(T);
            using LabelValue = typename ROProperty<T>::LabelValue;

            RWProperty() = default;
            // Basic RW property with owner/writer only.
            RWProperty(SpaInterface* owner, WriteFunction writer)
                : _owner(owner), _writer(writer) {}
            // RW property with label/value map for setLabel/getLabel.
            template <size_t N>
            RWProperty(SpaInterface* owner, WriteFunction writer,
                       const LabelValue (&map)[N])
                : ROProperty<T>(map),
                  _owner(owner),
                  _writer(writer) {}

            // Sends the value to the spa first; caches it only on success.
            // Validation is performed by the writer and any exception is propagated.
            void set(T newValue) {
                if (this->_hasValue && newValue == this->_value) {
                    return;
                }

                if (!_owner || !_writer) {
                    throw std::invalid_argument("RWProperty has no owner/writer");
                }

                if (!(_owner->*_writer)(newValue)) {
                    throw std::runtime_error("RWProperty write failed");
                }

                this->update(newValue);
            }

            RWProperty& operator=(T newValue) {
                set(newValue);
                return *this;
            }

            // Set by label, throws if map is not configured or label is unknown.
            void setLabel(const char* label) {
                if (!label || !this->_map || this->_mapSize == 0) {
                    throw std::invalid_argument("RWProperty label map not configured");
                }
                for (size_t i = 0; i < this->_mapSize; i++) {
                    if (strcmp(this->_map[i].label, label) == 0) {
                        set(this->_map[i].value);
                        return;
                    }
                }
                throw std::out_of_range("RWProperty label not found");
            }

        private:
            // Owner + writer are required to commit changes to the spa.
            SpaInterface* _owner = nullptr;
            WriteFunction _writer = nullptr;
        };

    private:
        // Label maps are private — use getLabelMap() on the property for external access.
        static constexpr ROProperty<int>::LabelValue HPMP_Map[] = {
            {"Auto", 0},
            {"Heat", 1},
            {"Cool", 2},
            {"Off", 3},
        };
        static constexpr ROProperty<int>::LabelValue ColorMode_Map[] = {
            {"White", 0},
            {"Color", 1},
            {"Fade", 2},
            {"Step", 3},
            {"Party", 4},
        };
        static constexpr ROProperty<int>::LabelValue LSPDValue_Map[] = {
            {"1", 1},
            {"2", 2},
            {"3", 3},
            {"4", 4},
            {"5", 5},
        };
        static constexpr ROProperty<int>::LabelValue CurrClr_Map[] = {
            {"0", 0},
            {"15", 4},
            {"30", 4},
            {"45", 19},
            {"60", 13},
            {"75", 25},
            {"90", 25},
            {"105", 16},
            {"120", 10},
            {"135", 7},
            {"150", 2},
            {"165", 8},
            {"180", 5},
            {"195", 3},
            {"210", 6},
            {"225", 6},
            {"240", 21},
            {"255", 21},
            {"270", 21},
            {"285", 18},
            {"300", 18},
            {"315", 9},
            {"330", 9},
            {"345", 1},
            {"360", 1},
        };
        /// @details Shared label/value map for both sleep timers: 128=Off, 127=Everyday, 96=Weekends, 31=Weekdays.
        static constexpr ROProperty<int>::LabelValue SNZ_DAY_Map[] = {
            {"Off", 128},
            {"Everyday", 127},
            {"Weekends", 96},
            {"Weekdays", 31},
            {"Monday", 16},
            {"Tuesday", 8},
            {"Wednesday", 4},
            {"Thuesday", 2},
            {"Friday", 1},
            {"Saturday", 64},
            {"Sunday", 32},
        };
        static constexpr ROProperty<int>::LabelValue SpaDayOfWeek_Map[] = {
            {"Monday",    0},
            {"Tuesday",   1},
            {"Wednesday", 2},
            {"Thursday",  3},
            {"Friday",    4},
            {"Saturday",  5},
            {"Sunday",    6},
        };
        static constexpr ROProperty<int>::LabelValue Outlet_Blower_Map[] = {
            {"Variable", 0},
            {"Ramp",     1},
            {"Off",      2},
        };
        static constexpr ROProperty<int>::LabelValue Mode_Map[] = {
            {"NORM", 0},
            {"ECON", 1},
            {"AWAY", 2},
            {"WEEK", 3},
        };
        static constexpr ROProperty<int>::LabelValue FiltBlockHrs_Map[] = {
            {"1",  1},
            {"2",  2},
            {"3",  3},
            {"4",  4},
            {"6",  6},
            {"8",  8},
            {"12", 12},
            {"24", 24},
        };
        static constexpr ROProperty<int>::LabelValue LockMode_Map[] = {
            {"Unlocked",         0},
            {"Partially Locked", 1},
            {"Locked",           2},
        };

    public:
        /// @brief configure how often the spa is polled in seconds.
        /// @param SpaPollFrequency
        void setSpaPollFrequency(int updateFrequency);

        /// @brief Complete RF command response in a single string
        RWProperty<String> statusResponse{this, &SpaInterface::setStatusResponse};

        const std::array<String, 2> autoPumpOptions = {"Manual", "Auto"};

        /// @brief Mains voltage (V).
        /// @details Read-only.
        ROProperty<int> MainsVoltage;

        /// @brief Mains current draw x10.
        /// @details Read-only; value 77 represents 7.7A.
        ROProperty<int> MainsCurrent;

        // R2
        /// @brief Controller case temperature x10 (°C). e.g. 250 = 25.0°C.
        ROProperty<int> CaseTemperature;
        /// @brief Port current draw x10 (A).
        ROProperty<int> PortCurrent;
        /// @brief Current day of week on Spa RTC.
        /// @details Read/write. 0=Monday .. 6=Sunday.
        RWProperty<int> SpaDayOfWeek{this, &SpaInterface::setSpaDayOfWeek, SpaDayOfWeek_Map};
        /// @brief Spa RTC clock value.
        /// @details Read/write. Writing sends S01..S05 + S06 to the controller.
        RWProperty<time_t> SpaTime{this, &SpaInterface::setSpaTime};
        /// @brief Heater element temperature x10 (°C).
        ROProperty<int> HeaterTemperature;
        /// @brief Pool temperature x10 (°C). Note: often returns unreliable values; use WTMP for actual water temperature.
        ROProperty<int> PoolTemperature;
        /// @brief Minutes remaining before spa returns to sleep.
        ROProperty<int> AwakeMinutesRemaining;
        /// @brief Total filtration pump run time (minutes).
        ROProperty<int> FiltPumpRunTimeTotal;
        /// @brief Filtration pump required run minutes remaining.
        ROProperty<int> FiltPumpReqMins;
        /// @brief Load management timeout counter.
        ROProperty<int> LoadTimeOut;
        /// @brief Total controller run time x10 (hours). e.g. 33207 = 3320.7 hours.
        ROProperty<int> HourMeter;
        /// @brief Relay 1 output state.
        ROProperty<int> Relay1;
        /// @brief Relay 2 output state.
        ROProperty<int> Relay2;
        /// @brief Relay 3 output state.
        ROProperty<int> Relay3;
        /// @brief Relay 4 output state.
        ROProperty<int> Relay4;
        /// @brief Relay 5 output state.
        ROProperty<int> Relay5;
        /// @brief Relay 6 output state.
        ROProperty<int> Relay6;
        /// @brief Relay 7 output state.
        ROProperty<int> Relay7;
        /// @brief Relay 8 output state.
        ROProperty<int> Relay8;
        /// @brief Relay 9 output state.
        ROProperty<int> Relay9;
        /// @brief True when water is detected in the spa.
        ROProperty<bool> WaterPresent;
        // R3
        /// @brief Current limit setting (A). Range 10–60A; should match the circuit breaker rating feeding the spa (C.LMT OEM setting).
        ROProperty<int> CLMT;
        /// @brief Mains phase configuration (1=Single Phase, 2=Dual Phase, 3=Three Phase).
        ROProperty<int> PHSE;
        /// @brief Phase 1 load limit — maximum number of loads (pumps/blower) allowed to run simultaneously on phase 1. Range 1–5 (x.LLM OEM setting).
        ROProperty<int> LLM1;
        /// @brief Phase 2 load limit — maximum number of loads (pumps/blower) allowed to run simultaneously on phase 2. Range 1–5 (x.LLM OEM setting).
        ROProperty<int> LLM2;
        /// @brief Phase 3 load limit — maximum number of loads (pumps/blower) allowed to run simultaneously on phase 3. Range 1–5 (x.LLM OEM setting).
        ROProperty<int> LLM3;
        /// @brief Controller software/firmware version string (e.g. "SW V3 SV3a").
        ROProperty<String> SVER;
        /// @brief Controller model identifier string.
        ROProperty<String> Model;
        /// @brief Serial number part 1.
        ROProperty<String> SerialNo1;
        /// @brief Serial number part 2.
        ROProperty<String> SerialNo2;
        /// @brief Dipswitch 1 state. SW1: Circulation pump fitted (ON=Fitted, OFF=Not Fitted).
        ROProperty<bool> D1;
        /// @brief Dipswitch 2 state. SW2: Pump 1 type (ON=Two Speed, OFF=Single Speed; if OFF, Pump 2 assumed fitted).
        ROProperty<bool> D2;
        /// @brief Dipswitch 3 state. SW3: SV2/SV4: Pump 3 type (ON=Two Speed, OFF=Single Speed); SV3: Pump 3 fitted (ON=Fitted, OFF=Not Fitted). Not used on SV2/SV2-VH.
        ROProperty<bool> D3;
        /// @brief Dipswitch 4 state. SW4: SV2/SV4: Pump 4 fitted (ON=Fitted, OFF=Not Fitted; not used on SV2/SV2-VH); SV3: Not used.
        ROProperty<bool> D4;
        /// @brief Dipswitch 5 state. SW5: Phase input selection (ON=2/3 Phase, OFF=Single Phase). Enables SW6 when ON.
        ROProperty<bool> D5;
        /// @brief Dipswitch 6 state. SW6: Multi-phase type, enabled when SW5=ON (ON=Three Phase, OFF=Two Phase).
        ROProperty<bool> D6;
        /// @brief Pump configuration string.
        ROProperty<String> Pump;
        ROProperty<int> LS;
        ROProperty<bool> HV;
        /// @brief Snooze mode remaining time (minutes).
        ROProperty<int> SnpMR;
        /// @brief Operational status string (e.g. "Filtering", "Heating").
        ROProperty<String> Status;
        /// @brief Priming cycle count.
        ROProperty<int> PrimeCount;
        /// @brief Error/fault code. 0 = no fault.
        ROProperty<int> EC;
        /// @brief Heater ambient air temperature x10 (°C).
        ROProperty<int> HAMB;
        /// @brief Heater connection/conductivity value.
        ROProperty<int> HCON;
        // R4
        /// @brief Spa operating mode.
        /// @details Read/write. 0=NORM, 1=ECON, 2=AWAY, 3=WEEK.
        RWProperty<int> Mode{this, &SpaInterface::setMode, Mode_Map};
        /// @brief Service interval 1 countdown (hours remaining).
        ROProperty<int> Ser1_Timer;
        /// @brief Service interval 2 countdown (hours remaining).
        ROProperty<int> Ser2_Timer;
        /// @brief Service interval 3 countdown (hours remaining).
        ROProperty<int> Ser3_Timer;
        /// @brief Heating mode (0=Element, 1=Heat pump, 2=Both, 3=Cool).
        ROProperty<int> HeatMode;
        /// @brief Pump idle timer (minutes since last use).
        ROProperty<int> PumpIdleTimer;
        /// @brief Pump run timer (minutes of continuous run).
        ROProperty<int> PumpRunTimer;
        /// @brief Adaptive pool temperature hysteresis x10 (°C).
        ROProperty<int> AdtPoolHys;
        /// @brief Adaptive heater temperature hysteresis x10 (°C).
        ROProperty<int> AdtHeaterHys;
        /// @brief Instantaneous power consumption x10 (W). e.g. 35 = 3.5 kW.
        ROProperty<int> Power;
        /// @brief Cumulative energy consumption x100 (kWh).
        ROProperty<int> Power_kWh;
        /// @brief Energy consumed today x10 (Wh).
        ROProperty<int> Power_Today;
        /// @brief Energy consumed yesterday x10 (Wh).
        ROProperty<int> Power_Yesterday;
        /// @brief Thermal cut-out trip count.
        ROProperty<int> ThermalCutOut;
        /// @brief Test/diagnostic output D1 state.
        ROProperty<int> Test_D1;
        /// @brief Test/diagnostic output D2 state.
        ROProperty<int> Test_D2;
        /// @brief Test/diagnostic output D3 state.
        ROProperty<int> Test_D3;
        /// @brief Element heat source temperature offset x10 (°C).
        ROProperty<int> ElementHeatSourceOffset;
        /// @brief Detected mains frequency (Hz).
        ROProperty<int> Frequency;
        /// @brief Heat pump heat mode source temperature offset x10 (°C).
        ROProperty<int> HPHeatSourceOffset_Heat;
        /// @brief Heat pump cool mode source temperature offset x10 (°C).
        ROProperty<int> HPHeatSourceOffset_Cool;
        /// @brief Heat source off-time (minutes).
        ROProperty<int> HeatSourceOffTime;
        /// @brief Variable speed pump current speed setting.
        ROProperty<int> Vari_Speed;
        /// @brief Variable speed pump output percentage (%).
        ROProperty<int> Vari_Percent;
        /// @brief Variable speed pump mode (0=Auto, 1=Manual).
        ROProperty<int> Vari_Mode;
        // R5
        /// @brief Blower/air injector operating state. Note: encoding unknown; this property is never populated from the RF response.
        ROProperty<int> RB_TP_Blower;
        /// @brief True when spa is sleeping due to a sleep timer.
        ROProperty<bool> RB_TP_Sleep;
        /// @brief True when ozone/UV sanitiser is running.
        ROProperty<bool> RB_TP_Ozone;
        /// @brief True when heating or cooling is actively running.
        ROProperty<bool> RB_TP_Heater;
        /// @brief True when auto mode is active.
        ROProperty<bool> RB_TP_Auto;
        /// @brief Light on/off state.
        /// @details Read/write. 0=Off, 1=On.
        RWProperty<int> RB_TP_Light{this, &SpaInterface::setRB_TP_Light};
        /// @brief Actual (measured) water temperature x10 (°C). e.g. 376 = 37.6°C. For the set point see STMP.
        ROProperty<int> WTMP;
        /// @brief True when a clean cycle is in progress.
        ROProperty<bool> CleanCycle;
        /// @brief Pump 1 operating state.
        /// @details Read/write. 0=Off, 1=On, 4=Auto (if supported).
        RWProperty<int> RB_TP_Pump1{this, &SpaInterface::setRB_TP_Pump1};
        /// @brief Pump 2 operating state.
        /// @details Read/write. 0=Off, 1=On, 4=Auto (if supported).
        RWProperty<int> RB_TP_Pump2{this, &SpaInterface::setRB_TP_Pump2};
        /// @brief Pump 3 operating state.
        /// @details Read/write. 0=Off, 1=On, 4=Auto (if supported).
        RWProperty<int> RB_TP_Pump3{this, &SpaInterface::setRB_TP_Pump3};
        /// @brief Pump 4 operating state.
        /// @details Read/write. 0=Off, 1=On, 4=Auto (if supported).
        RWProperty<int> RB_TP_Pump4{this, &SpaInterface::setRB_TP_Pump4};
        /// @brief Pump 5 operating state.
        /// @details Read/write. 0=Off, 1=On, 4=Auto (if supported).
        RWProperty<int> RB_TP_Pump5{this, &SpaInterface::setRB_TP_Pump5};
        // R6
        /// @brief Variable pump/blower speed.
        /// @details Read/write. Valid range 1..5.
        RWProperty<int> VARIValue{this, &SpaInterface::setVARIValue};
        /// @brief Light brightness.
        /// @details Read/write. Valid range 1..5.
        RWProperty<int> LBRTValue{this, &SpaInterface::setLBRTValue};
        /// @brief Light color index.
        /// @details Read/write. Valid range 0..31.
        RWProperty<int> CurrClr{this, &SpaInterface::setCurrClr, CurrClr_Map};
        /// @brief Light effect/mode.
        /// @details Read/write. 0=White, 1=Color, 2=Fade, 3=Step, 4=Party.
        RWProperty<int> ColorMode{this, &SpaInterface::setColorMode, ColorMode_Map};
        /// @brief Light effect speed.
        /// @details Read/write. Valid range 1..5.
        RWProperty<int> LSPDValue{this, &SpaInterface::setLSPDValue, LSPDValue_Map};
        /// @brief Filtration run time per block (hours).
        /// @details Read/write. Valid range 1..24.
        RWProperty<int> FiltHrs{this, &SpaInterface::setFiltHrs};
        /// @brief Filtration block duration (hours).
        /// @details Read/write. Valid values: 1, 2, 3, 4, 6, 8, 12, 24.
        RWProperty<int> FiltBlockHrs{this, &SpaInterface::setFiltBlockHrs, FiltBlockHrs_Map};
        /// @brief Water temperature set point x10.
        /// @details Read/write. Valid range 50..410 (5.0°C..41.0°C).
        RWProperty<int> STMP{this, &SpaInterface::setSTMP};
        /// @brief 24-hour operation flag (0=Off, 1=On).
        ROProperty<int> L_24HOURS;
        /// @brief Power save level (0=Off, 1=Low, 2=High).
        ROProperty<int> PSAV_LVL;
        /// @brief Power save start time encoded as h*256+m.
        ROProperty<int> PSAV_BGN;
        /// @brief Power save end time encoded as h*256+m.
        ROProperty<int> PSAV_END;
        /// @brief Sleep timer 1 day mode bitmap.
        /// @details Read/write. Typical values: 128=Off, 127=Everyday, 96=Weekends, 31=Weekdays.
        RWProperty<int> L_1SNZ_DAY{this, &SpaInterface::setL_1SNZ_DAY, SNZ_DAY_Map};
        /// @brief Sleep timer 2 day mode bitmap.
        /// @details Read/write. Typical values: 128=Off, 127=Everyday, 96=Weekends, 31=Weekdays.
        RWProperty<int> L_2SNZ_DAY{this, &SpaInterface::setL_2SNZ_DAY, SNZ_DAY_Map};
        /// @brief Sleep timer 1 start time.
        /// @details Read/write. Valid range 0..5947 encoded as h*256+m (24-hour clock).
        RWProperty<int> L_1SNZ_BGN{this, &SpaInterface::setL_1SNZ_BGN};
        /// @brief Sleep timer 2 start time.
        /// @details Read/write. Valid range 0..5947 encoded as h*256+m (24-hour clock).
        RWProperty<int> L_2SNZ_BGN{this, &SpaInterface::setL_2SNZ_BGN};
        /// @brief Sleep timer 1 finish time.
        /// @details Read/write. Valid range 0..5947 encoded as h*256+m (24-hour clock).
        RWProperty<int> L_1SNZ_END{this, &SpaInterface::setL_1SNZ_END};
        /// @brief Sleep timer 2 finish time.
        /// @details Read/write. Valid range 0..5947 encoded as h*256+m (24-hour clock).
        RWProperty<int> L_2SNZ_END{this, &SpaInterface::setL_2SNZ_END};
        /// @brief Default touchpad screen index shown when waking.
        ROProperty<int> DefaultScrn;
        /// @brief Pump and blower auto time-out duration (minutes). Range 10–60. Set via W74.
        ROProperty<int> TOUT;
        /// @brief OEM brand identifier.
        ROProperty<int> BRND;
        /// @brief Priming mode setting.
        ROProperty<int> PRME;
        /// @brief Heating element type/configuration.
        ROProperty<int> ELMT;
        /// @brief System type identifier.
        ROProperty<int> TYPE;
        /// @brief Gas heating installed (0=No, 1=Yes).
        ROProperty<int> GAS;
        /// @brief True when variable-speed pump is fitted.
        ROProperty<bool> VPMP;
        /// @brief True when HiFi audio output is enabled.
        ROProperty<bool> HIFI;
        // R7
        /// @brief Auto sanitise cycle start time encoded as h*256+m. Range 0–5947. e.g. 2304 = 9:00. Set via W73.
        ROProperty<int> WCLNTime;
        /// @brief Maximum mains voltage recorded this session (V).
        ROProperty<int> V_Max;
        /// @brief Minimum mains voltage recorded this session (V).
        ROProperty<int> V_Min;
        /// @brief Maximum mains voltage recorded in the last 24 hours (V).
        ROProperty<int> V_Max_24;
        /// @brief Minimum mains voltage recorded in the last 24 hours (V).
        ROProperty<int> V_Min_24;
        /// @brief Current sensor zero-point calibration offset.
        ROProperty<int> CurrentZero;
        /// @brief Current sensor gain calibration adjustment.
        ROProperty<int> CurrentAdjust;
        /// @brief Voltage sensor calibration adjustment.
        ROProperty<int> VoltageAdjust;
        /// @brief Service interval 1 period (hours).
        ROProperty<int> Ser1;
        /// @brief Service interval 2 period (hours).
        ROProperty<int> Ser2;
        /// @brief Service interval 3 period (hours).
        ROProperty<int> Ser3;
        /// @brief Maximum permitted supply voltage (V).
        ROProperty<int> VMAX;
        /// @brief Adaptive hysteresis setting x10 (°C).
        ROProperty<int> AHYS;
        /// @brief Minimum power level for load management (kW x10).
        ROProperty<int> PMIN;
        /// @brief Filtration pump power draw (kW x10).
        ROProperty<int> PFLT;
        /// @brief Heater element power draw (kW x10).
        ROProperty<int> PHTR;
        /// @brief Maximum total power for load management (kW x10).
        ROProperty<int> PMAX;
        /// @brief True when temperature display is in °F, false for °C.
        ROProperty<bool> TemperatureUnits;
        /// @brief True when ozone/sanitiser output is manually disabled.
        ROProperty<bool> OzoneOff;
        /// @brief True when ozone/sanitiser runs 24 hours, false for auto-controlled.
        ROProperty<bool> Ozone24;
        /// @brief True when circulation pump runs 24 hours, false for auto-controlled.
        ROProperty<bool> Circ24;
        /// @brief True when the circulation jet boost is active.
        ROProperty<bool> CJET;
        /// @brief True when variable-power element operation is enabled.
        ROProperty<bool> VELE;
        /// @brief True when heat pump operation is enabled while the spa pool is in use (H.USE OEM setting). When false, heat pump is suspended during spa use.
        ROProperty<bool> HUSE;
        /// @brief Aux element (booster) state.
        /// @details Read/write. false=Off, true=On.
        RWProperty<bool> HELE{this, &SpaInterface::setHELE};
        /// @brief Heatpump operating mode.
        /// @details Read/write. 0=Auto, 1=Heat, 2=Cool, 3=Off.
        RWProperty<int> HPMP{this, &SpaInterface::setHPMP, HPMP_Map};
        // R9/RA/RB fault logs — most recent 3 faults (F1=most recent)
        /// @brief Fault 1 hour-of-day at time of fault.
        ROProperty<int> F1_HR;
        /// @brief Fault 1 time-of-day encoded as h*256+m.
        ROProperty<int> F1_Time;
        /// @brief Fault 1 error code.
        ROProperty<int> F1_ER;
        /// @brief Fault 1 current reading x10 (A) at time of fault.
        ROProperty<int> F1_I;
        /// @brief Fault 1 voltage reading (V) at time of fault.
        ROProperty<int> F1_V;
        /// @brief Fault 1 pool temperature x10 (°C) at time of fault.
        ROProperty<int> F1_PT;
        /// @brief Fault 1 heater temperature x10 (°C) at time of fault.
        ROProperty<int> F1_HT;
        /// @brief Fault 1 case temperature x10 (°C) at time of fault.
        ROProperty<int> F1_CT;
        /// @brief Fault 1 pump state at time of fault.
        ROProperty<int> F1_PU;
        /// @brief Fault 1 system status at time of fault.
        ROProperty<int> F1_ST;
        /// @brief Fault 1 variable element state at time of fault.
        ROProperty<bool> F1_VE;
        /// @brief Fault 2 hour-of-day at time of fault.
        ROProperty<int> F2_HR;
        /// @brief Fault 2 time-of-day encoded as h*256+m.
        ROProperty<int> F2_Time;
        /// @brief Fault 2 error code.
        ROProperty<int> F2_ER;
        /// @brief Fault 2 current reading x10 (A) at time of fault.
        ROProperty<int> F2_I;
        /// @brief Fault 2 voltage reading (V) at time of fault.
        ROProperty<int> F2_V;
        /// @brief Fault 2 pool temperature x10 (°C) at time of fault.
        ROProperty<int> F2_PT;
        /// @brief Fault 2 heater temperature x10 (°C) at time of fault.
        ROProperty<int> F2_HT;
        /// @brief Fault 2 case temperature x10 (°C) at time of fault.
        ROProperty<int> F2_CT;
        /// @brief Fault 2 pump state at time of fault.
        ROProperty<int> F2_PU;
        /// @brief Fault 2 system status at time of fault.
        ROProperty<int> F2_ST;
        /// @brief Fault 2 variable element state at time of fault.
        ROProperty<bool> F2_VE;
        /// @brief Fault 3 hour-of-day at time of fault.
        ROProperty<int> F3_HR;
        /// @brief Fault 3 time-of-day encoded as h*256+m.
        ROProperty<int> F3_Time;
        /// @brief Fault 3 error code.
        ROProperty<int> F3_ER;
        /// @brief Fault 3 current reading x10 (A) at time of fault.
        ROProperty<int> F3_I;
        /// @brief Fault 3 voltage reading (V) at time of fault.
        ROProperty<int> F3_V;
        /// @brief Fault 3 pool temperature x10 (°C) at time of fault.
        ROProperty<int> F3_PT;
        /// @brief Fault 3 heater temperature x10 (°C) at time of fault.
        ROProperty<int> F3_HT;
        /// @brief Fault 3 case temperature x10 (°C) at time of fault.
        ROProperty<int> F3_CT;
        /// @brief Fault 3 pump state at time of fault.
        ROProperty<int> F3_PU;
        /// @brief Fault 3 system status at time of fault.
        ROProperty<int> F3_ST;
        /// @brief Fault 3 variable element state at time of fault.
        ROProperty<bool> F3_VE;
        // RC
        /// @brief Air blower operating mode.
        /// @details Read/write. 0=Variable, 1=Ramp, 2=Off.
        RWProperty<int> Outlet_Blower{this, &SpaInterface::setOutlet_Blower, Outlet_Blower_Map};
        // RE heatpump
        /// @brief Heat pump unit installed (0=No, 1=Yes).
        ROProperty<int> HP_Present;
        /// @brief Heat pump ambient air temperature x10 (°C).
        ROProperty<int> HP_Ambient;
        /// @brief Heat pump condenser temperature x10 (°C).
        ROProperty<int> HP_Condensor;
        /// @brief Heat pump operating state code.
        ROProperty<int> HP_State;
        /// @brief Heat pump mode (0=Auto, 1=Heat, 2=Cool, 3=Off).
        ROProperty<int> HP_Mode;
        /// @brief Heat pump defrost cycle timer (minutes).
        ROProperty<int> HP_Defrost_Timer;
        /// @brief Heat pump compressor cumulative run timer (minutes).
        ROProperty<int> HP_Comp_Run_Timer;
        /// @brief Heat pump low ambient temperature protection timer (minutes).
        ROProperty<int> HP_Low_Temp_Timer;
        /// @brief Heat pump heat accumulation timer (minutes).
        ROProperty<int> HP_Heat_Accum_Timer;
        /// @brief Heat pump start sequence timer (seconds).
        ROProperty<int> HP_Sequence_Timer;
        /// @brief Heat pump warning/fault code. 0 = no warning.
        ROProperty<int> HP_Warning;
        /// @brief Freeze protection activation timer (minutes).
        ROProperty<int> FrezTmr;
        /// @brief Defrost cycle start temperature x10 (°C).
        ROProperty<int> DBGN;
        /// @brief Defrost cycle end temperature x10 (°C).
        ROProperty<int> DEND;
        /// @brief Defrost compressor run time limit (minutes).
        ROProperty<int> DCMP;
        /// @brief Defrost maximum duration (minutes).
        ROProperty<int> DMAX;
        /// @brief Defrost element activation delay (minutes).
        ROProperty<int> DELE;
        /// @brief Defrost pump operating mode.
        ROProperty<int> DPMP;
        /// @brief True when heat pump compressor is running.
        ROProperty<bool> HP_Compressor_State;
        /// @brief True when heat pump fan is running.
        ROProperty<bool> HP_Fan_State;
        /// @brief True when heat pump 4-way reversing valve is active (cool mode).
        ROProperty<bool> HP_4W_Valve;
        /// @brief True when heat pump auxiliary heater element is active.
        ROProperty<bool> HP_Heater_State;

        // RG
        /// @brief Pump 1 installation configuration string.
        /// @details Format "I-S-PPP" where I=installed (1/0), S=speed type (1=single, 2=dual),
        /// PPP=possible states (e.g. "014" = Off/On/Auto). Example: "1-1-014".
        ROProperty<String> Pump1InstallState;
        /// @brief Pump 2 installation configuration string. See Pump1InstallState for format.
        ROProperty<String> Pump2InstallState;
        /// @brief Pump 3 installation configuration string. See Pump1InstallState for format.
        ROProperty<String> Pump3InstallState;
        /// @brief Pump 4 installation configuration string. See Pump1InstallState for format.
        ROProperty<String> Pump4InstallState;
        /// @brief Pump 5 installation configuration string. See Pump1InstallState for format.
        ROProperty<String> Pump5InstallState;
        /// @brief True when pump 1 is in a safe state to start.
        ROProperty<bool> Pump1OkToRun;
        /// @brief True when pump 2 is in a safe state to start.
        ROProperty<bool> Pump2OkToRun;
        /// @brief True when pump 3 is in a safe state to start.
        ROProperty<bool> Pump3OkToRun;
        /// @brief True when pump 4 is in a safe state to start.
        ROProperty<bool> Pump4OkToRun;
        /// @brief True when pump 5 is in a safe state to start.
        ROProperty<bool> Pump5OkToRun;
        /// @brief Keypad lock mode.
        /// @details Read/write. 0=Unlocked, 1=Partially Locked, 2=Locked.
        RWProperty<int> LockMode{this, &SpaInterface::setLockMode, LockMode_Map};

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

        /// @brief Unified array of RWProperty pointers for each migrated pump, used for
        /// both reading state and sending commands. Grows as pumps are migrated.
        using PumpStatus = RWProperty<int> SpaInterface::*;
        static constexpr PumpStatus pumpStatuses[] = {
            &SpaInterface::RB_TP_Pump1,
            &SpaInterface::RB_TP_Pump2,
            &SpaInterface::RB_TP_Pump3,
            &SpaInterface::RB_TP_Pump4,
            &SpaInterface::RB_TP_Pump5,
        };

        using PumpInstallStatePtr = ROProperty<String> SpaInterface::*;
        static constexpr PumpInstallStatePtr pumpInstallStateFunctions[] = {
            &SpaInterface::Pump1InstallState,
            &SpaInterface::Pump2InstallState,
            &SpaInterface::Pump3InstallState,
            &SpaInterface::Pump4InstallState,
            &SpaInterface::Pump5InstallState,
        };
};


#endif
