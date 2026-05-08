// TigerFlame - Reverb Effect Implementation

#include "reverb_effect.h"

#include <cmath>

namespace TigerFlame {

// Constructor
ReverbEffect::ReverbEffect() {
    reset();
}

// Destructor
ReverbEffect::~ReverbEffect() = default;

// Process single sample
void ReverbEffect::process(float* left, float* right) {
    if (bypass_ || !enabled_) {
        return;
    }
    
    float inputL = *left;
    float inputR = *right;
    
    // Apply pre-delay
    float preDelayL = preDelayBufferLeft_[preDelayPosLeft_];
    float preDelayR = preDelayBufferRight_[preDelayPosRight_];
    preDelayBufferLeft_[preDelayPosLeft_] = inputL;
    preDelayBufferRight_[preDelayPosRight_] = inputR;
    preDelayPosLeft_ = (preDelayPosLeft_ + 1) % preDelayLength_;
    preDelayPosRight_ = (preDelayPosRight_ + 1) % preDelayLength_;
    
    // Process through comb filters
    float sumL = 0.0f;
    float sumR = 0.0f;
    for (int i = 0; i < kNumCombs; ++i) {
        sumL += processComb(i, preDelayL);
        sumR += processComb(i, preDelayR);
    }
    sumL *= 0.25f; // Scale down
    sumR *= 0.25f;
    
    // Process through allpass filters
    for (int i = 0; i < kNumAllpasses; ++i) {
        sumL = processAllpass(i, sumL);
        sumR = processAllpass(i, sumR);
    }
    
    // Apply damping
    sumL = applyDamping(sumL, dampStateLeft_);
    sumR = applyDamping(sumR, dampStateRight_);
    
    // Mix dry and wet
    *left = inputL + sumL * wetMix_;
    *right = inputR + sumR * wetMix_;
}

// Process block
void ReverbEffect::processBlock(float* left, float* right, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        process(&left[i], &right[i]);
    }
}

// Set parameter
void ReverbEffect::setParameter(int paramId, float value) {
    switch (paramId) {
        case kRoomSize:
            roomSize_ = value;
            updateCombFeedback();
            break;
        case kDamping:
            damping_ = value;
            break;
        case kPreDelay:
            preDelay_ = value * 0.5f; // 0-500ms
            preDelayLength_ = static_cast<int>(preDelay_ * sampleRate_);
            preDelayBufferLeft_.resize(preDelayLength_ + 1, 0.0f);
            preDelayBufferRight_.resize(preDelayLength_ + 1, 0.0f);
            break;
    }
}

// Get parameter
float ReverbEffect::getParameter(int paramId) const {
    switch (paramId) {
        case kRoomSize: return roomSize_;
        case kDamping: return damping_;
        case kPreDelay: return preDelay_ / 0.5f;
        default: return 0.0f;
    }
}

// Get parameter name
const char* ReverbEffect::getParameterName(int paramId) const {
    switch (paramId) {
        case kRoomSize: return "Room Size";
        case kDamping: return "Damping";
        case kPreDelay: return "Pre-Delay";
        default: return "Unknown";
    }
}

// Reset
void ReverbEffect::reset() {
    roomSize_ = 0.5f;
    damping_ = 0.5f;
    preDelay_ = 0.05f;
    
    updateCombFeedback();
    
    preDelayLength_ = static_cast<int>(preDelay_ * sampleRate_);
    preDelayBufferLeft_.assign(preDelayLength_ + 1, 0.0f);
    preDelayBufferRight_.assign(preDelayLength_ + 1, 0.0f);
    preDelayPosLeft_ = 0;
    preDelayPosRight_ = 0;
    
    for (int i = 0; i < kNumCombs; ++i) {
        int length = kCombLengths[i] * static_cast<int>(sampleRate_) / 44100;
        combBuffers_[i].assign(length + 1, 0.0f);
        combPositions_[i] = 0;
    }
    
    for (int i = 0; i < kNumAllpasses; ++i) {
        int length = kAllpassLengths[i] * static_cast<int>(sampleRate_) / 44100;
        allpassBuffers_[i].assign(length + 1, 0.0f);
        allpassPositions_[i] = 0;
    }
    
    dampStateLeft_[0] = dampStateLeft_[1] = 0.0f;
    dampStateRight_[0] = dampStateRight_[1] = 0.0f;
}

// Update comb feedback
void ReverbEffect::updateCombFeedback() {
    for (int i = 0; i < kNumCombs; ++i) {
        combFeedback_[i] = roomSize_ * 0.9f;
    }
}

// Process comb filter
float ReverbEffect::processComb(int index, float input) {
    int readPos = (combPositions_[index] - combBuffers_[index].size() + 1 + combBuffers_[index].size()) % combBuffers_[index].size();
    float delayed = combBuffers_[index][readPos];
    float output = delayed;
    combBuffers_[index][combPositions_[index]] = input + delayed * combFeedback_[index];
    combPositions_[index] = (combPositions_[index] + 1) % combBuffers_[index].size();
    return output;
}

// Process allpass filter
float ReverbEffect::processAllpass(int index, float input) {
    int readPos = (allpassPositions_[index] - allpassBuffers_[index].size() + 1 + allpassBuffers_[index].size()) % allpassBuffers_[index].size();
    float delayed = allpassBuffers_[index][readPos];
    float output = -input + delayed;
    allpassBuffers_[index][allpassPositions_[index]] = input + delayed * allpassFeedback_;
    allpassPositions_[index] = (allpassPositions_[index] + 1) % allpassBuffers_[index].size();
    return output;
}

// Apply damping
float ReverbEffect::applyDamping(float input, float* state) const {
    if (damping_ <= 0.0f) {
        return input;
    }
    
    // Simple 2-pole lowpass
    state[0] = state[0] * 0.8f + input * 0.2f;
    state[1] = state[1] * 0.8f + state[0] * 0.2f;
    return state[1];
}

} // namespace TigerFlame
