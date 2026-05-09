// TigerFlame - Distortion Effect

#pragma once

#include "../effect_chain.h"

namespace TigerFlame {

class DistortionEffect : public EffectBase {
public:
    DistortionEffect();
    ~DistortionEffect() override;
    
    void process(float* left, float* right) override;
    void processBlock(float* left, float* right, int numSamples) override;
    
    EffectType getType() const override { return EffectType::kDistortion; }
    
    void setParameter(int paramId, float value) override;
    float getParameter(int paramId) const override;
    
    const char* getName() const override { return "Distortion"; }
    
    int getParameterCount() const override { return 3; }
    const char* getParameterName(int paramId) const override;
    
    void reset() override;
    
    enum ParamId {
        kDrive,
        kTone,
        kWetMix,
        kParamCount
    };

private:
    float drive_ = 0.5f;
    float tone_ = 0.5f;
    float wetMixInternal_ = 0.5f; // Separate from base class wetMix
    
    // Tone filter state
    float toneState_[2] = {0};
    
    // Apply soft clipping distortion
    float applyDistortion(float input) const;
    
    // Apply tone control (simple highpass/lowpass mix)
    float applyTone(float input);
};

} // namespace TigerFlame
