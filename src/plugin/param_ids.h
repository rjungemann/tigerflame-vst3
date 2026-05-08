// TigerFlame VST3 Plugin - Parameter IDs
// Maps core parameter IDs to VST3 ParamID values

#pragma once

#include "pluginterfaces/vst/vsttypes.h"

namespace TigerFlame {

// Core parameter IDs (from Core::ParamId enum)
enum class ParamId : Steinberg::ParamID {
    // Voice allocation mode
    kVoiceMode = 0,
    
    // Master parameters
    kMasterVolume = 1,
    kMasterTuning = 2,
    kMasterPan = 3,
    
    // Chip selection
    kChipSelect = 4,
    kChipCount = 5,
    
    // MIDI parameters
    kNoteTranspose = 6,
    kPitchBendRange = 7,
    kVelocitySensitivity = 8,
    
    // Voice allocation parameters
    kPolyphonyCount = 9,
    kLayerCount = 10,
    
    // Effect parameters
    kEffectChainBypass = 11,
    kEffect1Type = 12,
    kEffect1Bypass = 13,
    kEffect2Type = 14,
    kEffect2Bypass = 15,
    kEffect3Type = 16,
    kEffect3Bypass = 17,
    kEffect4Type = 18,
    kEffect4Bypass = 19,
    
    // Total parameter count (update this when adding new parameters)
    kParamCount = 20,
    
    // Special VST3 parameters
    kBypass = 1000,
    kProgram = 1001,
};

// Parameter tags for grouping in DAW UI
constexpr const char* kVoiceModeTag = "Voice Allocation";
constexpr const char* kMasterTag = "Master";
constexpr const char* kChipTag = "Chip";
constexpr const char* kMidTag = "MIDI";
constexpr const char* kEffectTag = "Effects";

} // namespace TigerFlame
