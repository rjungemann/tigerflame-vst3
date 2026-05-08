// TigerFlame - Reverb Effect
// Simple algorithmic reverb implementation

#pragma once

#include "../effect_chain.h"

#include <vector>
#include <cstdint>

namespace TigerFlame {

// Reverb Effect
// Implements a simple Schroeder-style algorithmic reverb
class ReverbEffect : public EffectBase {
public:
    // Constructor
    ReverbEffect();
    
    // Destructor
    ~ReverbEffect() override;
    
    // EffectBase interface
    void process(float* left, float* right) override;
    void processBlock(float* left, float* right, int numSamples) override;
    
    EffectType getType() const override { return EffectType::kEffectReverb; }
    
    void setParameter(int paramId, float value) override;
    float getParameter(int paramId) const override;
    
    const char* getName() const override { return "Reverb"; }
    
    int getParameterCount() const override { return 3; }
    const char* getParameterName(int paramId) const override;
    
    void reset() override;
    
    // Reverb-specific parameters
    enum ParamId {
        kRoomSize,       // 0.0-1.0 (0-100%)
        kDamping,        // 0.0-1.0 (high frequency damping)
        kPreDelay,       // 0.0-1.0 (0-500ms)
        kParamCount
    };

private:
    // Parameters
    float roomSize_ = 0.5f;
    float damping_ = 0.5f;
    float preDelay_ = 0.05f; // 50ms default
    
    // Comb filter parameters
    static constexpr int kNumCombs = 4;
    static constexpr int kCombLengths[kNumCombs] = {1116, 1357, 1617, 1891};
    std::vector<float> combBuffers_[kNumCombs];
    int combPositions_[kNumCombs] = {0};
    float combFeedback_[kNumCombs] = {0};
    
    // Allpass filter parameters
    static constexpr int kNumAllpasses = 2;
    static constexpr int kAllpassLengths[kNumAllpasses] = {557, 443};
    std::vector<float> allpassBuffers_[kNumAllpasses];
    int allpassPositions_[kNumAllpasses] = {0};
    float allpassFeedback_ = 0.5f;
    
    // Pre-delay buffer
    std::vector<float> preDelayBufferLeft_;
    std::vector<float> preDelayBufferRight_;
    int preDelayPosLeft_ = 0;
    int preDelayPosRight_ = 0;
    int preDelayLength_ = 0;
    
    // Damping filter state
    float dampStateLeft_[2] = {0};
    float dampStateRight_[2] = {0};
    
    // Calculate comb feedback based on room size
    void updateCombFeedback();
    
    // Process comb filter
    float processComb(int index, float input);
    
    // Process allpass filter
    float processAllpass(int index, float input);
    
    // Apply damping (simple lowpass)
    float applyDamping(float input, float* state) const;
};

} // namespace TigerFlame
