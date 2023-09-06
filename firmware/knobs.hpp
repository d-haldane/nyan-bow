#pragma once

#include <vector>
#include <functional>
#include <map>
#include <stdio.h>
#include "hardware/adc.h"
#include "oscillator.hpp"


enum class Mode {
    OSC_MIX_1,
    OSC_MIX_2,
    ADSR_1,
    ADSR_2,
    TOTAL_MODES
};

class Knobs {
public:
    struct Setting {
        std::function<void(float)> function;
        float normalizationFactor = 1.0f; // default normalization
    };

    explicit Knobs(const std::vector<int>& adcPins);

    void mapSettingsToMode(Mode mode, const std::vector<Setting>& settings);
    void scan();

private:
    std::vector<int> adcPins_;
    std::vector<uint16_t> lastKnobValues_;
    std::vector<bool> fineTuningMode_;
    std::vector<uint16_t> modeEntryValues_;
    Mode currentMode_;
    void handleModeChange();

    static constexpr int HYSTERESIS_THRESHOLD = 50;
    static constexpr int MODE_KNOB_INDEX = 0;
    static constexpr int ADC_MAX_VALUE = 4096; 

    std::map<Mode, std::vector<Setting>> settingsMap_;

    uint16_t readADC(int pin);
};
