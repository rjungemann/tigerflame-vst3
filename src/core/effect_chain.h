// TigerFlame - Effect Chain
// Manages a chain of audio effects (QSound, Reverb, Delay, etc.)

#pragma once

#include "parameter_model.h"
#include "stream.h"

#include <vector>
#include <memory>
#include <cstdint>

namespace TigerFlame {

// Effect slot configuration
struct EffectSlotConfig {
    EffectType type = EffectType::kEffectNone;
    bool enabled = true;
    bool bypass = false;
    float wetMix = 0.5f;       // 0.0 = dry only, 1.0 = wet only
    float volume = 1.0f;       // Overall volume for this effect
    
    // Effect-specific parameters
    // QSound
    float qsoundDelayTime = 0.1f;    // 57-170ms
    float qsoundFeedback = 0.5f;     // 0.0-1.0
    float qsoundDamping = 0.5f;     // 0.0-1.0
    float qsoundStereoWidth = 0.5f; // 0.0-1.0
    
    // Reverb
    float reverbRoomSize = 0.5f;    // 0.0-1.0
    float reverbDamping = 0.5f;     // 0.0-1.0
    float reverbPreDelay = 0.05f;   // 0.0-0.5s
    
    // Delay
    float delayTimeLeft = 0.25f;     // 0.0-2.0s
    float delayTimeRight = 0.3f;    // 0.0-2.0s
    float delayFeedback = 0.5f;     // 0.0-1.0
    float delayDamping = 0.5f;      // 0.0-1.0
    bool delayPingPong = false;
    bool delayTempoSync = false;
    float delayTempo = 120.0f;      // BPM
    
    // Distortion
    float distortionDrive = 0.5f;   // 0.0-1.0
    float distortionTone = 0.5f;    // 0.0-1.0
    float distortionWetMix = 0.5f;  // 0.0-1.0
    
    // EQ
    float eqLowGain = 0.0f;         // -12 to +12 dB
    float eqMidGain = 0.0f;         // -12 to +12 dB
    float eqHighGain = 0.0f;        // -12 to +12 dB
    float eqLowFreq = 200.0f;       // Hz
    float eqMidFreq = 1000.0f;      // Hz
    float eqHighFreq = 5000.0f;     // Hz
};

// Effect base class
class EffectBase {
public:
    virtual ~EffectBase() = default;
    
    // Process audio
    virtual void process(float* left, float* right) = 0;
    
    // Process block of samples
    virtual void processBlock(float* left, float* right, int numSamples) = 0;
    
    // Get effect type
    virtual EffectType getType() const = 0;
    
    // Set parameters
    virtual void setParameter(int paramId, float value) = 0;
    virtual float getParameter(int paramId) const = 0;
    
    // Enable/disable
    virtual void setEnabled(bool enabled) { enabled_ = enabled; }
    virtual bool isEnabled() const { return enabled_; }
    
    // Bypass
    virtual void setBypass(bool bypass) { bypass_ = bypass; }
    virtual bool isBypassed() const { return bypass_; }
    
    // Get effect name
    virtual const char* getName() const = 0;
    
    // Get parameter count
    virtual int getParameterCount() const = 0;
    virtual const char* getParameterName(int paramId) const = 0;
    
    // Reset
    virtual void reset() = 0;
    
    // Get wet/dry mix
    virtual void setWetMix(float mix) { wetMix_ = mix; }
    virtual float getWetMix() const { return wetMix_; }
    
    // Set sample rate
    virtual void setSampleRate(double sampleRate) { sampleRate_ = sampleRate; }
    double getSampleRate() const { return sampleRate_; }

protected:
    bool enabled_ = true;
    bool bypass_ = false;
    float wetMix_ = 0.5f;
    double sampleRate_ = 44100.0;
};

// Effect Chain
// Manages a chain of audio effects applied in series
class EffectChain {
public:
    // Constants
    static constexpr int kMaxSlots = 8;
    
    // Constructor
    EffectChain();
    
    // Destructor
    ~EffectChain();
    
    // Set sample rate for all effects
    void setSampleRate(double sampleRate);
    double getSampleRate() const { return sampleRate_; }
    
    // Slot management
    int addEffect(EffectType type, int slotIndex = -1);
    bool removeEffect(int slotIndex);
    void moveEffect(int fromIndex, int toIndex);
    int getEffectCount() const { return static_cast<int>(slots_.size()); }
    int getMaxSlots() const { return kMaxSlots; }
    
    // Effect configuration
    void setEffectType(int slotIndex, EffectType type);
    EffectType getEffectType(int slotIndex) const;
    
    void setEffectEnabled(int slotIndex, bool enabled);
    bool isEffectEnabled(int slotIndex) const;
    
    void setEffectBypass(int slotIndex, bool bypass);
    bool isEffectBypassed(int slotIndex) const;
    
    void setEffectWetMix(int slotIndex, float mix);
    float getEffectWetMix(int slotIndex) const;
    
    // Set effect parameter
    void setEffectParameter(int slotIndex, int paramId, float value);
    float getEffectParameter(int slotIndex, int paramId) const;
    
    // Global controls
    void setGlobalBypass(bool bypass) { globalBypass_ = bypass; }
    bool isGlobalBypassed() const { return globalBypass_; }
    
    void setGlobalWetMix(float mix) { globalWetMix_ = mix; }
    float getGlobalWetMix() const { return globalWetMix_; }
    
    // Process audio through the chain
    void process(float* left, float* right);
    void processBlock(float* left, float* right, int numSamples);
    
    // Reset all effects
    void resetAll();
    void resetSlot(int slotIndex);
    
    // Get slot configuration
    const EffectSlotConfig& getSlotConfig(int slotIndex) const;
    EffectSlotConfig& getSlotConfig(int slotIndex);
    
    // State persistence
    void saveState(IStream* stream) const;
    void restoreState(IStream* stream);
    
    // Get effect name
    static const char* getEffectTypeName(EffectType type);
    
    // Get effect parameter info
    static int getEffectParameterCount(EffectType type);
    static const char* getEffectParameterName(EffectType type, int paramId);

private:
    // Sample rate
    double sampleRate_ = 44100.0;
    
    // Effect slots
    std::vector<EffectSlotConfig> slots_;
    std::vector<EffectBase*> effects_;
    
    // Global settings
    bool globalBypass_ = false;
    float globalWetMix_ = 1.0f; // 0.0 = full dry, 1.0 = full wet
    
    // Create an effect instance
    EffectBase* createEffect(EffectType type);
    
    // Destroy an effect instance
    void destroyEffect(EffectBase* effect);
};

} // namespace TigerFlame
