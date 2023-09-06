

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"

#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

#include <libfixmath/fixmath.h>

#include "oscillator.hpp"
#include "effects.hpp"
#include "animations.hpp"
#include "ledstrip.hpp"
#include "knobs.hpp"



#define SYSTEM_CLOCK_FREQ 176000000
#define SAMPLE_RATE 22050
#define BUFFER_SIZE 256
#define PWM_WRAP 250
#define PWM_PIN 0
#define AUDIO_ENABLE_PIN 1
#define WS2816_PIN 14
#define NUM_LEDS 10

#define KNOB_1 40
#define KNOB_1_ADC 2
#define KNOB_2 38
#define KNOB_2_ADC 4
#define KNOB_3 41
#define KNOB_3_ADC 3
#define KNOB_4 39
#define KNOB_4_ADC 1

volatile fix16_t audio_buffer[2][BUFFER_SIZE];
volatile uint8_t current_buffer = 0;

std::vector<int> adcPins = { /*... your adc pin numbers ...*/ };

// Create the knobs instance
Knobs knobs(adcPins);

uint rows[] = {8, 11};
uint cols[] = {5, 6, 3, 12};
#define rowCount sizeof(rows)/sizeof(rows[0])
#define  colCount sizeof(cols)/sizeof(cols[0])
uint8_t keys[rowCount][colCount] = {0}; // Current state
uint8_t prev_keys[rowCount][colCount] = {0}; // Previous state

// Oscillator instances
Oscillator oscillator[2] = {
    Oscillator(fix16_from_int(SAMPLE_RATE)),
    Oscillator(fix16_from_int(SAMPLE_RATE))
};

ADSR adsr[2] = {
    ADSR(fix16_from_int(SAMPLE_RATE), 0.2f, 0.1f, 0.6f, 0.5f),
    ADSR(fix16_from_int(SAMPLE_RATE), 0.2f, 0.1f, 0.6f, 0.5f)
};

LEDStrip strip(NUM_LEDS, WS2816_PIN);
AnimationManager animationManager(NUM_LEDS);
// mutex_t ui_mutex;


void pwm_interrupt_handler() {
    pwm_clear_irq(pwm_gpio_to_slice_num(PWM_PIN));

    // Generate the sample for the current position using the oscillators
    fix16_t sample = 0;
    for (int i = 0; i < rowCount; ++i) {
        fix16_t oscSample = oscillator[i].nextSample();
        oscSample += oscillator[i].sampleLFO();
        fix16_t envelopeValue = adsr[i].process(oscSample); // Process the ADSR for this oscillator with current oscillator sample
        sample += envelopeValue; // Sum the processed samples
    }

    // Scale the sample to fit the PWM resolution
    uint16_t pwm_value = (fix16_to_int(sample) + 100) * (PWM_WRAP / 200);

    // Set PWM level
    pwm_set_gpio_level(PWM_PIN, pwm_value);
}


// Function to handle key events
void handle_key_event(int row, int col, bool pressed) {
    if (pressed) {
        oscillator[row].setFrequency(SCALE[col]);
        adsr[row].noteOn(); // Call noteOn when the key is pressed
        
        auto pulse = std::make_unique<RandomPulse>(strip);
        pulse->triggerPulse();
        animationManager.addAnimation(std::move(pulse));

    } else {
        adsr[row].noteOff(); // Call noteOff when the key is released
    }
}


void scan_keys() {
        for(int r = 0; r < rowCount; r++) {
            gpio_set_dir(rows[r], GPIO_OUT);
            gpio_put(rows[r], 0);
            sleep_us(1); // Short delay for settling

            for(int c = 0; c < colCount; c++) {
                uint8_t key_state = !gpio_get(cols[c]);
                keys[r][c] = key_state;
                if (keys[r][c] != prev_keys[r][c]) {
                    handle_key_event(r, c, keys[r][c] == 1);
                    prev_keys[r][c] = keys[r][c];
                    }
            }
            gpio_put(rows[r], 1);
            gpio_set_dir(rows[r], GPIO_IN);
            gpio_pull_up(rows[r]);
        }
}

volatile bool shouldScanKeys = false;
volatile bool shouldScanKnobs = false;
volatile bool shouldUpdateAnimation = false;

bool ui_timer_callback(repeating_timer_t *rt) {
    shouldScanKeys = true;
    shouldScanKnobs = true;
    shouldUpdateAnimation = true;
    return true;
}

const float MAX_AMPLITUDE = 25.0f;        // Representing full volume or intensity. Could be 100 if using percentage.

void core1_entry() {

    // Set up the timer interrupt
    static repeating_timer_t ui_timer;
    add_repeating_timer_ms(10, ui_timer_callback, NULL, &ui_timer);
    irq_set_enabled(TIMER_IRQ_0, true);
    adc_init();

    std::vector<int> adcPins = {0, 1, 2, 3};
    Knobs knobs(adcPins);

    // Example settings for the OSC_MIX_1 mode:
    std::vector<Knobs::Setting> oscMix1Settings = {
        { nullptr, 0.0f}, // No setting for the mode knob
        { [&](float value){ oscillator[0].setAmplitude(value); }, MAX_AMPLITUDE},   
        { [&](float value){ oscillator[0].setLFOAmplitude(value); }, MAX_AMPLITUDE},   
        { [&](float value){ oscillator[0].setDuty(value); }, 1.0f},   
        // ... other settings for this mode ...
    };
    knobs.mapSettingsToMode(Mode::OSC_MIX_1, oscMix1Settings);

    std::vector<Knobs::Setting> oscMix2Settings = {
    { nullptr, 0.0f}, // No setting for the mode knob
    { [&](float value){ oscillator[1].setAmplitude(value); }, MAX_AMPLITUDE}, 
    { [&](float value){ oscillator[1].setLFOAmplitude(value); }, MAX_AMPLITUDE},   
    { [&](float value){ oscillator[1].setDuty(value); }, 1.0f},   
    // ... other settings for this mode ...
    };
    knobs.mapSettingsToMode(Mode::OSC_MIX_2, oscMix2Settings);

    std::vector<Knobs::Setting> adsr1Settings = {
    { nullptr, 0.0f}, // No setting for the mode knob
    { [&](float value){ adsr[0].setAttack(value); }, MAX_AMPLITUDE},   
    { [&](float value){ adsr[0].setDecay(value); }, MAX_AMPLITUDE},   
    { [&](float value){ adsr[0].setRelease(value); }, 1.0f},   
    // ... other settings for this mode ...
    };
    knobs.mapSettingsToMode(Mode::OSC_MIX_2, oscMix2Settings);

        // Set up buttons
    // Initialize rows as input with pull-ups
    for (int r = 0; r < rowCount; r++) {
        gpio_init(rows[r]);
        gpio_set_dir(rows[r], GPIO_IN);
    }
    // Initialize cols as input initially
    for (int c = 0; c < colCount; c++) {
        gpio_init(cols[c]);
        gpio_set_dir(cols[c], GPIO_IN);
        gpio_pull_up(cols[c]);
    }

    while (true) {
        if (shouldScanKeys) {
            scan_keys();
            shouldScanKeys = false;
        }

        if (shouldScanKnobs) {
            knobs.scan();
            shouldScanKnobs = false;
        }

        if (shouldUpdateAnimation) {
            animationManager.updateAll();
            const auto& buffer = animationManager.getFinalBuffer();
            for (size_t i = 0; i < NUM_LEDS; ++i) {
                strip.setColor(i, buffer[i]);
            }
            strip.show();
            shouldUpdateAnimation = false;
        }
    }
}

int main() {
    stdio_init_all();
    set_sys_clock_khz(176000, true); 

    // Shuffle the ui off to core1

    printf("\nStart in 5 seconds...\n");
    sleep_ms(2000.0);
    printf("\nStarting soon...\n");
    sleep_ms(200.0);
    printf("\nStarting now...\n");

    multicore_launch_core1(core1_entry);


    // Set up oscillators
    oscillator[0].setFrequency(0); // Example frequency for A4
    oscillator[0].setAmplitude(25.0); // Example amplitude (range 0-100 or any scale you are using)
    oscillator[0].setDuty(0.4); // Example amplitude (range 0-100 or any scale you are using)
    oscillator[0].setWaveform(Oscillator::PULSE);         // Assuming you have an enumeration or constant for waveform types

    // Set up oscillator 1
    oscillator[1].setFrequency(0); // Example frequency for A5
    oscillator[1].setAmplitude(25.0);  // Example amplitude (range 0-100 or any scale you are using)
    oscillator[1].setWaveform(Oscillator::SAWTOOTH);        // Assuming you have an enumeration or constant for waveform types
    oscillator[1].setLFOAmplitude(25.0); // Example amplitude (range 0-100 or any scale you are using)

    adsr[0].setRelease(fix16_from_float(0.5));
    
    // Setup PWM and interrupt

    irq_set_priority(PWM_IRQ_WRAP, 0); // Highest priority
    irq_set_priority(TIMER_IRQ_0, 64); // Lower priority


    int audio_pin_slice = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, PWM_WRAP);
    pwm_config_set_clkdiv(&config, 31.847f); //22kHz, 250 wrap
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    pwm_init(audio_pin_slice, &config, true);

    // Enable Audio
    gpio_init(AUDIO_ENABLE_PIN);
    gpio_set_dir(AUDIO_ENABLE_PIN, GPIO_OUT);
    gpio_put(AUDIO_ENABLE_PIN, 1);

    // Main loop
    while (true) {
    }

    return 0;
}
