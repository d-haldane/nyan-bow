// Effects.cpp
#include "effects.hpp"
#include <stdio.h>

// Implementations for Effect
// ...

void ADSR::setRelease(float releaseTime) {
    release = fix16_div(fix16_one, fix16_mul(fix16_mul(fix16_from_float(releaseTime), sampleRate), fix16_from_float(0.1f))); // 0.1f is an adjustment factor which can be modified for decay behavior.
}


fix16_t ADSR::process(fix16_t sample) {
    switch (phase) {
        case ATTACK:
            amplitude += attack;
            if (amplitude >= fix16_from_int(1)) {
                amplitude = fix16_from_int(1);
                phase = DECAY;
            }
            break;

        case DECAY:
            amplitude -= decay;
            if (amplitude <= sustain) {
                phase = SUSTAIN;
            }
            break;

        case SUSTAIN:
            // Amplitude remains at sustain level
            break;

        case RELEASE:
            amplitude -= fix16_mul(amplitude, release);
            if (amplitude < fix16_from_float(0.01f)) {
                amplitude = fix16_from_int(0);
                phase = OFF;
            }
            break;




        case OFF:
            amplitude = fix16_from_int(0);
            break;
    }
    // Apply the current amplitude to the sample
    return fix16_mul(sample, amplitude);
}

// Implementations for StateVariableFilter

// StateVariableFilter::StateVariableFilter()
//     : filterControl(fix16_from_int(0)), f(fix16_from_int(0)), q(fix16_from_int(0)),
//       bp(fix16_from_int(0)), lp(fix16_from_int(0)), hp(fix16_from_int(0)) {}

// fix16_t StateVariableFilter::process(fix16_t input) {
//     // State variable filter algorithm
//     fix16_t f1 = f * (fix16_one - (f * f) / 6);
//     hp = input - (lp + q * bp);
//     bp += f1 * hp;
//     lp += f1 * bp;

//     // Return the filtered output based on the control parameter
//     return (filterControl * lp) + ((fix16_one - filterControl) * hp);
// }

// void StateVariableFilter::setFilterType(fix16_t filterControl) {
//     this->filterControl = filterControl;
// }

// void StateVariableFilter::setCutoffFrequency(fix16_t frequency) {
//     this->f = frequency * 2 * fix16_pi; // Assuming the frequency is normalized to the sample rate
// }

// void StateVariableFilter::setResonance(fix16_t resonance) {
//     this->q = resonance;
// }


// // Implementations for additional effect classes...
