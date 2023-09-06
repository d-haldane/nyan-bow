#include "animations.hpp"
#include <algorithm>
#include <stdio.h>

int BaseAnimation::nextID_ = 0;  // initialize static member

void RandomPulse::update(std::vector<uint64_t>& buffer) {
    // Dim the previous LED
    if (!active_) return;  // If the animation isn't active, don't do anything

    if (currentLED != -1) {
        uint64_t newColor = strip_.adjustBrightness(0xFFFFFFFFFFFF, currentBrightness);
        buffer[currentLED] = newColor;

        currentBrightness -= 0.02f;  // decrease brightness by 1%

        if (currentBrightness <= 0.0f) {
            buffer[currentLED] = 0; // Turn off the LED
            currentLED = -1;  // Reset to -1 to select new random LED next time
            active_ = false;  // Deactivate the animation once the pulse completes
        }
    }

}

// AnimationManager Implementation
AnimationManager::AnimationManager(size_t numLEDs) : finalBuffer(numLEDs, 0) {}


void AnimationManager::updateAll() {
    // Collect all active animations' outputs in intermediate buffers
    std::vector<std::vector<uint64_t>> intermediateBuffers;

    // Iterate over active animations, let each one update its own buffer
    for (auto& anim : activeAnimations_) {
        if (anim->isActive()) {
            std::vector<uint64_t> buffer(finalBuffer.size(), 0);
            anim->update(buffer);
            intermediateBuffers.push_back(buffer);
        }
    }

    // Clear the final buffer to blend in the new values
    for (uint64_t& led : finalBuffer) {
        led = 0;
    }

    // Blend the intermediate buffers into the final buffer
    for (const auto& buffer : intermediateBuffers) {
        for (size_t i = 0; i < finalBuffer.size(); ++i) {
            finalBuffer[i] += buffer[i];
        }
    }

    // Cleanup: Remove any animations which are no longer active
    activeAnimations_.erase(
        std::remove_if(activeAnimations_.begin(), activeAnimations_.end(),
                       [](const std::unique_ptr<BaseAnimation>& anim) {
                           return !anim->isActive();
                       }),
        activeAnimations_.end());
}


const std::vector<uint64_t>& AnimationManager::getFinalBuffer() const {
    return finalBuffer;
}
