// TigerFlame - MIDI Translator Implementation

#include "midi_translator.h"
#include "chip_engine.h"
#include "state.h"

#include "base/source/fstreamer.h"

#include <algorithm>

namespace TigerFlame {

// Constants for CC mappings
constexpr int kMaxCC = 128;

// Constructor
MidiTranslator::MidiTranslator() {
    ccMappings_.resize(kMaxCC);
}

// Destructor
MidiTranslator::~MidiTranslator() = default;

// Set chip engine
void MidiTranslator::setChipEngine(ChipEngine* engine) {
    chipEngine_ = engine;
}

// Register chip
void MidiTranslator::registerChip(int chipIndex, ChipType type, int paramCount) {
    // Ensure we have enough space
    if (chipIndex >= static_cast<int>(chipParamCounts_.size())) {
        chipParamCounts_.resize(chipIndex + 1, 0);
    }
    chipParamCounts_[chipIndex] = paramCount;
}

// Translate note on
void MidiTranslator::translateNoteOn(int note, float velocity) {
    // Apply note transpose
    note += noteTranspose_;
    note = std::max(0, std::min(note, 127));
    
    // Apply velocity sensitivity
    velocity = std::min(velocity * velocitySensitivity_, 1.0f);
    
    ChipCommand cmd;
    cmd.type = ChipCommandType::kNoteOn;
    cmd.midiNote = note;
    cmd.velocity = velocity;
    
    // Queue command for dispatch
    commandQueue_.push_back(cmd);
}

// Translate note off
void MidiTranslator::translateNoteOff(int note) {
    // Apply note transpose
    note += noteTranspose_;
    note = std::max(0, std::min(note, 127));
    
    ChipCommand cmd;
    cmd.type = ChipCommandType::kNoteOff;
    cmd.midiNote = note;
    
    // Queue command for dispatch
    commandQueue_.push_back(cmd);
}

// Translate pitch bend
void MidiTranslator::translatePitchBend(double semitones) {
    ChipCommand cmd;
    cmd.type = ChipCommandType::kPitchBend;
    cmd.pitchBend = semitones;
    
    // Queue command for dispatch
    commandQueue_.push_back(cmd);
}

// Translate control change
void MidiTranslator::translateControlChange(int cc, float value) {
    // Check if CC is mapped
    if (cc >= 0 && cc < kMaxCC && ccMappings_[cc].chipIndex != -1) {
        const auto& mapping = ccMappings_[cc];
        
        // Scale value to mapped range
        float scaledValue = mapping.minValue + value * (mapping.maxValue - mapping.minValue);
        
        ChipCommand cmd;
        cmd.chipIndex = mapping.chipIndex;
        cmd.type = ChipCommandType::kSetParameter;
        cmd.paramId = mapping.paramId;
        cmd.paramValue = scaledValue;
        
        // Queue command for immediate dispatch (CC changes should be immediate)
        dispatchCommand(cmd);
    }
}

// Map CC to chip parameter
void MidiTranslator::mapCC(int cc, int chipIndex, int paramId, float minValue, float maxValue) {
    if (cc >= 0 && cc < kMaxCC) {
        auto& mapping = ccMappings_[cc];
        mapping.chipIndex = chipIndex;
        mapping.paramId = paramId;
        mapping.minValue = minValue;
        mapping.maxValue = maxValue;
    }
}

// Get mapped CC value
float MidiTranslator::getMappedCCValue(int cc) const {
    if (cc >= 0 && cc < kMaxCC) {
        const auto& mapping = ccMappings_[cc];
        if (mapping.chipIndex != -1 && chipEngine_) {
            // Ask chip engine for current parameter value
            return chipEngine_->getParameter(mapping.chipIndex, mapping.paramId);
        }
    }
    return 0.0f;
}

// Process queued commands
void MidiTranslator::processQueue() {
    for (const auto& cmd : commandQueue_) {
        dispatchCommand(cmd);
    }
    commandQueue_.clear();
}

// Clear pending commands
void MidiTranslator::clearQueue() {
    commandQueue_.clear();
}

// Dispatch command
void MidiTranslator::dispatchCommand(const ChipCommand& cmd) {
    if (onCommand_) {
        onCommand_(cmd);
    } else if (chipEngine_) {
        chipEngine_->sendCommand(cmd);
    }
}

// Save state
void MidiTranslator::saveState(IStream* stream) const {
    if (!stream) return;
    
    StateUtil::writeChunkHeader(stream, StateUtil::kMidiTranslatorMagic, 1, 0);
    
    // Save CC mappings
    uint32_t mappingCount = 0;
    for (const auto& mapping : ccMappings_) {
        if (mapping.chipIndex != -1) {
            mappingCount++;
        }
    }
    stream->write(&mappingCount, sizeof(mappingCount));
    
    for (const auto& mapping : ccMappings_) {
        if (mapping.chipIndex != -1) {
            stream->write(&mapping, sizeof(mapping));
        }
    }
    
    // Save note transpose and velocity sensitivity
    stream->write(&noteTranspose_, sizeof(noteTranspose_));
    stream->write(&velocitySensitivity_, sizeof(velocitySensitivity_));
}

// Restore state
void MidiTranslator::restoreState(IStream* stream) {
    if (!stream) return;
    
    uint32_t magic, version, size;
    if (!StateUtil::readChunkHeader(stream, magic, version, size)) {
        return;
    }
    
    if (magic != StateUtil::kMidiTranslatorMagic || version > 1) {
        return;
    }
    
    // Reset CC mappings
    for (auto& mapping : ccMappings_) {
        mapping = CCMapping();
    }
    
    // Read CC mappings
    uint32_t mappingCount;
    if (stream->read(&mappingCount, sizeof(mappingCount)) != StreamResult::kOk) return;
    
    for (uint32_t i = 0; i < mappingCount; ++i) {
        CCMapping mapping;
        if (stream->read(&mapping, sizeof(mapping)) != StreamResult::kOk) return;
        ccMappings_[mapping.paramId] = mapping;
    }
    
    // Read note transpose and velocity sensitivity
    if (stream->read(&noteTranspose_, sizeof(noteTranspose_)) != StreamResult::kOk) return;
    if (stream->read(&velocitySensitivity_, sizeof(velocitySensitivity_)) != StreamResult::kOk) return;
}

} // namespace TigerFlame
