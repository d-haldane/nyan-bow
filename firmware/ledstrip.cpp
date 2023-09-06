#include "ledstrip.hpp"
#include "hardware/pio.h"
#include <cmath>
#include <algorithm>

LEDStrip::LEDStrip(uint16_t numLEDs, uint8_t pin) : numLEDs_(numLEDs), pin_(pin) {
    leds_ = new uint64_t[numLEDs_];
    for(uint16_t i = 0; i < numLEDs_; ++i) {
        leds_[i] = 0;
    }
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2816_program);
    ws2816_program_init(pio, 0, offset, pin_, 700000);
}

LEDStrip::~LEDStrip() {
    delete[] leds_;
}

void LEDStrip::clear() {
    for (uint16_t i = 0; i < numLEDs_; ++i) {
        leds_[i] = 0x0000000000000000;
    }
}

void LEDStrip::setBrightness(uint16_t ledIndex, float brightness) {
    if (ledIndex >= numLEDs_) return;
    
    uint64_t currentColor = leds_[ledIndex];
    uint64_t red   = (currentColor & 0x000000FF00000000) * brightness;
    uint64_t green = (currentColor & 0x00000000FF000000) * brightness;
    uint64_t blue  = (currentColor & 0x0000000000FF0000) * brightness;
    
    leds_[ledIndex] = red | green | blue;
}

uint64_t LEDStrip::adjustBrightness(uint64_t color, float brightness) {
    uint64_t red   = (color & 0x0000FFFF00000000) >> 32;
    uint64_t green = (color & 0x00000000FFFF0000) >> 16;
    uint64_t blue  = (color & 0x000000000000FFFF);

    red   = std::min(static_cast<uint64_t>(red   * brightness), static_cast<uint64_t>(0xFFFF));
    green = std::min(static_cast<uint64_t>(green * brightness), static_cast<uint64_t>(0xFFFF));
    blue  = std::min(static_cast<uint64_t>(blue  * brightness), static_cast<uint64_t>(0xFFFF));

    return (red << 32) | (green << 16) | blue;
}



void LEDStrip::setColor(uint16_t ledIndex, uint64_t color) {
    if (ledIndex >= numLEDs_) return;
    leds_[ledIndex] = color;
}

void LEDStrip::setAll(uint64_t color) {
    for (uint16_t i = 0; i < numLEDs_; ++i) {
        leds_[i] = color;
    }
}

void LEDStrip::show() {
    for (uint16_t i = 0; i < numLEDs_; ++i) {
        sendPixel(leds_[i]);
    }
}

void LEDStrip::sendPixel(uint64_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, (uint32_t)((pixel_grb << 16) >> 32));
    pio_sm_put_blocking(pio0, 0, (uint32_t)((pixel_grb << 32) >> 32));
    pio_sm_put_blocking(pio0, 0, (uint32_t)((pixel_grb << 48) >> 32));
}
