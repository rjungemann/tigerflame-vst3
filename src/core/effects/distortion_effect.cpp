// TigerFlame - Distortion Effect Implementation

#include "distortion_effect.h"

#include <cmath>

namespace TigerFlame {

DistortionEffect::DistortionEffect() {
    reset();
}

DistortionEffect::~DistortionEffect() = default;

void DistortionEffect::process(float* left, float* right) {
    if (bypass_ || !enabled_) {
        return;
    }
    
    float inputL = *left;
    float inputR = *right;
    
    // Apply distortion
    float distortedL = applyDistortion(inputL * drive_ * 2.0f);
    float distortedR = applyDistortion(inputR * drive_ * 2.0f);
    
    // Apply tone control
    distortedL = applyTone(distortedL);
    distortedR = applyTone(distortedR);
    
    // Mix dry and wet
    *left = inputL + (distortedL - inputL) * wetMixInternal_ * wetMix_;
    *right = inputR + (distortedR - inputR) * wetMixInternal_ * wetMix_;
}

void DistortionEffect::processBlock(float* left, float* right, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        process(&left[i], &right[i]);
    }
}

void DistortionEffect::setParameter(int paramId, float value) {
    switch (paramId) {
        case kDrive:
            drive_ = value;
            break;
        case kTone:
            tone_ = value;
            break;
        case kWetMix:
            wetMixInternal_ = value;
            break;
    }
}

float DistortionEffect::getParameter(int paramId) const {
    switch (paramId) {
        case kDrive: return drive_;
        case kTone: return tone_;
        case kWetMix: return wetMixInternal_;
        default: return 0.0f;
    }
}

const char* DistortionEffect::getParameterName(int paramId) const {
    switch (paramId) {
        case kDrive: return "Drive";
        case kTone: return "Tone";
        case kWetMix: return "Wet/Dry";
        default: return "Unknown";
    }
}

void DistortionEffect::reset() {
    drive_ = 0.5f;
    tone_ = 0.5f;
    wetMixInternal_ = 0.5f;
    toneState_[0] = toneState_[1] = 0.0f;
}

float DistortionEffect::applyDistortion(float input) const {
    // Soft clipping
    if (input > 1.0f) {
        return 1.0f + (input - 1.0f) / (1.0f + (input - 1.0f));
    } else if (input < -1.0f) {
        return -1.0f - (input + 1.0f) / (1.0f + (-input - 1.0f));
    }
    return input;
}

float DistortionEffect::applyTone(float input) {
    // Simple tone control: mix between lowpass and highpass
    float lowpass = toneState_[0] * 0.9f + input * 0.1f;
    toneState_[0] = lowpass;
    
    float highpass = input - toneState_[1] * 0.9f;
    toneState_[1] = input;
    
    // Mix based on tone parameter (0 = dark, 1 = bright)
    return lowpass * (1.0f - tone_) + highpass * tone_;
}

} // namespace TigerFlame
