// TigerFlame - Parameter Model Implementation

#include "parameter_model.h"

#include <algorithm>
#include <vector>

namespace TigerFlame {

// Parameter descriptors
static const std::vector<ParameterDescriptor> kParameterDescriptors = {
    // Voice Mode
    {
        ParamId::kVoiceMode,
        "Voice Mode",
        "Mode",
        "",
        0.0,    // default: Mono
        0.0,    // min
        2.0,    // max
        0.0,    // default normalized
        true,   // discrete
        3       // 3 modes: Mono, Layered, Polyphonic
    },
    
    // Master Volume (0.0 - 1.0 linear)
    {
        ParamId::kMasterVolume,
        "Master Volume",
        "Volume",
        "",
        0.8,    // default: 80%
        0.0,    // min: 0%
        1.0,    // max: 100%
        0.8,    // default normalized
        false,  // continuous
        0
    },
    
    // Master Tuning (-100 to +100 cents)
    {
        ParamId::kMasterTuning,
        "Master Tuning",
        "Tuning",
        "cents",
        0.0,    // default: 0 cents
        -100.0, // min: -100 cents
        100.0,  // max: +100 cents
        0.5,    // default normalized (centered)
        false,  // continuous
        0
    },
    
    // Master Pan (-100% to +100%)
    {
        ParamId::kMasterPan,
        "Master Pan",
        "Pan",
        "%",
        0.0,    // default: center
        -100.0, // min: left 100%
        100.0,  // max: right 100%
        0.5,    // default normalized (centered)
        false,  // continuous
        0
    },
    
    // Chip Select (0-7 for 8 chip types)
    {
        ParamId::kChipSelect,
        "Chip Type",
        "Chip",
        "",
        0.0,    // default: YM2151
        0.0,    // min
        7.0,    // max (8 chip types - 1)
        0.0,    // default normalized
        true,   // discrete
        8       // 8 chip types
    },
    
    // Chip Count (1-8 chips)
    {
        ParamId::kChipCount,
        "Chip Count",
        "Chips",
        "",
        1.0,    // default: 1 chip
        1.0,    // min
        8.0,    // max
        0.0,    // default normalized (will be 0/7 = 0)
        true,   // discrete
        8
    },
    
    // Note Transpose (-48 to +48 semitones)
    {
        ParamId::kNoteTranspose,
        "Transpose",
        "Transpose",
        "semitones",
        0.0,    // default: 0
        -48.0,  // min: -4 octaves
        48.0,   // max: +4 octaves
        0.5,    // default normalized (centered)
        false,  // continuous
        0
    },
    
    // Pitch Bend Range (0-24 semitones)
    {
        ParamId::kPitchBendRange,
        "Pitch Bend Range",
        "Bend Range",
        "semitones",
        2.0,    // default: 2 semitones
        0.0,    // min
        24.0,   // max
        2.0 / 24.0, // default normalized
        false,  // continuous
        0
    },
    
    // Velocity Sensitivity (0.0-2.0)
    {
        ParamId::kVelocitySensitivity,
        "Velocity Sensitivity",
        "Velocity",
        "",
        1.0,    // default: 100%
        0.0,    // min: 0%
        2.0,    // max: 200%
        1.0,    // default normalized
        false,  // continuous
        0
    },
    
    // Polyphony Count (2-32 voices)
    {
        ParamId::kPolyphonyCount,
        "Polyphony",
        "Poly",
        "voices",
        8.0,    // default: 8 voices
        2.0,    // min
        32.0,   // max
        (8.0 - 2.0) / 30.0, // default normalized
        true,   // discrete
        31      // 31 steps (2-32)
    },
    
    // Layer Count (1-8 layers)
    {
        ParamId::kLayerCount,
        "Layers",
        "Layers",
        "",
        1.0,    // default: 1 layer
        1.0,    // min
        8.0,    // max
        0.0,    // default normalized
        true,   // discrete
        8
    },
    
    // Effect Chain Bypass
    {
        ParamId::kEffectChainBypass,
        "Effects Bypass",
        "FX Bypass",
        "",
        0.0,    // default: effects enabled
        0.0,    // min
        1.0,    // max
        0.0,    // default normalized
        true,   // discrete (bypass is on/off)
        2
    },
    
    // Effect 1 Type
    {
        ParamId::kEffect1Type,
        "Effect 1 Type",
        "FX1 Type",
        "",
        0.0,    // default: None
        0.0,    // min
        5.0,    // max (6 effect types - 1)
        0.0,    // default normalized
        true,   // discrete
        6       // 6 effect types
    },
    
    // Effect 1 Bypass
    {
        ParamId::kEffect1Bypass,
        "Effect 1 Bypass",
        "FX1 Bypass",
        "",
        0.0,    // default: enabled
        0.0,    // min
        1.0,    // max
        0.0,    // default normalized
        true,   // discrete
        2
    },
    
    // Effect 2 Type
    {
        ParamId::kEffect2Type,
        "Effect 2 Type",
        "FX2 Type",
        "",
        0.0,
        0.0,
        5.0,
        0.0,
        true,
        6
    },
    
    // Effect 2 Bypass
    {
        ParamId::kEffect2Bypass,
        "Effect 2 Bypass",
        "FX2 Bypass",
        "",
        0.0,
        0.0,
        1.0,
        0.0,
        true,
        2
    },
    
    // Effect 3 Type
    {
        ParamId::kEffect3Type,
        "Effect 3 Type",
        "FX3 Type",
        "",
        0.0,
        0.0,
        5.0,
        0.0,
        true,
        6
    },
    
    // Effect 3 Bypass
    {
        ParamId::kEffect3Bypass,
        "Effect 3 Bypass",
        "FX3 Bypass",
        "",
        0.0,
        0.0,
        1.0,
        0.0,
        true,
        2
    },
    
    // Effect 4 Type
    {
        ParamId::kEffect4Type,
        "Effect 4 Type",
        "FX4 Type",
        "",
        0.0,
        0.0,
        5.0,
        0.0,
        true,
        6
    },
    
    // Effect 4 Bypass
    {
        ParamId::kEffect4Bypass,
        "Effect 4 Bypass",
        "FX4 Bypass",
        "",
        0.0,
        0.0,
        1.0,
        0.0,
        true,
        2
    },
};

// Get parameter descriptor by ID
const ParameterDescriptor* getParameterDescriptor(ParamId id) {
    for (const auto& desc : kParameterDescriptors) {
        if (desc.id == id) {
            return &desc;
        }
    }
    return nullptr;
}

// Get all parameter descriptors
const std::vector<ParameterDescriptor>& getAllParameterDescriptors() {
    return kParameterDescriptors;
}

} // namespace TigerFlame
