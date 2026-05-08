// TigerFlame - QSound Effect Implementation

#include "qsound_effect.h"

#include <cmath>
#include <algorithm>

namespace TigerFlame {

// Constructor
QSoundEffect::QSoundEffect() {
    reset();
}

// Destructor
QSoundEffect::~QSoundEffect() = default;

// Process single sample
void QSoundEffect::process(float* left, float* right) {
    if (bypass_ || !enabled_) {
        return;
    }
    
    // Get input samples
    float inputL = *left;
    float inputR = *right;
    
    // Convert to native sample rate for processing
    float nativeL = resampleToNative(inputL);
    float nativeR = resampleToNative(inputR);
    
    // Calculate Haas effect offset (right channel delayed)
    // In QSound, the right channel is delayed by a fixed amount for stereo widening
    int rightOffset = static_cast<int>(haasOffset_ * stereoWidth_);
    
    // Get delayed samples
    int leftReadPos = (leftDelayPos_ - leftDelayLength_ + leftDelayBuffer_.size()) % leftDelayBuffer_.size();
    int rightReadPos = (rightDelayPos_ - rightDelayLength_ + rightDelayBuffer_.size()) % rightDelayBuffer_.size();
    
    float delayedL = leftDelayBuffer_[leftReadPos];
    float delayedR = rightDelayBuffer_[rightReadPos];
    
    // Apply damping to delayed signal
    delayedL = applyDamping(delayedL, dampingStateLeft_);
    delayedR = applyDamping(delayedR, dampingStateRight_);
    
    // Calculate echo output
    float echoL = delayedL * feedback_;
    float echoR = delayedR * feedback_;
    
    // Write to delay buffers (with Haas offset)
    leftDelayBuffer_[leftDelayPos_] = nativeL + echoL;
    rightDelayBuffer_[rightDelayPos_] = nativeR + echoR;
    
    // Increment positions
    leftDelayPos_ = (leftDelayPos_ + 1) % leftDelayBuffer_.size();
    rightDelayPos_ = (rightDelayPos_ + 1) % rightDelayBuffer_.size();
    
    // Mix dry and wet signals
    float outputL = inputL + delayedL * wetMix_;
    float outputR = inputR + delayedR * wetMix_;
    
    // Convert back from native rate
    *left = resampleFromNative(outputL);
    *right = resampleFromNative(outputR);
}

// Process block of samples
void QSoundEffect::processBlock(float* left, float* right, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        process(&left[i], &right[i]);
    }
}

// Set parameter
void QSoundEffect::setParameter(int paramId, float value) {
    switch (paramId) {
        case kDelayTime:
            delayTime_ = kMinDelayTime + value * (kMaxDelayTime - kMinDelayTime);
            break;
        case kFeedback:
            feedback_ = std::max(0.0f, std::min(value, 0.99f));
            break;
        case kDamping:
            damping_ = std::max(0.0f, std::min(value, 1.0f));
            dampingStages_ = static_cast<int>(1 + damping_ * 7);
            break;
        case kStereoWidth:
            stereoWidth_ = std::max(0.0f, std::min(value, 1.0f));
            break;
    }
    
    // Recalculate delay lengths
    leftDelayLength_ = calculateDelayLength(delayTime_, sampleRate_);
    rightDelayLength_ = calculateDelayLength(delayTime_, sampleRate_);
    
    // Haas offset is a fixed number of samples at native rate
    // Typical QSound Haas delay is about 30 samples at 24038 Hz
    haasOffset_ = static_cast<int>(30 * sampleRate_ / kNativeSampleRate);
    
    // Resize delay buffers if needed
    int maxDelaySamples = calculateDelayLength(kMaxDelayTime, sampleRate_);
    if (leftDelayBuffer_.size() < static_cast<size_t>(maxDelaySamples)) {
        leftDelayBuffer_.resize(maxDelaySamples * 2, 0.0f);
        rightDelayBuffer_.resize(maxDelaySamples * 2, 0.0f);
    }
}

// Get parameter
float QSoundEffect::getParameter(int paramId) const {
    switch (paramId) {
        case kDelayTime:
            return (delayTime_ - kMinDelayTime) / (kMaxDelayTime - kMinDelayTime);
        case kFeedback:
            return feedback_;
        case kDamping:
            return damping_;
        case kStereoWidth:
            return stereoWidth_;
        default:
            return 0.0f;
    }
}

// Get parameter name
const char* QSoundEffect::getParameterName(int paramId) const {
    switch (paramId) {
        case kDelayTime: return "Delay Time";
        case kFeedback: return "Feedback";
        case kDamping: return "Damping";
        case kStereoWidth: return "Stereo Width";
        default: return "Unknown";
    }
}

// Reset
void QSoundEffect::reset() {
    delayTime_ = 0.1f;
    feedback_ = 0.5f;
    damping_ = 0.5f;
    stereoWidth_ = 0.5f;
    
    leftDelayPos_ = 0;
    rightDelayPos_ = 0;
    
    leftDelayLength_ = calculateDelayLength(delayTime_, sampleRate_);
    rightDelayLength_ = calculateDelayLength(delayTime_, sampleRate_);
    
    haasOffset_ = static_cast<int>(30 * sampleRate_ / kNativeSampleRate);
    
    dampingStages_ = 4;
    
    for (int i = 0; i < 8; ++i) {
        dampingStateLeft_[i] = 0.0f;
        dampingStateRight_[i] = 0.0f;
    }
    
    resamplePhase_ = 0.0;
    resampleIncrement_ = kNativeSampleRate / sampleRate_;
    lastInputLeft_ = 0.0f;
    lastInputRight_ = 0.0f;
    
    int maxDelaySamples = calculateDelayLength(kMaxDelayTime, sampleRate_);
    leftDelayBuffer_.assign(maxDelaySamples * 2, 0.0f);
    rightDelayBuffer_.assign(maxDelaySamples * 2, 0.0f);
}

// Calculate delay length in samples
int QSoundEffect::calculateDelayLength(double delayTime, double sampleRate) const {
    return static_cast<int>(delayTime * sampleRate);
}

// Apply damping filter (cascaded moving average)
float QSoundEffect::applyDamping(float input, float* state) const {
    // Simple lowpass filter
    // In QSound, damping is implemented as a cascaded moving average
    // This is a simplified version
    
    if (damping_ <= 0.0f) {
        return input;
    }
    
    float filtered = input;
    float dampingAmount = 0.5f + 0.5f * damping_; // 0.5 to 1.0
    
    for (int i = 0; i < dampingStages_; ++i) {
        state[i] = state[i] * dampingAmount + filtered * (1.0f - dampingAmount);
        filtered = state[i];
    }
    
    return filtered;
}

// Resample to native rate (for internal processing)
float QSoundEffect::resampleToNative(float input) {
    // No resampling for now - process at host rate
    // For authentic QSound, we'd resample to 24038 Hz
    return input;
}

// Resample from native rate
float QSoundEffect::resampleFromNative(float input) {
    return input;
}

} // namespace TigerFlame
