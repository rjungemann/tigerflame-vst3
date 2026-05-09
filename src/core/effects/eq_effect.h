// TigerFlame - EQ Effect

#pragma once

#include "../effect_chain.h"

namespace TigerFlame {

class EQEffect : public EffectBase {
public:
    EQEffect();
    ~EQEffect() override;
    
    void process(float* left, float* right) override;
    void processBlock(float* left, float* right, int numSamples) override;
    
    EffectType getType() const override { return EffectType::kEQ; }
    
    void setParameter(int paramId, float value) override;
    float getParameter(int paramId) const override;
    
    const char* getName() const override { return "EQ"; }
    
    int getParameterCount() const override { return 6; }
    const char* getParameterName(int paramId) const override;
    
    void reset() override;
    
    enum ParamId {
        kLowGain,
        kMidGain,
        kHighGain,
        kLowFreq,
        kMidFreq,
        kHighFreq,
        kParamCount
    };

private:
    // Gain parameters (-12 to +12 dB)
    float lowGain_ = 0.0f;
    float midGain_ = 0.0f;
    float highGain_ = 0.0f;
    
    // Frequency parameters
    float lowFreq_ = 200.0f;
    float midFreq_ = 1000.0f;
    float highFreq_ = 5000.0f;
    
    // Filter state for each band
    float lowState_[2][2] = {{0}};  // [channel][stage]
    float midState_[2][2] = {{0}};
    float highState_[2][2] = {{0}};
    
    // Apply EQ to a sample
    float applyEQ(float input, int channel);
    
    // Apply low shelf filter
    float applyLowShelf(float input, int channel);
    
    // Apply peaking filter (mid)
    float applyPeaking(float input, float freq, float gain, int channel) const;
    
    // Apply high shelf filter
    float applyHighShelf(float input, int channel);
    
    // Convert dB to linear gain
    float dbToGain(float db) const;
};

} // namespace TigerFlame
