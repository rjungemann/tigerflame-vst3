// TigerFlame - Delay Effect

#pragma once

#include "../effect_chain.h"

#include <vector>

namespace TigerFlame {

class DelayEffect : public EffectBase {
public:
    DelayEffect();
    ~DelayEffect() override;
    
    void process(float* left, float* right) override;
    void processBlock(float* left, float* right, int numSamples) override;
    
    EffectType getType() const override { return EffectType::kEffectDelay; }
    
    void setParameter(int paramId, float value) override;
    float getParameter(int paramId) const override;
    
    const char* getName() const override { return "Delay"; }
    
    int getParameterCount() const override { return 6; }
    const char* getParameterName(int paramId) const override;
    
    void reset() override;
    
    enum ParamId {
        kDelayTimeLeft,
        kDelayTimeRight,
        kFeedback,
        kDamping,
        kPingPong,
        kTempoSync,
        kParamCount
    };

private:
    float delayTimeLeft_ = 0.25f;
    float delayTimeRight_ = 0.3f;
    float feedback_ = 0.5f;
    float damping_ = 0.5f;
    bool pingPong_ = false;
    bool tempoSync_ = false;
    float tempo_ = 120.0f;
    
    std::vector<float> delayBufferLeft_;
    std::vector<float> delayBufferRight_;
    int delayPosLeft_ = 0;
    int delayPosRight_ = 0;
    int delayLengthLeft_ = 0;
    int delayLengthRight_ = 0;
    
    float dampStateLeft_[2] = {0};
    float dampStateRight_[2] = {0};
    
    float applyDamping(float input, float* state) const;
};

} // namespace TigerFlame
