#include "MultiBlinker.h"

// Define the on/off times for each state (-1 to 15)
const LEDPattern LED_PATTERNS[17] = {
    {2000, 2000},   //KNIGHT_RIDER
    {UINT_MAX, 0},  // STATE_NONE: Always off
    {100, 100},     // STATE_WIFI_NOT_CONNECTED
    {1000, 1000},   // STATE_WAITING_FOR_SPA
    {0, 0},         // Reserved
    {500, 500},     // STATE_MQTT_NOT_CONNECTED
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
    {0, UINT_MAX}   // STATE_STARTED_WIFI_AP: Always on
};

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
    xTaskCreate(runTask, "MultiBlinkerTask", 2048, this, 1, &taskHandle);
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
