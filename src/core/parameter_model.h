// TigerFlame - Parameter Model
// Defines parameter IDs, ranges, and conversion functions for the plugin

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace TigerFlame {

// Core parameter IDs (matches VST3 ParamId enum)
enum class ParamId : int32_t {
    // Voice allocation mode
    kVoiceMode = 0,
    
    // Master parameters
    kMasterVolume,
    kMasterTuning,
    kMasterPan,
    
    // Chip selection
    kChipSelect,
    kChipCount,
    
    // MIDI parameters
    kNoteTranspose,
    kPitchBendRange,
    kVelocitySensitivity,
    
    // Voice allocation parameters
    kPolyphonyCount,
    kLayerCount,
    
    // Effect parameters
    kEffectChainBypass,
    kEffect1Type,
    kEffect1Bypass,
    kEffect2Type,
    kEffect2Bypass,
    kEffect3Type,
    kEffect3Bypass,
    kEffect4Type,
    kEffect4Bypass,
    
    // Total parameter count
    kParamCount,
    
    // Special parameters
    kBypass = 1000,
    kProgram = 1001,
};

// Voice mode enum
enum class VoiceMode : int32_t {
    kMono = 0,
    kLayeredMultitimbral,
    kPolyphonic,
    kVoiceModeCount
};

// Chip type enum (matches mml2vgm chips)
enum class ChipType : int32_t {
    kChipYM2151 = 0,
    kChipYM2612,
    kChipSN76489,
    kChipOPL2,
    kChipOPL3,
    kChipQSound,
    kChipC140,
    kChipPOKEY,
    kChipVRC6,
    kChipCount
};

// Effect type enum
enum class EffectType : int32_t {
    kEffectNone = 0,
    kEffectQSound,
    kEffectReverb,
    kEffectDelay,
    kEffectDistortion,
    kEffectEQ,
    kEffectCount
};

// Parameter descriptor structure
struct ParameterDescriptor {
    ParamId id;
    const char* title;
    const char* shortTitle;
    const char* units;
    double defaultValue;        // Domain value (not normalized)
    double minValue;            // Domain minimum
    double maxValue;            // Domain maximum
    double defaultNormalized;  // Normalized default (0.0 - 1.0)
    bool isDiscrete;            // Has discrete steps
    int32_t stepCount;           // Number of steps (if discrete)
    
    // Conversion functions
    double normalize(double domainValue) const {
        if (maxValue == minValue) return defaultNormalized;
        return (domainValue - minValue) / (maxValue - minValue);
    }
    
    double denormalize(double normalizedValue) const {
        if (maxValue == minValue) return defaultValue;
        return minValue + normalizedValue * (maxValue - minValue);
    }
    
    double snapToStep(double value) const {
        if (!isDiscrete || stepCount <= 1) return value;
        double stepSize = (maxValue - minValue) / (stepCount - 1);
        double steps = std::round((value - minValue) / stepSize);
        return minValue + steps * stepSize;
    }
};

// Parameter conversion helper functions
namespace ParameterConversion {

// Voice Mode conversion
inline VoiceMode paramToVoiceMode(double normalizedValue) {
    int mode = static_cast<int>(normalizedValue * 3);
    return static_cast<VoiceMode>(std::max(0, std::min(mode, 2)));
}

inline double voiceModeToParam(VoiceMode mode) {
    return static_cast<double>(mode) / 2.0;
}

// Chip Type conversion
inline ChipType paramToChipType(double normalizedValue) {
    int type = static_cast<int>(normalizedValue * (static_cast<int>(ChipType::kChipCount) - 1));
    return static_cast<ChipType>(std::max(0, std::min(type, static_cast<int>(ChipType::kChipCount) - 1)));
}

inline double chipTypeToParam(ChipType type) {
    int count = static_cast<int>(ChipType::kChipCount) - 1;
    return static_cast<double>(type) / count;
}

// Effect Type conversion
inline EffectType paramToEffectType(double normalizedValue) {
    int type = static_cast<int>(normalizedValue * (static_cast<int>(EffectType::kEffectCount) - 1));
    return static_cast<EffectType>(std::max(0, std::min(type, static_cast<int>(EffectType::kEffectCount) - 1)));
}

inline double effectTypeToParam(EffectType type) {
    int count = static_cast<int>(EffectType::kEffectCount) - 1;
    return static_cast<double>(type) / count;
}

// Master Volume: 0.0-1.0 normalized -> -20dB to 0dB
inline float normalizedToDb(float normalized) {
    return normalized * 20.0f - 20.0f;
}

inline float dbToNormalized(float db) {
    return (db + 20.0f) / 20.0f;
}

// Master Volume: linear 0.0-1.0
inline float getMasterVolumeLinear(float normalized) {
    return normalized;
}

// Master Tuning: -100 to +100 cents
inline float normalizedToCents(float normalized) {
    return (normalized - 0.5f) * 200.0f;
}

inline float centsToNormalized(float cents) {
    return cents / 200.0f + 0.5f;
}

// Pitch Bend Range: 0-24 semitones
inline float normalizedToPitchBendRange(float normalized) {
    return normalized * 24.0f;
}

inline float pitchBendRangeToNormalized(float semitones) {
    return semitones / 24.0f;
}

// Polyphony Count: 2-32 voices
inline int normalizedToPolyphonyCount(float normalized) {
    return static_cast<int>(2 + normalized * 30); // 2 to 32
}

inline float polyphonyCountToNormalized(int count) {
    return (count - 2) / 30.0f;
}

// Layer Count: 1-8 layers
inline int normalizedToLayerCount(float normalized) {
    return static_cast<int>(1 + normalized * 7); // 1 to 8
}

inline float layerCountToNormalized(int count) {
    return (count - 1) / 7.0f;
}

} // namespace ParameterConversion

// Get parameter descriptor by ID
const ParameterDescriptor* getParameterDescriptor(ParamId id);

// Get all parameter descriptors
const std::vector<ParameterDescriptor>& getAllParameterDescriptors();

} // namespace TigerFlame
