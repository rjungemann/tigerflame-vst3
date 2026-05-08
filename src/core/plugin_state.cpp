// TigerFlame - Plugin State Implementation

#include "plugin_state.h"
#include <algorithm>

namespace TigerFlame {

// Magic bytes for state file identification
static const uint32_t kStateMagic = 0x54474630; // "TGF0"
static const uint32_t kStateVersion = 1;

// Save state to stream
void PluginState::saveToStream(IStream* stream) const {
    if (!stream) {
        return;
    }
    
    // Write magic bytes and version
    stream->write(&kStateMagic, sizeof(kStateMagic));
    stream->write(&kStateVersion, sizeof(kStateVersion));
    
    // Write all parameter values
    stream->write(&voiceMode, sizeof(voiceMode));
    stream->write(&masterVolume, sizeof(masterVolume));
    stream->write(&masterTuning, sizeof(masterTuning));
    stream->write(&masterPan, sizeof(masterPan));
    stream->write(&chipSelect, sizeof(chipSelect));
    stream->write(&chipCount, sizeof(chipCount));
    stream->write(&noteTranspose, sizeof(noteTranspose));
    stream->write(&pitchBendRange, sizeof(pitchBendRange));
    stream->write(&velocitySensitivity, sizeof(velocitySensitivity));
    stream->write(&polyphonyCount, sizeof(polyphonyCount));
    stream->write(&layerCount, sizeof(layerCount));
    stream->write(&effectChainBypass, sizeof(effectChainBypass));
    stream->write(&effect1Type, sizeof(effect1Type));
    stream->write(&effect1Bypass, sizeof(effect1Bypass));
    stream->write(&effect2Type, sizeof(effect2Type));
    stream->write(&effect2Bypass, sizeof(effect2Bypass));
    stream->write(&effect3Type, sizeof(effect3Type));
    stream->write(&effect3Bypass, sizeof(effect3Bypass));
    stream->write(&effect4Type, sizeof(effect4Type));
    stream->write(&effect4Bypass, sizeof(effect4Bypass));
}

// Restore state from stream
void PluginState::restoreFromStream(IStream* stream) {
    if (!stream) {
        return;
    }
    
    // Read magic bytes and version
    uint32_t magic, version;
    if (stream->read(&magic, sizeof(magic)) != StreamResult::kOk || magic != kStateMagic) {
        // Invalid magic, reset to defaults
        reset();
        return;
    }
    
    if (stream->read(&version, sizeof(version)) != StreamResult::kOk || version > kStateVersion) {
        // Unsupported version, reset to defaults
        reset();
        return;
    }
    
    // Read all parameter values
    float value;
    
    #define READ_PARAM(dest) \
        if (stream->read(&value, sizeof(value)) == StreamResult::kOk) { \
            dest = value; \
        }
    
    READ_PARAM(voiceMode);
    READ_PARAM(masterVolume);
    READ_PARAM(masterTuning);
    READ_PARAM(masterPan);
    READ_PARAM(chipSelect);
    READ_PARAM(chipCount);
    READ_PARAM(noteTranspose);
    READ_PARAM(pitchBendRange);
    READ_PARAM(velocitySensitivity);
    READ_PARAM(polyphonyCount);
    READ_PARAM(layerCount);
    READ_PARAM(effectChainBypass);
    READ_PARAM(effect1Type);
    READ_PARAM(effect1Bypass);
    READ_PARAM(effect2Type);
    READ_PARAM(effect2Bypass);
    READ_PARAM(effect3Type);
    READ_PARAM(effect3Bypass);
    READ_PARAM(effect4Type);
    READ_PARAM(effect4Bypass);
    
    #undef READ_PARAM
}

} // namespace TigerFlame
