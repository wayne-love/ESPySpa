#include "MultiBlinker.h"

// Define the on/off times for each state (-1 to 15)
const LEDPattern LED_PATTERNS[17] = {
    {2000, 2000},   // KNIGHT_RIDER: rainbow cycle
    {UINT_MAX, 0},  // STATE_NONE: Always off
    {100, 100},     // STATE_WIFI_NOT_CONNECTED: Red fast blink
    {1000, 1000},   // STATE_WAITING_FOR_SPA: Yellow slow blink
    {0, 0},         // Reserved
    {500, 500},     // STATE_MQTT_NOT_CONNECTED: Purple medium blink
    {0, 0},         // Reserved
    {0, 0},         // Reserved
    {0, 0},         // Reserved
    {0, 0},         // Reserved
    {0, 0},         // Reserved
    {0, 0},         // Reserved
    {0, 0},         // Reserved
    {0, 0},         // Reserved
    {0, 0},         // Reserved
    {0, 0},         // Reserved
    {0, UINT_MAX}   // STATE_STARTED_WIFI_AP: Always on (Blue)
};

#ifdef USE_RGB_LED

// RGB LED implementation for ESPA_V2

MultiBlinker::MultiBlinker(int rgbPin) : rgbPin(rgbPin), pixel(nullptr) {
    // Defer pixel initialization to start() - constructor runs before Arduino setup()
}

uint32_t MultiBlinker::applyBrightness(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    // Scale RGB values by brightness (0-255)
    return pixel->Color(
        (r * brightness) / 255,
        (g * brightness) / 255,
        (b * brightness) / 255
    );
}

AnimationType MultiBlinker::getAnimationForState(int state) {
    switch (state) {
        case KNIGHT_RIDER:
            return ANIM_RAINBOW;
        case STATE_NONE:
            return ANIM_SOLID;  // Off
        case STATE_WIFI_NOT_CONNECTED:
            return ANIM_HEARTBEAT;  // Urgent attention - heartbeat red
        case STATE_WAITING_FOR_SPA:
            return ANIM_BREATHE;    // Waiting - gentle breathing yellow
        case STATE_MQTT_NOT_CONNECTED:
            return ANIM_SPARKLE;    // Connection issue - sparkle purple
        case STATE_STARTED_WIFI_AP:
            return ANIM_COLOR_BLEND;  // AP mode - blend blue/cyan
        default:
            return ANIM_SOLID;
    }
}

RGBColor MultiBlinker::getColorForState(int state) {
    switch (state) {
        case KNIGHT_RIDER:
            return RGB_CYAN;  // Placeholder, rainbow cycle handles this
        case STATE_NONE:
            return RGB_OFF;
        case STATE_WIFI_NOT_CONNECTED:
            return RGB_RED;
        case STATE_WAITING_FOR_SPA:
            return RGB_YELLOW;
        case STATE_MQTT_NOT_CONNECTED:
            return RGB_PURPLE;
        case STATE_STARTED_WIFI_AP:
            return RGB_BLUE;
        default:
            return RGB_OFF;
    }
}

void MultiBlinker::updateRGB() {
    if (pixel == nullptr) return;

    // Handle STATE_NONE - turn off
    if (currentState == STATE_NONE) {
        pixel->setPixelColor(0, 0, 0, 0);
        pixel->show();
        return;
    }

    // Get animation type for current state and dispatch
    AnimationType anim = getAnimationForState(currentState);
    
    switch (anim) {
        case ANIM_RAINBOW:
            rainbowCycle();
            break;
        case ANIM_BREATHE:
            breatheEffect();
            break;
        case ANIM_HEARTBEAT:
            heartbeatEffect();
            break;
        case ANIM_COLOR_BLEND:
            colorBlendEffect();
            break;
        case ANIM_SPARKLE:
            sparkleEffect();
            break;
        case ANIM_SOLID:
        case ANIM_BLINK:
        default: {
            // Simple solid color with brightness control
            RGBColor color = getColorForState(currentState);
            uint32_t c = applyBrightness(color.r, color.g, color.b, MAX_BRIGHTNESS);
            pixel->setPixelColor(0, c);
            pixel->show();
            break;
        }
    }
}

void MultiBlinker::rainbowCycle() {
    if (pixel == nullptr) return;

    // Smooth rainbow using 16-bit hue with gamma correction
    rainbowHue += HUE_STEP;
    
    // ColorHSV with reduced brightness, gamma-corrected for smoother perception
    uint32_t color = pixel->ColorHSV(rainbowHue, 255, MAX_BRIGHTNESS);
    color = pixel->gamma32(color);
    pixel->setPixelColor(0, color);
    pixel->show();
}

void MultiBlinker::breatheEffect() {
    if (pixel == nullptr) return;
    
    RGBColor color = getColorForState(currentState);
    
    // Smooth breathing: 2 second full cycle (100 frames at 20ms)
    animPhase += 655;  // 65536 / 100 = ~655 for 2 second cycle
    
    // Smooth sine-like curve using 16-bit precision
    uint16_t phase = animPhase;
    uint8_t breath;
    if (phase < 32768) {
        // Rising phase: 0 to 255
        breath = (phase * 255) / 32768;
    } else {
        // Falling phase: 255 to 0
        breath = ((65535 - phase) * 255) / 32768;
    }
    
    // Apply gamma curve for more natural breathing perception
    breath = (breath * breath) / 255;  // Simple gamma approximation
    
    // Scale to max brightness
    breath = (breath * MAX_BRIGHTNESS) / 255;
    
    uint32_t c = applyBrightness(color.r, color.g, color.b, breath);
    pixel->setPixelColor(0, c);
    pixel->show();
}

void MultiBlinker::heartbeatEffect() {
    if (pixel == nullptr) return;
    
    RGBColor color = getColorForState(currentState);
    
    // Heartbeat: bump-bump...pause (~1.5 second cycle)
    animPhase += 875;  // ~1.5 second cycle
    
    // Divide cycle into 8 segments
    uint8_t segment = animPhase >> 13;  // 0-7
    uint16_t segmentPhase = (animPhase & 0x1FFF);  // Position within segment (0-8191)
    uint8_t brightness = 0;
    
    switch (segment) {
        case 0: // First beat rise
            brightness = (segmentPhase * 255) / 8192;
            break;
        case 1: // First beat fall
            brightness = 255 - ((segmentPhase * 255) / 8192);
            break;
        case 2: // Second beat rise
            brightness = (segmentPhase * 255) / 8192;
            break;
        case 3: // Second beat fall
            brightness = 255 - ((segmentPhase * 255) / 8192);
            break;
        default: // Pause (segments 4-7)
            brightness = 0;
            break;
    }
    
    brightness = (brightness * MAX_BRIGHTNESS) / 255;
    
    uint32_t c = applyBrightness(color.r, color.g, color.b, brightness);
    pixel->setPixelColor(0, c);
    pixel->show();
}

void MultiBlinker::colorBlendEffect() {
    if (pixel == nullptr) return;
    
    // Blend between blue and cyan for AP mode
    RGBColor color1 = RGB_BLUE;
    RGBColor color2 = RGB_CYAN;
    
    animPhase += 400;  // ~3 second blend cycle
    
    // Smooth triangle wave using 16-bit precision
    uint16_t phase = animPhase;
    uint8_t blend;
    if (phase < 32768) {
        blend = (phase * 255) / 32768;
    } else {
        blend = ((65535 - phase) * 255) / 32768;
    }
    
    // Interpolate between colors
    uint8_t r = color1.r + ((int)(color2.r - color1.r) * blend / 255);
    uint8_t g = color1.g + ((int)(color2.g - color1.g) * blend / 255);
    uint8_t b = color1.b + ((int)(color2.b - color1.b) * blend / 255);
    
    uint32_t c = applyBrightness(r, g, b, MAX_BRIGHTNESS);
    pixel->setPixelColor(0, c);
    pixel->show();
}

void MultiBlinker::sparkleEffect() {
    if (pixel == nullptr) return;
    
    RGBColor color = getColorForState(currentState);
    
    // Random sparkle with base glow
    animPhase++;
    
    // Base brightness with random sparkle overlay
    uint8_t baseBrightness = MAX_BRIGHTNESS / 3;  // Dim base
    uint8_t sparkle = random(0, MAX_BRIGHTNESS);
    
    // Occasionally do a bright sparkle
    uint8_t brightness;
    if (random(100) < 15) {  // 15% chance of sparkle
        brightness = max(baseBrightness, sparkle);
    } else {
        brightness = baseBrightness + random(0, MAX_BRIGHTNESS / 4);
    }
    
    if (brightness > MAX_BRIGHTNESS) brightness = MAX_BRIGHTNESS;
    
    uint32_t c = applyBrightness(color.r, color.g, color.b, brightness);
    pixel->setPixelColor(0, c);
    pixel->show();
}

void MultiBlinker::setState(int state) {
    if (rgbPin == -1 || state < -1 || state >= 16) {
        return;
    }
    if (state == currentState) {
        return;
    }
    debugD("Changing RGB LED state to: %d\n", state);
    currentState = state;
}

void MultiBlinker::start() {
    if (rgbPin == -1) {
        return;
    }
    // Initialize NeoPixel here (deferred from constructor for ESP32-C6 compatibility)
    if (pixel == nullptr) {
        pixel = new Adafruit_NeoPixel(1, rgbPin, NEO_GRB + NEO_KHZ800);
        pixel->begin();
        pixel->setBrightness(50);  // 0-255, start at ~20% brightness
        pixel->setPixelColor(0, 0, 0, 0);
        pixel->show();
    }
    running = true;
    xTaskCreate(runTask, "MultiBlinkerTask", 4096, this, 1, &taskHandle);
}

void MultiBlinker::stop() {
    if (pixel == nullptr) {
        return;
    }
    running = false;
    if (taskHandle != NULL) {
        vTaskDelete(taskHandle);
        taskHandle = NULL;
    }
    // Turn off LED when stopped
    pixel->setPixelColor(0, 0, 0, 0);
    pixel->show();
}

void MultiBlinker::runTask(void *pvParameters) {
    MultiBlinker *blinker = static_cast<MultiBlinker *>(pvParameters);
    blinker->run();
}

void MultiBlinker::run() {
    while (running) {
        // Use fast 20ms interval for smooth animations (~50fps)
        const int interval = 20;

        if (millis() - lastUpdate >= interval) {
            updateRGB();
            lastUpdate = millis();
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void MultiBlinker::updateLEDs() {
    updateRGB();
}

void MultiBlinker::knightRider() {
    rainbowCycle();
}

#else

// Original multi-LED implementation for ESPA_V1 and other boards

MultiBlinker::MultiBlinker(int led1, int led2, int led3, int led4) {
    ledPins[0] = led1;
    ledPins[1] = led2;
    ledPins[2] = led3;
    ledPins[3] = led4;
    numLeds = 0;
    for (int i = 0; i < 4; i++) {
        if (ledPins[i] != -1) {
            numLeds++;
            pinMode(ledPins[i], OUTPUT);
        }
    }
}

void MultiBlinker::setState(int state) {
    if (numLeds == 0 || state < -1 || state >= 16) {
        return;
    }
    if (state == currentState) {
        return;
    }
    debugD("Changing LED state to: %d\n", state);
    currentState = state;
}

void MultiBlinker::start() {
    if (numLeds == 0) {
        return;
    }
    running = true;
    xTaskCreate(runTask, "MultiBlinkerTask", 4096, this, 1, &taskHandle);
}

void MultiBlinker::stop() {
    if (numLeds == 0) {
        return;
    }
    running = false;
    if (taskHandle != NULL) {
        vTaskDelete(taskHandle);
        taskHandle = NULL;
    }
}

void MultiBlinker::runTask(void *pvParameters) {
    MultiBlinker *blinker = static_cast<MultiBlinker *>(pvParameters);
    blinker->run();
}

void MultiBlinker::run() {
    while (running) {
        int interval = MULTI_BLINKER_INTERVAL;
        if (numLeds == 1) {
            const LEDPattern &pattern = LED_PATTERNS[currentState + 1];
            interval = digitalRead(ledPins[0]) ? pattern.onTime : pattern.offTime;
        }

        if (millis() - lastUpdate >= interval) {
            updateLEDs();
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task from hogging the CPU
    }
}

void MultiBlinker::updateLEDs() {
    lastUpdate = millis();
    if (numLeds == 1) {
        // Blink the single LED using the interval value - where the interval is multiplied by the state (so it gets slower as the state increases)
        bool newState = !digitalRead(ledPins[0]);
        digitalWrite(ledPins[0], newState);
    } else if (numLeds == 4) {
        if (currentState == -1) {
            knightRider();
        } else {
            for (int i = 0; i < 4; i++) {
                digitalWrite(ledPins[i], (currentState & (1 << (3 - i))) ? HIGH : LOW);
            }
        }
    }
}

void MultiBlinker::knightRider() {
    static int direction = 1;
    static int position = 0;

    for (int i = 0; i < 4; i++) {
        digitalWrite(ledPins[i], LOW);
    }
    digitalWrite(ledPins[position], HIGH);

    position += direction;
    if (position == 3 || position == 0) {
        direction = -direction;
        // Keep the end LEDs on for a longer duration
        vTaskDelay(150 / portTICK_PERIOD_MS);
    }
}

#endif // USE_RGB_LED
