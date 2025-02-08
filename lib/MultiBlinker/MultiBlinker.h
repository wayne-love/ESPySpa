#ifndef MULTIBLINKER_H
#define MULTIBLINKER_H

#include <Arduino.h>
#include <RemoteDebug.h>

extern RemoteDebug Debug;

// These are the four LEDs on the PCB
#ifdef SPACTRLPCB
    const int PCB_LED1 = 14;
    const int PCB_LED2 = 41;
    const int PCB_LED3 = 42;
    const int PCB_LED4 = 43;
#endif

// Define error state constants
                                                // LED State:
const int KNIGHT_RIDER = -1;                    // Knight Rider animation or 2000ms blink
const int STATE_NONE = 0;                       // ON: (nothing)
const int STATE_STARTED_WIFI_AP     = 15;       // ON: ALL or solid on
const int STATE_WIFI_NOT_CONNECTED  = 1;        // ON: LED 4 or 100ms blink
const int STATE_WAITING_FOR_SPA     = 2;        // ON: LED 3 or 1000ms blink
const int STATE_MQTT_NOT_CONNECTED  = 4;        // ON: LED 2 or 500ms blink

const int MULTI_BLINKER_INTERVAL = 100;

struct LEDPattern {
    u_int offTime; // Time in milliseconds the LED is off
    u_int onTime;  // Time in milliseconds the LED is on
};

class MultiBlinker {
public:
    MultiBlinker(int led1 = -1, int led2 = -1, int led3 = -1, int led4 = -1);
    void setState(int state);
    void start();
    void stop();

private:
    static void runTask(void *pvParameters);
    void run();
    void updateLEDs();
    void knightRider();

    int ledPins[4];
    int numLeds;
    int currentState = STATE_NONE;
    bool running = false;
    ulong lastUpdate = 0;
    TaskHandle_t taskHandle = NULL;
};

#endif // MULTIBLINKER_H
