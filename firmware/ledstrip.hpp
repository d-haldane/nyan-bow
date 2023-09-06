#pragma once

#include <cstdint>
#include <vector>
#include "ws2816.pio.h"

class LEDStrip {
public:
    explicit LEDStrip(uint16_t numLEDs, uint8_t pin);
    ~LEDStrip();

    void clear();
    void setBrightness(uint16_t ledIndex, float brightness);
    uint64_t adjustBrightness(uint64_t color, float brightness);
    void setColor(uint16_t ledIndex, uint64_t color);
    void setAll(uint64_t color);
    void show();   // Method to push the current state to the actual LEDs
    void setModeColor(int mode);
    void pulseUpdate();  // If you wish to have an updating method for pulsing without showing it immediately
    uint16_t numLEDs_;

private:
    uint8_t pin_;
    uint64_t* leds_;

    void sendPixel(uint64_t pixel_grb);
};
