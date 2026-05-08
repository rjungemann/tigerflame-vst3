// TigerFlame - QSound Effect
// Implementation of the Capcom QSound DL-1425 echo algorithm

#pragma once

#include "../effect_chain.h"

#include <vector>
#include <cstdint>

namespace TigerFlame {

// QSound Effect
// Implements the Capcom QSound DL-1425 echo algorithm
// Used in arcade games like 1942, Ghouls 'n Ghosts, Final Fight, etc.
class QSoundEffect : public EffectBase {
public:
    // Constructor
    QSoundEffect();
    
    // Destructor
    ~QSoundEffect() override;
    
    // EffectBase interface
    void process(float* left, float* right) override;
    void processBlock(float* left, float* right, int numSamples) override;
    
    EffectType getType() const override { return EffectType::kEffectQSound; }
    
    void setParameter(int paramId, float value) override;
    float getParameter(int paramId) const override;
    
    const char* getName() const override { return "QSound"; }
    
    int getParameterCount() const override { return 4; }
    const char* getParameterName(int paramId) const override;
    
    void reset() override;
    
    // QSound-specific parameters
    enum ParamId {
        kDelayTime,      // 0.0-1.0 (57-170ms in QSound mode)
        kFeedback,       // 0.0-1.0
        kDamping,        // 0.0-1.0
        kStereoWidth,   // 0.0-1.0
        kParamCount
    };
    
    // QSound native sample rate (24038 Hz)
    static constexpr double kNativeSampleRate = 24038.0;
    
    // QSound delay range in seconds
    static constexpr double kMinDelayTime = 0.057;  // 57ms
    static constexpr double kMaxDelayTime = 0.170;  // 170ms

private:
    // Parameters
    float delayTime_ = 0.1f;     // seconds
    float feedback_ = 0.5f;      // 0.0-1.0
    float damping_ = 0.5f;       // 0.0-1.0
    float stereoWidth_ = 0.5f;   // 0.0-1.0
    
    // Delay buffers
    std::vector<float> leftDelayBuffer_;
    std::vector<float> rightDelayBuffer_;
    
    // Delay buffer positions
    int leftDelayPos_ = 0;
    int rightDelayPos_ = 0;
    
    // Delay lengths in samples
    int leftDelayLength_ = 0;
    int rightDelayLength_ = 0;
    
    // Haas effect offset (samples)
    int haasOffset_ = 0;
    
    // Damping filter state (cascaded moving average)
    float dampingStateLeft_[8] = {0};
    float dampingStateRight_[8] = {0};
    int dampingStages_ = 4;
    
    // Resampling state for native rate conversion
    double resamplePhase_ = 0.0;
    double resampleIncrement_ = 1.0;
    float lastInputLeft_ = 0.0f;
    float lastInputRight_ = 0.0f;
    
    // Calculate delay length in samples
    int calculateDelayLength(double delayTime, double sampleRate) const;
    
    // Apply damping filter
    float applyDamping(float input, float* state) const;
    
    // Resample to native rate
    float resampleToNative(float input);
    
    // Resample from native rate
    float resampleFromNative(float input);
};

} // namespace TigerFlame
