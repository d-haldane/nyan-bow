#pragma once

#include <libfixmath/fixmath.h>
#include <cstdint>


#define NUMBER_OF_NOTES 8 // Example: 8 notes in an octave

// Define a scale (e.g., C Major scale)
extern const fix16_t SCALE[NUMBER_OF_NOTES];


class Oscillator {
public:
    enum Waveform {
        PULSE,
        SAWTOOTH
    };

    Oscillator(fix16_t sampleRateVal);
    void setWaveform(Waveform waveform);
    void setFrequency(fix16_t frequency);
    void setAmplitude(float amplitude);
    void setLFOAmplitude(float amplitude);
    void setDuty(float duty);
    fix16_t getPhase();
    fix16_t getIncrement();
    fix16_t nextSample();
    fix16_t sampleLFO();

private:
    Waveform waveform;
    fix16_t lfoAmplitude;
    fix16_t lfoPhase;
    fix16_t frequency;
    fix16_t amplitude;
    fix16_t phase;
    fix16_t dutyCycle; // Duty cycle for pulse
    fix16_t increment; // Phase increment per sample
    fix16_t sampleRate; // Phase increment per sample

    fix16_t generatePulse();
    fix16_t generateSawtooth();
};
