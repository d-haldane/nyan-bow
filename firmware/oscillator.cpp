#include "oscillator.hpp"
#include <cmath>
#include <stdio.h>

const fix16_t SCALE[NUMBER_OF_NOTES] = {
    fix16_from_float(261.63f), // C4
    fix16_from_float(293.66f), // D4
    fix16_from_float(329.63f), // E4
    fix16_from_float(349.23f), // F4
    fix16_from_float(392.00f), // G4
    fix16_from_float(440.00f), // A4
    fix16_from_float(493.88f), // B4
    fix16_from_float(523.25f)  // C5
};


Oscillator::Oscillator(fix16_t sampleRateVal) :  waveform(PULSE), frequency(fix16_from_int(440)), amplitude(fix16_one), 
                                                phase(fix16_from_int(0)), dutyCycle(fix16_from_float(0.5)), 
                                                lfoAmplitude(fix16_from_int(25)), lfoPhase(fix16_from_int(0)), sampleRate(sampleRateVal) {
    increment = fix16_div(fix16_mul(frequency, fix16_from_int(2)), sampleRate);
}

void Oscillator::setWaveform(Waveform wf) {
    waveform = wf;
}

void Oscillator::setDuty(float duty) {
    dutyCycle = fix16_from_float(duty);
    if (dutyCycle < fix16_from_float(0.05)) dutyCycle = fix16_from_float(0.05);
    if (dutyCycle > fix16_from_float(0.95)) dutyCycle = fix16_from_float(0.95);
}

void Oscillator::setFrequency(fix16_t freq) {
    frequency = freq;
    increment = fix16_div(fix16_mul(frequency, fix16_from_int(2)), sampleRate);
}

fix16_t Oscillator::getPhase() {
    return phase;
}

fix16_t Oscillator::getIncrement() {
    return increment;
}

void Oscillator::setAmplitude(float amp) {
    amplitude = fix16_from_float(amp);
}

void Oscillator::setLFOAmplitude(float amp) {
    lfoAmplitude = fix16_from_float(amp);
}

fix16_t Oscillator::nextSample() {
    switch (waveform) {
        case PULSE:
            return generatePulse();
        case SAWTOOTH:
            return generateSawtooth();
        default:
            return fix16_from_int(0);
    }
}

fix16_t Oscillator::generatePulse() {
    // printf("Before Increment: Phase: %f, Increment: %f\n, Amplitude: %f", fix16_to_float(phase), fix16_to_float(increment), fix16_to_float(amplitude));
    phase += increment;
    // printf("After Increment: Phase: %f\n", fix16_to_float(phase));
    if (phase >= fix16_one) phase -= fix16_one;
    fix16_t squareWaveValue = (phase < dutyCycle ? fix16_one : fix16_from_int(-1));
    fix16_t result = fix16_mul(squareWaveValue, amplitude);
    // printf("Result: %f\n", fix16_to_float(result));
    return result;
}

fix16_t Oscillator::generateSawtooth() {
    phase += increment;
    if (phase >= fix16_one) phase -= fix16_one;

    fix16_t result = phase - fix16_from_float(0.5f);
    result = fix16_mul(result, amplitude);
    result = fix16_mul(result, fix16_from_int(2));

    return result;
}

fix16_t Oscillator::sampleLFO() {
    fix16_t lfoIncrement = fix16_div(increment, fix16_from_int(2));
    lfoPhase += lfoIncrement;
    if (lfoPhase >= fix16_one) lfoPhase -= fix16_one;
    fix16_t result = (lfoPhase < fix16_one / 2) ? fix16_one : fix16_from_int(-1);
    result = fix16_mul(result, lfoAmplitude);
    return result;
}
