// TigerFlame - Effect Chain Implementation

#include "effect_chain.h"
#include "effects/qsound_effect.h"
#include "effects/reverb_effect.h"
#include "effects/delay_effect.h"
#include "effects/distortion_effect.h"
#include "effects/eq_effect.h"
#include "state.h"

#include <algorithm>

namespace TigerFlame {

// Effect type names
static const char* kEffectTypeNames[] = {
    "None",
    "QSound",
    "Reverb",
    "Delay",
    "Distortion",
    "EQ"
};

// Effect Chain Constructor
EffectChain::EffectChain() {
    slots_.resize(kMaxSlots);
    effects_.resize(kMaxSlots, nullptr);
}

// Effect Chain Destructor
EffectChain::~EffectChain() {
    for (auto* effect : effects_) {
        destroyEffect(effect);
    }
    effects_.clear();
}

// Set sample rate
void EffectChain::setSampleRate(double sampleRate) {
    sampleRate_ = sampleRate;
    for (auto* effect : effects_) {
        if (effect) {
            effect->setSampleRate(sampleRate);
        }
    }
}

// Add effect
int EffectChain::addEffect(EffectType type, int slotIndex) {
    // Find first empty slot or use specified index
    if (slotIndex < 0) {
        for (int i = 0; i < kMaxSlots; ++i) {
            if (effects_[i] == nullptr) {
                slotIndex = i;
                break;
            }
        }
        if (slotIndex < 0) {
            return -1; // No empty slots
        }
    }
    
    if (slotIndex >= kMaxSlots) {
        return -1; // Invalid slot
    }
    
    // Remove existing effect at this slot
    destroyEffect(effects_[slotIndex]);
    effects_[slotIndex] = nullptr;
    
    // Create new effect
    EffectBase* effect = createEffect(type);
    if (!effect) {
        return -1;
    }
    
    effect->setSampleRate(sampleRate_);
    effects_[slotIndex] = effect;
    slots_[slotIndex].type = type;
    
    return slotIndex;
}

// Remove effect
bool EffectChain::removeEffect(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= kMaxSlots) {
        return false;
    }
    
    destroyEffect(effects_[slotIndex]);
    effects_[slotIndex] = nullptr;
    slots_[slotIndex] = EffectSlotConfig();
    
    return true;
}

// Move effect
void EffectChain::moveEffect(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= kMaxSlots ||
        toIndex < 0 || toIndex >= kMaxSlots ||
        fromIndex == toIndex) {
        return;
    }
    
    std::swap(effects_[fromIndex], effects_[toIndex]);
    std::swap(slots_[fromIndex], slots_[toIndex]);
}

// Set effect type
void EffectChain::setEffectType(int slotIndex, EffectType type) {
    if (slotIndex < 0 || slotIndex >= kMaxSlots) {
        return;
    }
    
    if (type == slots_[slotIndex].type) {
        return; // Same type, no change needed
    }
    
    // Replace with new effect type
    destroyEffect(effects_[slotIndex]);
    effects_[slotIndex] = nullptr;
    
    EffectBase* effect = createEffect(type);
    if (effect) {
        effect->setSampleRate(sampleRate_);
        effects_[slotIndex] = effect;
    }
    
    slots_[slotIndex].type = type;
}

// Get effect type
EffectType EffectChain::getEffectType(int slotIndex) const {
    if (slotIndex >= 0 && slotIndex < kMaxSlots) {
        return slots_[slotIndex].type;
    }
    return EffectType::kEffectNone;
}

// Set effect enabled
void EffectChain::setEffectEnabled(int slotIndex, bool enabled) {
    if (slotIndex >= 0 && slotIndex < kMaxSlots && effects_[slotIndex]) {
        effects_[slotIndex]->setEnabled(enabled);
        slots_[slotIndex].enabled = enabled;
    }
}

// Is effect enabled
bool EffectChain::isEffectEnabled(int slotIndex) const {
    if (slotIndex >= 0 && slotIndex < kMaxSlots) {
        return slots_[slotIndex].enabled;
    }
    return false;
}

// Set effect bypass
void EffectChain::setEffectBypass(int slotIndex, bool bypass) {
    if (slotIndex >= 0 && slotIndex < kMaxSlots && effects_[slotIndex]) {
        effects_[slotIndex]->setBypass(bypass);
        slots_[slotIndex].bypass = bypass;
    }
}

// Is effect bypassed
bool EffectChain::isEffectBypassed(int slotIndex) const {
    if (slotIndex >= 0 && slotIndex < kMaxSlots) {
        return slots_[slotIndex].bypass;
    }
    return false;
}

// Set effect wet mix
void EffectChain::setEffectWetMix(int slotIndex, float mix) {
    if (slotIndex >= 0 && slotIndex < kMaxSlots && effects_[slotIndex]) {
        effects_[slotIndex]->setWetMix(mix);
        slots_[slotIndex].wetMix = mix;
    }
}

// Get effect wet mix
float EffectChain::getEffectWetMix(int slotIndex) const {
    if (slotIndex >= 0 && slotIndex < kMaxSlots) {
        return slots_[slotIndex].wetMix;
    }
    return 0.5f;
}

// Set effect parameter
void EffectChain::setEffectParameter(int slotIndex, int paramId, float value) {
    if (slotIndex >= 0 && slotIndex < kMaxSlots && effects_[slotIndex]) {
        effects_[slotIndex]->setParameter(paramId, value);
        
        // Also update the slot config for persistence
        EffectSlotConfig& config = slots_[slotIndex];
        switch (config.type) {
            case EffectType::kEffectQSound:
                switch (paramId) {
                    case 0: config.qsoundDelayTime = value; break;
                    case 1: config.qsoundFeedback = value; break;
                    case 2: config.qsoundDamping = value; break;
                    case 3: config.qsoundStereoWidth = value; break;
                }
                break;
            case EffectType::kEffectReverb:
                switch (paramId) {
                    case 0: config.reverbRoomSize = value; break;
                    case 1: config.reverbDamping = value; break;
                    case 2: config.reverbPreDelay = value; break;
                }
                break;
            case EffectType::kEffectDelay:
                switch (paramId) {
                    case 0: config.delayTimeLeft = value; break;
                    case 1: config.delayTimeRight = value; break;
                    case 2: config.delayFeedback = value; break;
                    case 3: config.delayDamping = value; break;
                    case 4: config.delayPingPong = (value > 0.5f); break;
                    case 5: config.delayTempoSync = (value > 0.5f); break;
                }
                break;
            default:
                break;
        }
    }
}

// Get effect parameter
float EffectChain::getEffectParameter(int slotIndex, int paramId) const {
    if (slotIndex >= 0 && slotIndex < kMaxSlots && effects_[slotIndex]) {
        return effects_[slotIndex]->getParameter(paramId);
    }
    return 0.0f;
}

// Process audio through the chain
void EffectChain::process(float* left, float* right) {
    if (globalBypass_) {
        return; // Global bypass - no processing
    }
    
    for (int i = 0; i < kMaxSlots; ++i) {
        if (effects_[i] && effects_[i]->isEnabled() && !effects_[i]->isBypassed()) {
            effects_[i]->process(left, right);
        }
    }
}

// Process block of samples
void EffectChain::processBlock(float* left, float* right, int numSamples) {
    if (globalBypass_) {
        return; // Global bypass - no processing
    }
    
    for (int i = 0; i < kMaxSlots; ++i) {
        if (effects_[i] && effects_[i]->isEnabled() && !effects_[i]->isBypassed()) {
            effects_[i]->processBlock(left, right, numSamples);
        }
    }
}

// Reset all effects
void EffectChain::resetAll() {
    for (auto* effect : effects_) {
        if (effect) {
            effect->reset();
        }
    }
}

// Reset slot
void EffectChain::resetSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < kMaxSlots && effects_[slotIndex]) {
        effects_[slotIndex]->reset();
    }
}

// Get slot configuration
const EffectSlotConfig& EffectChain::getSlotConfig(int slotIndex) const {
    static const EffectSlotConfig kDefaultConfig;
    if (slotIndex >= 0 && slotIndex < kMaxSlots) {
        return slots_[slotIndex];
    }
    return kDefaultConfig;
}

EffectSlotConfig& EffectChain::getSlotConfig(int slotIndex) {
    static EffectSlotConfig kDefaultConfig;
    if (slotIndex >= 0 && slotIndex < kMaxSlots) {
        return slots_[slotIndex];
    }
    return kDefaultConfig;
}

// Save state
void EffectChain::saveState(IStream* stream) const {
    if (!stream) return;
    
    uint32_t magic = 0x45464643; // "EFFC"
    uint32_t version = 1;
    uint32_t size = 0; // Will be calculated
    
    StateUtil::writeChunkHeader(stream, magic, version, size);
    
    // Save global settings
    stream->write(&globalBypass_, sizeof(globalBypass_));
    stream->write(&globalWetMix_, sizeof(globalWetMix_));
    
    // Save sample rate
    stream->write(&sampleRate_, sizeof(sampleRate_));
    
    // Save each slot config
    for (const auto& slot : slots_) {
        stream->write(&slot, sizeof(slot));
    }
}

// Restore state
void EffectChain::restoreState(IStream* stream) {
    if (!stream) return;
    
    uint32_t magic, version, size;
    if (!StateUtil::readChunkHeader(stream, magic, version, size)) {
        return;
    }
    
    if (magic != 0x45464643 || version > 1) {
        return;
    }
    
    // Remove all existing effects
    for (auto* effect : effects_) {
        destroyEffect(effect);
    }
    effects_.assign(kMaxSlots, nullptr);
    
    // Read global settings
    if (stream->read(&globalBypass_, sizeof(globalBypass_)) != StreamResult::kOk) return;
    if (stream->read(&globalWetMix_, sizeof(globalWetMix_)) != StreamResult::kOk) return;
    if (stream->read(&sampleRate_, sizeof(sampleRate_)) != StreamResult::kOk) return;
    
    // Read each slot config and recreate effects
    for (int i = 0; i < kMaxSlots; ++i) {
        if (stream->read(&slots_[i], sizeof(slots_[i])) != StreamResult::kOk) return;
        
        if (slots_[i].type != EffectType::kEffectNone) {
            EffectBase* effect = createEffect(slots_[i].type);
            if (effect) {
                effect->setSampleRate(sampleRate_);
                effect->setWetMix(slots_[i].wetMix);
                effects_[i] = effect;
            }
        }
    }
}

// Get effect type name
const char* EffectChain::getEffectTypeName(EffectType type) {
    int index = static_cast<int>(type);
    if (index >= 0 && index < static_cast<int>(sizeof(kEffectTypeNames) / sizeof(kEffectTypeNames[0]))) {
        return kEffectTypeNames[index];
    }
    return "Unknown";
}

// Get effect parameter count
int EffectChain::getEffectParameterCount(EffectType type) {
    switch (type) {
        case EffectType::kEffectNone: return 0;
        case EffectType::kEffectQSound: return 4;
        case EffectType::kEffectReverb: return 3;
        case EffectType::kEffectDelay: return 6;
        case EffectType::kEffectDistortion: return 3;
        case EffectType::kEffectEQ: return 6;
        default: return 0;
    }
}

// Get effect parameter name
const char* EffectChain::getEffectParameterName(EffectType type, int paramId) {
    switch (type) {
        case EffectType::kEffectQSound:
            switch (paramId) {
                case 0: return "Delay Time";
                case 1: return "Feedback";
                case 2: return "Damping";
                case 3: return "Stereo Width";
                default: return "Unknown";
            }
        case EffectType::kEffectReverb:
            switch (paramId) {
                case 0: return "Room Size";
                case 1: return "Damping";
                case 2: return "Pre-Delay";
                default: return "Unknown";
            }
        case EffectType::kEffectDelay:
            switch (paramId) {
                case 0: return "Delay Left";
                case 1: return "Delay Right";
                case 2: return "Feedback";
                case 3: return "Damping";
                case 4: return "Ping-Pong";
                case 5: return "Tempo Sync";
                default: return "Unknown";
            }
        case EffectType::kEffectDistortion:
            switch (paramId) {
                case 0: return "Drive";
                case 1: return "Tone";
                case 2: return "Wet/Dry";
                default: return "Unknown";
            }
        case EffectType::kEffectEQ:
            switch (paramId) {
                case 0: return "Low Gain";
                case 1: return "Mid Gain";
                case 2: return "High Gain";
                case 3: return "Low Freq";
                case 4: return "Mid Freq";
                case 5: return "High Freq";
                default: return "Unknown";
            }
        default:
            return "Unknown";
    }
}

// Create an effect instance
EffectBase* EffectChain::createEffect(EffectType type) {
    switch (type) {
        case EffectType::kEffectQSound:
            return new QSoundEffect();
        case EffectType::kEffectReverb:
            return new ReverbEffect();
        case EffectType::kEffectDelay:
            return new DelayEffect();
        case EffectType::kEffectDistortion:
            return new DistortionEffect();
        case EffectType::kEffectEQ:
            return new EQEffect();
        default:
            return nullptr;
    }
}

// Destroy an effect instance
void EffectChain::destroyEffect(EffectBase* effect) {
    delete effect;
}

} // namespace TigerFlame
