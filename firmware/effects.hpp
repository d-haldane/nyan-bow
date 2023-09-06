#pragma once
#include <libfixmath/fixmath.h>

class Effect {
public:
    Effect(fix16_t sampleRateVal) : sampleRate(sampleRateVal) {} // Constructor
    virtual fix16_t process(fix16_t sample) = 0; // Pure virtual function

protected:
    fix16_t sampleRate;
};


class ADSR : public Effect {
public:
    // Notice how the sampleRate is passed to the base constructor
    ADSR(fix16_t sampleRateVal, fix16_t attack, fix16_t decay, fix16_t sustain, fix16_t releaseTime)
        : Effect(sampleRateVal), attack(fix16_div(fix16_one, fix16_mul(attack, sampleRateVal))), decay(fix16_div(fix16_one, fix16_mul(decay, sampleRateVal))), 
          sustain(sustain) {
        release = fix16_div(fix16_from_int(1), fix16_mul(fix16_mul(releaseTime, sampleRate), fix16_from_float(0.1f)));
        amplitude = fix16_from_int(0);
        phase = OFF;
    }

    void setAttack(float attackValue) {attack = fix16_div(fix16_from_float(attackValue), sampleRate);}
    void setDecay(float decayValue) { decay = fix16_div(fix16_from_float(decayValue), sampleRate); }
    void setSustain(float sustainValue) { sustain = fix16_from_float(sustainValue); }
    void setRelease(float releaseTime);
    virtual fix16_t process(fix16_t sample) override;
    void noteOn() {phase = ATTACK;}
    void noteOff() {phase = RELEASE; }

private:
    enum Phase { ATTACK, DECAY, SUSTAIN, RELEASE, OFF };

    fix16_t attack, decay, sustain, release;
    fix16_t amplitude; // Current amplitude
    Phase phase;       // Current phase of the envelope
};

class StateVariableFilter : public Effect {
public:
    // Notice the added sampleRateVal parameter to the constructor
    StateVariableFilter(fix16_t sampleRateVal)
        : Effect(sampleRateVal), filterControl(fix16_from_int(0)), f(fix16_from_int(0)), q(fix16_from_int(0)),
          bp(fix16_from_int(0)), lp(fix16_from_int(0)), hp(fix16_from_int(0)) {}

    // Implement the pure virtual function from the base class
    fix16_t process(fix16_t input) override;

    // Set filter type and control parameters
    void setFilterType(fix16_t filterControl);
    void setCutoffFrequency(fix16_t frequency);
    void setResonance(fix16_t resonance);

private:
    // Private members for the StateVariableFilter
    fix16_t filterControl; // Filter control parameter (low-pass, band-pass, high-pass)
    fix16_t f;             // Frequency control parameter
    fix16_t q;             // Resonance control parameter
    fix16_t bp;            // Band-pass state
    fix16_t lp;            // Low-pass state
    fix16_t hp;            // High-pass state
};
