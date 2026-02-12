#ifndef MULTIBLINKER_H
#define MULTIBLINKER_H

/**
 * @file MultiBlinker.h
 * @brief LED status indicator for eSpa hardware variants.
 * 
 * This library provides visual status feedback through LEDs:
 * 
 * - ESPA_V1 (ESP32-S3): Four separate red LEDs in Knight Rider pattern
 * - ESPA_V2 (ESP32-C6): Single WS2812 RGB NeoPixel with animated effects
 * - Generic boards: Single LED on LED_PIN
 * 
 * ESPA_V2 RGB LED Animations:
 * - KNIGHT_RIDER: Smooth rainbow spectrum cycle
 * - STATE_WIFI_NOT_CONNECTED: Red heartbeat (urgent)
 * - STATE_WAITING_FOR_SPA: Yellow breathing (waiting)
 * - STATE_MQTT_NOT_CONNECTED: Purple sparkle (connection issue)
 * - STATE_STARTED_WIFI_AP: Blue/cyan color blend (AP mode)
 * 
 * @note On ESP32-C6, NeoPixel initialization is deferred to start() method
 *       because global constructors run before Arduino framework is ready.
 */

#include <Arduino.h>
#include <RemoteDebug.h>

#if defined(ESPA_V2) && defined(RGB_LED_PIN)
    #include <Adafruit_NeoPixel.h>
    #define USE_RGB_LED
#endif

extern RemoteDebug Debug;

// These are the four LEDs on the PCB (for ESPA_V1 and legacy boards)
#if defined(ESPA_V1)
    const int PCB_LED1 = 14;
    const int PCB_LED2 = 41;
    const int PCB_LED3 = 42;
    const int PCB_LED4 = -1; // GPIO 43 conflicts with USB on ESP32-S3, disabled
#elif !defined(ESPA_V2)
    // Generic ESP32 dev boards - no PCB LEDs defined
#endif

// Define error state constants
                                                // LED State:
const int KNIGHT_RIDER = -1;                    // Knight Rider animation or rainbow cycle
const int STATE_NONE = 0;                       // OFF (nothing)
const int STATE_STARTED_WIFI_AP     = 15;       // Blue solid (AP mode active)
const int STATE_WIFI_NOT_CONNECTED  = 1;        // Red fast blink (100ms)
const int STATE_WAITING_FOR_SPA     = 2;        // Yellow slow blink (1000ms)
const int STATE_MQTT_NOT_CONNECTED  = 4;        // Purple medium blink (500ms)

const int MULTI_BLINKER_INTERVAL = 100;

// RGB Color structure for addressable LEDs
struct RGBColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Color definitions for RGB LED states
const RGBColor RGB_OFF     = {0, 0, 0};
const RGBColor RGB_RED     = {255, 0, 0};       // WiFi not connected
const RGBColor RGB_YELLOW  = {255, 180, 0};     // Waiting for spa
const RGBColor RGB_BLUE    = {0, 0, 255};       // WiFi AP mode
const RGBColor RGB_PURPLE  = {180, 0, 255};     // MQTT not connected
const RGBColor RGB_GREEN   = {0, 255, 0};       // All good (for future use)
const RGBColor RGB_CYAN    = {0, 255, 255};     // Rainbow cycle marker

// Animation types for RGB LED
enum AnimationType {
    ANIM_SOLID,         // Static color
    ANIM_BLINK,         // Simple on/off blink
    ANIM_BREATHE,       // Smooth fade in/out (breathing)
    ANIM_HEARTBEAT,     // Two quick pulses then pause
    ANIM_COLOR_BLEND,   // Smooth transition between two colors
    ANIM_SPARKLE,       // Random brightness variations
    ANIM_RAINBOW        // Full spectrum cycle
};

struct LEDPattern {
    u_int offTime; // Time in milliseconds the LED is off
    u_int onTime;  // Time in milliseconds the LED is on
};

class MultiBlinker {
public:
#ifdef USE_RGB_LED
    MultiBlinker(int rgbPin);
#else
    MultiBlinker(int led1 = -1, int led2 = -1, int led3 = -1, int led4 = -1);
#endif
    void setState(int state);
    void start();
    void stop();

private:
    static void runTask(void *pvParameters);
    void run();
    void updateLEDs();
    void knightRider();

#ifdef USE_RGB_LED
    Adafruit_NeoPixel *pixel;
    int rgbPin;
    void updateRGB();
    void rainbowCycle();
    void breatheEffect();
    void heartbeatEffect();
    void colorBlendEffect();
    void sparkleEffect();
    RGBColor getColorForState(int state);
    AnimationType getAnimationForState(int state);
    uint32_t applyBrightness(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
    
    uint16_t rainbowHue = 0;          // 16-bit for smooth transitions (0-65535)
    uint16_t animPhase = 0;           // Animation phase counter
    uint8_t heartbeatStage = 0;       // Stage in heartbeat animation
    static const uint16_t HUE_STEP = 128;  // Smaller = slower, smoother cycle
    static const uint8_t MAX_BRIGHTNESS = 40;  // Max brightness (0-255), lowered to reduce eye strain
#else
    int ledPins[4];
    int numLeds;
#endif

    int currentState = STATE_NONE;
    bool running = false;
    ulong lastUpdate = 0;
    TaskHandle_t taskHandle = NULL;
};

#endif // MULTIBLINKER_H
