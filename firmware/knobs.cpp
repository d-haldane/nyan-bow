#include "knobs.hpp"

Knobs::Knobs(const std::vector<int>& adcPins)
    :   adcPins_(adcPins), 
        currentMode_(Mode::OSC_MIX_1), 
        lastKnobValues_(adcPins.size(), 0),
        fineTuningMode_(adcPins.size(), false),
        modeEntryValues_(adcPins.size(), 0) {
}

void Knobs::mapSettingsToMode(Mode mode, const std::vector<Setting>& settings) {
    settingsMap_[mode] = settings;
}

uint16_t Knobs::readADC(int pin) {
    adc_select_input(pin);
    return adc_read();
}

void Knobs::handleModeChange() {
    int modeKnobValue = lastKnobValues_[MODE_KNOB_INDEX];
    int interval = ADC_MAX_VALUE / static_cast<int>(Mode::TOTAL_MODES);

    Mode newMode = static_cast<Mode>(modeKnobValue / interval);

    if (newMode != currentMode_) {
        currentMode_ = newMode;
        for (size_t i = 0; i < lastKnobValues_.size(); ++i) {
            modeEntryValues_[i] = lastKnobValues_[i];
            fineTuningMode_[i] = false;
        }
    }
}

void Knobs::scan() {
    for (size_t i = 0; i < adcPins_.size(); ++i) {
        uint16_t currentValue = readADC(adcPins_[i]);
        if (std::abs(currentValue - modeEntryValues_[i]) > HYSTERESIS_THRESHOLD) {
            fineTuningMode_[i] = true;
            modeEntryValues_[i] = currentValue;
        }
        lastKnobValues_[i] = currentValue;
    }

    handleModeChange();

    for (size_t i = 0; i < adcPins_.size(); ++i) {
        if (fineTuningMode_[i] && i != MODE_KNOB_INDEX) {
            float normalizedValue = static_cast<float>(lastKnobValues_[i]) / ADC_MAX_VALUE;
            if (settingsMap_[currentMode_][i].function) {
                float valueToApply = normalizedValue * settingsMap_[currentMode_][i].normalizationFactor;
                settingsMap_[currentMode_][i].function(valueToApply);
            }
        }
    }
}
