#pragma once

#include <vector>
#include <algorithm>
#include <memory>
#include "ledstrip.hpp"


// Base Animation Interface
class AnimationInterface {
public:
    virtual void update(std::vector<uint64_t>& buffer) = 0;
    virtual bool isActive() const = 0;
};

class BaseAnimation {
public:
    virtual void update(std::vector<uint64_t>& buffer) = 0;
    virtual bool isActive() const = 0;
    int getID() const { return id_; }
    
protected:
    int id_;
    static int nextID_;  // This can be used to generate unique IDs
};


class AnimationManager {
public:
    explicit AnimationManager(size_t numLEDs);

    void registerAnimation(AnimationInterface* animation);
    void deregisterAnimation(AnimationInterface* animation);
    const std::vector<uint64_t>& getFinalBuffer() const;
    void updateAll();
    void addAnimation(std::unique_ptr<BaseAnimation> anim) {
        activeAnimations_.push_back(std::move(anim));
    }

private:
    std::vector<uint64_t> finalBuffer;
    std::vector<std::unique_ptr<BaseAnimation>> activeAnimations_;
};

class RandomPulse : public BaseAnimation {
public:
    RandomPulse(LEDStrip& strip) : strip_(strip), currentLED(-1), currentBrightness(0.0f) {
        id_ = nextID_++;
    }

    void update(std::vector<uint64_t>& buffer) override;
    bool isActive() const override {
        return active_;
    }

    void activate() {
        active_ = true;
        // Any other setup for your animation
    }

    void deactivate() {
        active_ = false;
    }

    void triggerPulse() {
        currentLED = rand() % strip_.numLEDs_; // Random LED
        currentBrightness = 0.2f;              // 20% brightness
        active_ = true;                        // Activate the animation
    }


private:
    LEDStrip& strip_;
    int currentLED;
    float currentBrightness;
    bool active_;
    size_t counter_;
};
