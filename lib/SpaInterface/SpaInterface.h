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
template <typename T, size_t N>
constexpr size_t array_count(const T (&)[N]) { return N; }

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

            // Called by SpaInterface when a fresh value is received from the spa.
            void update(T newValue) {
                _value = newValue;
                _hasValue = true;
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
                throw std::invalid_argument(String("updateFromLabel: unknown label '") + label + "'");
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

        /// @brief configure how often the spa is polled in seconds.
        /// @param SpaPollFrequency
        void setSpaPollFrequency(int updateFrequency);

        /// @brief Complete RF command response in a single string
        Property<String> statusResponse;

        /// @brief Mains current draw x10.
        /// @details Read-only; value 77 represents 7.7A.
        ROProperty<int> MainsCurrent;

        /// @brief Water temperature set point x10.
        /// @details Read/write. Valid range 50..410 (5.0C..41.0C). 
        RWProperty<int> STMP{this, &SpaInterface::setSTMP};

        /// @brief Heatpump operating mode values.
        /// @details 0=Auto, 1=Heat, 2=Cool, 3=Off.
        static constexpr ROProperty<int>::LabelValue HPMP_Map[] = {
            {"Auto", 0},
            {"Heat", 1},
            {"Cool", 2},
            {"Off", 3},
        };
        /// @brief Heatpump operating mode.
        /// @details Read/write wrapper around the private `setHPMP` writer.
        /// Valid range 0..3.
        RWProperty<int> HPMP{this, &SpaInterface::setHPMP, HPMP_Map};

        /// @brief Light effect/mode values.
        /// @details 0=White, 1=Color, 2=Fade, 3=Step, 4=Party.
        static constexpr ROProperty<int>::LabelValue ColorMode_Map[] = {
            {"White", 0},
            {"Color", 1},
            {"Fade", 2},
            {"Step", 3},
            {"Party", 4},
        };
        /// @brief Light effect/mode.
        /// @details Read/write wrapper around the private `setColorMode` writer.
        /// Valid range 0..4.
        RWProperty<int> ColorMode{this, &SpaInterface::setColorMode, ColorMode_Map};

        /// @brief Light brightness.
        /// @details Read/write. Valid range 1..5.
        RWProperty<int> LBRTValue{this, &SpaInterface::setLBRTValue};

        /// @brief Light effect speed values.  This seems dumb but it is used to build the UI dropdowns as we
        /// present light effect speed as a select.
        static constexpr ROProperty<int>::LabelValue LSPDValue_Map[] = {
            {"1", 1},
            {"2", 2},
            {"3", 3},
            {"4", 4},
            {"5", 5},
        };
        /// @brief Light effect speed.
        /// @details Read/write. Valid range 1..5.
        RWProperty<int> LSPDValue{this, &SpaInterface::setLSPDValue, LSPDValue_Map};

        /// @brief Maps hue values in 15-degree buckets to spa color indices.
        /// @details Reverse lookup is ambiguous and returns the first matching hue label.
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
        /// @brief Light color index.
        /// @details Read/write. Valid range 0..31.
        RWProperty<int> CurrClr{this, &SpaInterface::setCurrClr, CurrClr_Map};

        /// @brief Sleep timer day mode values.
        /// @details Shared label/value map for both timers: 128=Off, 127=Everyday, 96=Weekends, 31=Weekdays.
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
        /// @brief Sleep timer 1 day mode bitmap.
        /// @details Read/write. Typical values: 128=Off, 127=Everyday, 96=Weekends, 31=Weekdays.
        RWProperty<int> L_1SNZ_DAY{this, &SpaInterface::setL_1SNZ_DAY, SNZ_DAY_Map};
        /// @brief Sleep timer 2 day mode bitmap.
        /// @details Read/write. Typical values: 128=Off, 127=Everyday, 96=Weekends, 31=Weekdays.
        RWProperty<int> L_2SNZ_DAY{this, &SpaInterface::setL_2SNZ_DAY, SNZ_DAY_Map};
        
        /// @brief Sleep timer 1 start time.
        /// @details Read/write. Valid range 0..5947 encoded as h*256+m (24-hour clock).
        RWProperty<int> L_1SNZ_BGN{this, &SpaInterface::setL_1SNZ_BGN};
        /// @brief Sleep timer 1 finish time.
        /// @details Read/write. Valid range 0..5947 encoded as h*256+m (24-hour clock).
        RWProperty<int> L_1SNZ_END{this, &SpaInterface::setL_1SNZ_END};
        /// @brief Sleep timer 2 start time.
        /// @details Read/write. Valid range 0..5947 encoded as h*256+m (24-hour clock).
        RWProperty<int> L_2SNZ_BGN{this, &SpaInterface::setL_2SNZ_BGN};
        /// @brief Sleep timer 2 finish time.
        /// @details Read/write. Valid range 0..5947 encoded as h*256+m (24-hour clock).
        RWProperty<int> L_2SNZ_END{this, &SpaInterface::setL_2SNZ_END};

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
        /// @brief Aux element (booster) state.
        /// @details Read/write. false=Off, true=On.
        RWProperty<bool> HELE{this, &SpaInterface::setHELE};

        /// @brief Day of week label map. 0=Monday .. 6=Sunday.
        static constexpr ROProperty<int>::LabelValue SpaDayOfWeek_Map[] = {
            {"Monday",    0},
            {"Tuesday",   1},
            {"Wednesday", 2},
            {"Thursday",  3},
            {"Friday",    4},
            {"Saturday",  5},
            {"Sunday",    6},
        };
        /// @brief Current day of week on Spa RTC.
        /// @details Read/write. 0=Monday .. 6=Sunday.
        RWProperty<int> SpaDayOfWeek{this, &SpaInterface::setSpaDayOfWeek, SpaDayOfWeek_Map};

        /// @brief Blower mode label map.
        static constexpr ROProperty<int>::LabelValue Outlet_Blower_Map[] = {
            {"Variable", 0},
            {"Ramp",     1},
            {"Off",      2},
        };
        /// @brief Air blower operating mode.
        /// @details Read/write. 0=Variable, 1=Ramp, 2=Off.
        RWProperty<int> Outlet_Blower{this, &SpaInterface::setOutlet_Blower, Outlet_Blower_Map};

        /// @brief Spa operating mode label map.
        static constexpr ROProperty<int>::LabelValue Mode_Map[] = {
            {"NORM", 0},
            {"ECON", 1},
            {"AWAY", 2},
            {"WEEK", 3},
        };
        /// @brief Spa operating mode.
        /// @details Read/write. 0=NORM, 1=ECON, 2=AWAY, 3=WEEK.
        RWProperty<int> Mode{this, &SpaInterface::setMode, Mode_Map};

        /// @brief To be called by loop function of main sketch.  Does regular updates, etc.
        void loop();

        /// @brief Have we sucessfuly read the registers from the SpaNet controller.
        /// @return 
        bool isInitialised();

        /// @brief Water temperature set point multiplied by 10 (380 = 38.0 actual)
        /// @return
        int getSTMP() { return STMP.get(); }

        /// @brief Set the function to be called when properties have been updated.
        /// @param f 
        void setUpdateCallback(void (*f)());

        /// @brief Clear the call back function.
        void clearUpdateCallback();

        bool setRB_TP_Light(int mode);


        /// @brief Sets the clock on the spa
        /// @param t Time
        /// @return True if successful
        bool setSpaTime(time_t t);

        /// @brief Set the speed of the air blower
        /// @param mode 1 = low, 5 = high
        /// @return True if successful
        bool setVARIValue(int mode);

        /// @brief Set filtration block duration (1,2,3,4,6,8,12,24 hours)
        /// @param duration 
        /// @return 
        bool setFiltBlockHrs(String duration);

        /// @brief Set filtration hours (1 to 24 hours)
        /// @param duration
        bool setFiltHrs(String duration);

        bool setLockMode(int mode);

        /// @brief Unified array of RWProperty pointers for each migrated pump, used for
        /// both reading state and sending commands. Grows as pumps are migrated.
        using PumpStatus = RWProperty<int> SpaInterface::*;
        static PumpStatus pumpStatuses[];
        static const int pumpStatusesCount = 5;
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


#endif
