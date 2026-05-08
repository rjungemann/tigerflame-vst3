// TigerFlame - EQ Effect Implementation

#include "eq_effect.h"

#include <cmath>

namespace TigerFlame {

EQEffect::EQEffect() {
    reset();
}

EQEffect::~EQEffect() = default;

void EQEffect::process(float* left, float* right) {
    if (bypass_ || !enabled_) {
        return;
    }
    
    *left = applyEQ(*left, 0);
    *right = applyEQ(*right, 1);
}

void EQEffect::processBlock(float* left, float* right, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        process(&left[i], &right[i]);
    }
}

void EQEffect::setParameter(int paramId, float value) {
    switch (paramId) {
        case kLowGain:
            lowGain_ = (value - 0.5f) * 24.0f; // -12 to +12 dB
            break;
        case kMidGain:
            midGain_ = (value - 0.5f) * 24.0f;
            break;
        case kHighGain:
            highGain_ = (value - 0.5f) * 24.0f;
            break;
        case kLowFreq:
            lowFreq_ = 50.0f + value * 400.0f; // 50-450 Hz
            break;
        case kMidFreq:
            midFreq_ = 200.0f + value * 4000.0f; // 200-4200 Hz
            break;
        case kHighFreq:
            highFreq_ = 1000.0f + value * 8000.0f; // 1000-9000 Hz
            break;
    }
}

float EQEffect::getParameter(int paramId) const {
    switch (paramId) {
        case kLowGain: return lowGain_ / 24.0f + 0.5f;
        case kMidGain: return midGain_ / 24.0f + 0.5f;
        case kHighGain: return highGain_ / 24.0f + 0.5f;
        case kLowFreq: return (lowFreq_ - 50.0f) / 400.0f;
        case kMidFreq: return (midFreq_ - 200.0f) / 4000.0f;
        case kHighFreq: return (highFreq_ - 1000.0f) / 8000.0f;
        default: return 0.0f;
    }
}

const char* EQEffect::getParameterName(int paramId) const {
    switch (paramId) {
        case kLowGain: return "Low Gain";
        case kMidGain: return "Mid Gain";
        case kHighGain: return "High Gain";
        case kLowFreq: return "Low Freq";
        case kMidFreq: return "Mid Freq";
        case kHighFreq: return "High Freq";
        default: return "Unknown";
    }
}

void EQEffect::reset() {
    lowGain_ = 0.0f;
    midGain_ = 0.0f;
    highGain_ = 0.0f;
    lowFreq_ = 200.0f;
    midFreq_ = 1000.0f;
    highFreq_ = 5000.0f;
    
    for (int c = 0; c < 2; ++c) {
        for (int s = 0; s < 2; ++s) {
            lowState_[c][s] = 0.0f;
            midState_[c][s] = 0.0f;
            highState_[c][s] = 0.0f;
        }
    }
}

float EQEffect::applyEQ(float input, int channel) {
    float output = input;
    
    // Apply low shelf
    output = applyLowShelf(output, channel);
    
    // Apply mid peaking
    output = applyPeaking(output, midFreq_, midGain_, channel);
    
    // Apply high shelf
    output = applyHighShelf(output, channel);
    
    return output;
}

float EQEffect::applyLowShelf(float input, int channel) {
    // Simplified low shelf filter
    float gain = dbToGain(lowGain_);
    float freq = lowFreq_ / sampleRate_;
    
    // Calculate filter coefficients (simplified)
    float a0 = 1.0f;
    float b1 = -2.0f * std::cos(2.0f * 3.1415926535f * freq) / (1.0f + std::sin(2.0f * 3.1415926535f * freq));
    
    // Apply filter
    float output = input * gain + lowState_[channel][0] * b1;
    lowState_[channel][0] = output;
    
    return output;
}

float EQEffect::applyPeaking(float input, float freq, float gain, int channel) const {
    // Simplified peaking filter
    if (gain == 0.0f) {
        return input;
    }
    
    float g = dbToGain(gain);
    float w0 = 2.0f * 3.1415926535f * freq / sampleRate_;
    
    // Simplified: just apply gain at this frequency band
    // A proper implementation would use biquad filters
    float output = input;
    
    // Very simple approximation
    if (freq < 500.0f) {
        // Low frequency boost/cut
        output = input * (1.0f + (g - 1.0f) * 0.3f);
    } else if (freq < 2000.0f) {
        // Mid frequency boost/cut
        output = input * (1.0f + (g - 1.0f) * 0.5f);
    } else {
        // High frequency boost/cut
        output = input * (1.0f + (g - 1.0f) * 0.3f);
    }
    
    return output;
}

float EQEffect::applyHighShelf(float input, int channel) {
    // Simplified high shelf filter
    float gain = dbToGain(highGain_);
    float freq = highFreq_ / sampleRate_;
    
    // Calculate filter coefficients (simplified)
    float a0 = 1.0f;
    float b1 = -2.0f * std::cos(2.0f * 3.1415926535f * freq) / (1.0f + std::sin(2.0f * 3.1415926535f * freq));
    
    // Apply filter
    float output = input * gain + highState_[channel][0] * b1;
    highState_[channel][0] = output;
    
    return output;
}

float EQEffect::dbToGain(float db) const {
    return std::pow(10.0f, db / 20.0f);
}

} // namespace TigerFlame
