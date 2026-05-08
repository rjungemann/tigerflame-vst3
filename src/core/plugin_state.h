// TigerFlame - Plugin State
// Manages the normalized state of all plugin parameters

#pragma once

#include "parameter_model.h"
#include "stream.h"

#include <cstdint>
#include <vector>

namespace TigerFlame {

// Plugin state structure
// Stores all parameter values in normalized form (0.0 - 1.0)
struct PluginState {
    // Voice allocation
    float voiceMode = 0.0f; // 0=Mono, 0.333=Layered, 0.666=Polyphonic
    
    // Master parameters
    float masterVolume = 0.8f; // 0.0 - 1.0
    float masterTuning = 0.5f; // 0.0 - 1.0 (-100 to +100 cents)
    float masterPan = 0.5f;    // 0.0 - 1.0 (-100% to +100%)
    
    // Chip selection
    float chipSelect = 0.0f; // 0.0 - 0.875 (8 chip types)
    float chipCount = 0.0f;  // 0.0 - 1.0 (1-8 chips)
    
    // MIDI parameters
    float noteTranspose = 0.5f;     // 0.0 - 1.0 (-48 to +48 semitones)
    float pitchBendRange = 2.0f / 24.0f; // 0.0 - 1.0 (0-24 semitones)
    float velocitySensitivity = 1.0f; // 0.0 - 1.0 (0-200%)
    
    // Voice allocation parameters
    float polyphonyCount = (8.0f - 2.0f) / 30.0f; // 0.0 - 1.0 (2-32 voices)
    float layerCount = 0.0f; // 0.0 - 1.0 (1-8 layers)
    
    // Effect parameters
    float effectChainBypass = 0.0f; // 0.0 - 1.0
    float effect1Type = 0.0f;       // 0.0 - 1.0
    float effect1Bypass = 0.0f;
    float effect2Type = 0.0f;
    float effect2Bypass = 0.0f;
    float effect3Type = 0.0f;
    float effect3Bypass = 0.0f;
    float effect4Type = 0.0f;
    float effect4Bypass = 0.0f;
    
    // Constructor - initialize with defaults
    PluginState() = default;
    
    // Reset to default state
    void reset() {
        *this = PluginState();
    }
    
    // Get normalized value for a parameter
    float getNormalizedValue(ParamId id) const {
        switch (id) {
            case ParamId::kVoiceMode: return voiceMode;
            case ParamId::kMasterVolume: return masterVolume;
            case ParamId::kMasterTuning: return masterTuning;
            case ParamId::kMasterPan: return masterPan;
            case ParamId::kChipSelect: return chipSelect;
            case ParamId::kChipCount: return chipCount;
            case ParamId::kNoteTranspose: return noteTranspose;
            case ParamId::kPitchBendRange: return pitchBendRange;
            case ParamId::kVelocitySensitivity: return velocitySensitivity;
            case ParamId::kPolyphonyCount: return polyphonyCount;
            case ParamId::kLayerCount: return layerCount;
            case ParamId::kEffectChainBypass: return effectChainBypass;
            case ParamId::kEffect1Type: return effect1Type;
            case ParamId::kEffect1Bypass: return effect1Bypass;
            case ParamId::kEffect2Type: return effect2Type;
            case ParamId::kEffect2Bypass: return effect2Bypass;
            case ParamId::kEffect3Type: return effect3Type;
            case ParamId::kEffect3Bypass: return effect3Bypass;
            case ParamId::kEffect4Type: return effect4Type;
            case ParamId::kEffect4Bypass: return effect4Bypass;
            default: return 0.0f;
        }
    }
    
    // Set normalized value for a parameter
    void setNormalizedValue(ParamId id, float value) {
        switch (id) {
            case ParamId::kVoiceMode: voiceMode = value; break;
            case ParamId::kMasterVolume: masterVolume = value; break;
            case ParamId::kMasterTuning: masterTuning = value; break;
            case ParamId::kMasterPan: masterPan = value; break;
            case ParamId::kChipSelect: chipSelect = value; break;
            case ParamId::kChipCount: chipCount = value; break;
            case ParamId::kNoteTranspose: noteTranspose = value; break;
            case ParamId::kPitchBendRange: pitchBendRange = value; break;
            case ParamId::kVelocitySensitivity: velocitySensitivity = value; break;
            case ParamId::kPolyphonyCount: polyphonyCount = value; break;
            case ParamId::kLayerCount: layerCount = value; break;
            case ParamId::kEffectChainBypass: effectChainBypass = value; break;
            case ParamId::kEffect1Type: effect1Type = value; break;
            case ParamId::kEffect1Bypass: effect1Bypass = value; break;
            case ParamId::kEffect2Type: effect2Type = value; break;
            case ParamId::kEffect2Bypass: effect2Bypass = value; break;
            case ParamId::kEffect3Type: effect3Type = value; break;
            case ParamId::kEffect3Bypass: effect3Bypass = value; break;
            case ParamId::kEffect4Type: effect4Type = value; break;
            case ParamId::kEffect4Bypass: effect4Bypass = value; break;
            default: break;
        }
    }
    
    // Get domain value for a parameter
    template<typename T = double>
    T getValue(ParamId id) const {
        const ParameterDescriptor* desc = getParameterDescriptor(id);
        if (!desc) {
            return T(0);
        }
        return static_cast<T>(desc->denormalize(getNormalizedValue(id)));
    }
    
    // Set domain value for a parameter
    template<typename T = double>
    void setValue(ParamId id, T value) {
        const ParameterDescriptor* desc = getParameterDescriptor(id);
        if (!desc) {
            return;
        }
        setNormalizedValue(id, static_cast<float>(desc->normalize(value)));
    }
    
    // Convenience accessors for common parameters
    float getMasterVolume() const { return masterVolume; }
    void setMasterVolume(float value) { masterVolume = std::max(0.0f, std::min(value, 1.0f)); }
    
    float getMasterTuning() const { return masterTuning; }
    void setMasterTuning(float value) { masterTuning = std::max(0.0f, std::min(value, 1.0f)); }
    
    float getPitchBendRange() const { 
        const ParameterDescriptor* desc = getParameterDescriptor(ParamId::kPitchBendRange);
        return desc ? static_cast<float>(desc->denormalize(pitchBendRange)) : 2.0f;
    }
    void setPitchBendRange(float semitones) { 
        const ParameterDescriptor* desc = getParameterDescriptor(ParamId::kPitchBendRange);
        if (desc) {
            pitchBendRange = static_cast<float>(desc->normalize(semitones));
        }
    }
    
    VoiceMode getVoiceMode() const {
        return ParameterConversion::paramToVoiceMode(voiceMode);
    }
    void setVoiceMode(VoiceMode mode) {
        voiceMode = ParameterConversion::voiceModeToParam(mode);
    }
    
    ChipType getChipType() const {
        return ParameterConversion::paramToChipType(chipSelect);
    }
    void setChipType(ChipType type) {
        chipSelect = ParameterConversion::chipTypeToParam(type);
    }
    
    // Save state to stream
    void saveToStream(IStream* stream) const;
    
    // Restore state from stream
    void restoreFromStream(IStream* stream);
    
    // Get all parameter values as a vector
    std::vector<float> toVector() const {
        std::vector<float> values;
        for (int i = 0; i < static_cast<int>(ParamId::kParamCount); ++i) {
            values.push_back(getNormalizedValue(static_cast<ParamId>(i)));
        }
        return values;
    }
    
    // Set from vector
    void fromVector(const std::vector<float>& values) {
        for (int i = 0; i < static_cast<int>(ParamId::kParamCount) && i < static_cast<int>(values.size()); ++i) {
            setNormalizedValue(static_cast<ParamId>(i), values[i]);
        }
    }
};

} // namespace TigerFlame
