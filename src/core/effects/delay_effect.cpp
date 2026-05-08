// TigerFlame - Delay Effect Implementation

#include "delay_effect.h"

namespace TigerFlame {

DelayEffect::DelayEffect() {
    reset();
}

DelayEffect::~DelayEffect() = default;

void DelayEffect::process(float* left, float* right) {
    if (bypass_ || !enabled_) {
        return;
    }
    
    float inputL = *left;
    float inputR = *right;
    
    // Get delayed samples
    int readPosL = (delayPosLeft_ - delayLengthLeft_ + delayBufferLeft_.size()) % delayBufferLeft_.size();
    int readPosR = (delayPosRight_ - delayLengthRight_ + delayBufferRight_.size()) % delayBufferRight_.size();
    
    float delayedL = delayBufferLeft_[readPosL];
    float delayedR = delayBufferRight_[readPosR];
    
    // Apply damping
    delayedL = applyDamping(delayedL, dampStateLeft_);
    delayedR = applyDamping(delayedR, dampStateRight_);
    
    // Calculate feedback
    float feedbackL = delayedL * feedback_;
    float feedbackR = delayedR * feedback_;
    
    // Ping-pong mode
    if (pingPong_) {
        delayBufferLeft_[delayPosLeft_] = inputL + feedbackR;
        delayBufferRight_[delayPosRight_] = inputR + feedbackL;
    } else {
        delayBufferLeft_[delayPosLeft_] = inputL + feedbackL;
        delayBufferRight_[delayPosRight_] = inputR + feedbackR;
    }
    
    // Increment positions
    delayPosLeft_ = (delayPosLeft_ + 1) % delayBufferLeft_.size();
    delayPosRight_ = (delayPosRight_ + 1) % delayBufferRight_.size();
    
    // Mix dry and wet
    *left = inputL + delayedL * wetMix_;
    *right = inputR + delayedR * wetMix_;
}

void DelayEffect::processBlock(float* left, float* right, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        process(&left[i], &right[i]);
    }
}

void DelayEffect::setParameter(int paramId, float value) {
    switch (paramId) {
        case kDelayTimeLeft:
            delayTimeLeft_ = value * 2.0f; // 0-2s
            delayLengthLeft_ = static_cast<int>(delayTimeLeft_ * sampleRate_);
            if (delayBufferLeft_.size() < static_cast<size_t>(delayLengthLeft_ + 1)) {
                delayBufferLeft_.resize(delayLengthLeft_ + 1, 0.0f);
            }
            break;
        case kDelayTimeRight:
            delayTimeRight_ = value * 2.0f;
            delayLengthRight_ = static_cast<int>(delayTimeRight_ * sampleRate_);
            if (delayBufferRight_.size() < static_cast<size_t>(delayLengthRight_ + 1)) {
                delayBufferRight_.resize(delayLengthRight_ + 1, 0.0f);
            }
            break;
        case kFeedback:
            feedback_ = value;
            break;
        case kDamping:
            damping_ = value;
            break;
        case kPingPong:
            pingPong_ = (value > 0.5f);
            break;
        case kTempoSync:
            tempoSync_ = (value > 0.5f);
            break;
    }
}

float DelayEffect::getParameter(int paramId) const {
    switch (paramId) {
        case kDelayTimeLeft: return delayTimeLeft_ / 2.0f;
        case kDelayTimeRight: return delayTimeRight_ / 2.0f;
        case kFeedback: return feedback_;
        case kDamping: return damping_;
        case kPingPong: return pingPong_ ? 1.0f : 0.0f;
        case kTempoSync: return tempoSync_ ? 1.0f : 0.0f;
        default: return 0.0f;
    }
}

const char* DelayEffect::getParameterName(int paramId) const {
    switch (paramId) {
        case kDelayTimeLeft: return "Delay Left";
        case kDelayTimeRight: return "Delay Right";
        case kFeedback: return "Feedback";
        case kDamping: return "Damping";
        case kPingPong: return "Ping-Pong";
        case kTempoSync: return "Tempo Sync";
        default: return "Unknown";
    }
}

void DelayEffect::reset() {
    delayTimeLeft_ = 0.25f;
    delayTimeRight_ = 0.3f;
    feedback_ = 0.5f;
    damping_ = 0.5f;
    pingPong_ = false;
    tempoSync_ = false;
    
    delayLengthLeft_ = static_cast<int>(delayTimeLeft_ * sampleRate_);
    delayLengthRight_ = static_cast<int>(delayTimeRight_ * sampleRate_);
    
    delayBufferLeft_.assign(delayLengthLeft_ + 1, 0.0f);
    delayBufferRight_.assign(delayLengthRight_ + 1, 0.0f);
    
    delayPosLeft_ = 0;
    delayPosRight_ = 0;
    
    dampStateLeft_[0] = dampStateLeft_[1] = 0.0f;
    dampStateRight_[0] = dampStateRight_[1] = 0.0f;
}

float DelayEffect::applyDamping(float input, float* state) const {
    if (damping_ <= 0.0f) {
        return input;
    }
    state[0] = state[0] * 0.8f + input * 0.2f;
    state[1] = state[1] * 0.8f + state[0] * 0.2f;
    return state[1];
}

} // namespace TigerFlame
