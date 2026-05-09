// TigerFlame - Voice Allocator Implementation

#include "voice_allocator.h"
#include "midi_translator.h"
#include "state.h"

#include <algorithm>

namespace TigerFlame {

// Constants for CC numbers
constexpr int kCCModulation = 1;
constexpr int kCCVolume = 7;
constexpr int kCCPan = 10;
constexpr int kCCExpression = 11;
constexpr int kCCSustain = 64;
constexpr int kCCAllNotesOff = 123;
constexpr int kCCAllSoundOff = 120;
constexpr int kCCResetAllControllers = 121;

// Constructor
VoiceAllocator::VoiceAllocator() : mode_(Mode::kMono) {
    // Initialize polyphonic slots
    setPolyphonyCount(8);
    reset();
}

// Destructor
VoiceAllocator::~VoiceAllocator() = default;

// Set mode
void VoiceAllocator::setMode(Mode mode) {
    if (mode == mode_) return;
    
    Mode oldMode = mode_;
    
    // When switching mono → poly: promote the active mono note into the first poly slot
    if (oldMode == Mode::kMono && mode == Mode::kPolyphonic) {
        // Save mono state before setPolyphonyCount clears it via reset()
        MonoVoiceState savedMono = monoState_;
        // Change mode first so poly state is used going forward
        mode_ = mode;
        setPolyphonyCount(polyphonyCount_);
        // Promote active mono note if there is one
        if (savedMono.activeNote != -1) {
            auto& slot = polySlots_[0];
            slot.midiNote = savedMono.activeNote;
            slot.velocity = savedMono.velocity;
            slot.pitchBend = savedMono.pitchBend;
            slot.noteOnTime = savedMono.noteOnTime;
            slot.sustain = savedMono.sustain;
            slot.released = savedMono.released;
        }
    } else {
        // All other transitions: silence everything first
        allSoundOff();
        mode_ = mode;
        switch (mode_) {
            case Mode::kMono:
                reset();
                break;
            case Mode::kLayeredMultitimbral:
                layerStates_.resize(layers_.size());
                break;
            case Mode::kPolyphonic:
                setPolyphonyCount(polyphonyCount_);
                break;
        }
    }
    
    updateActiveVoices();
}

// Set polyphony count
void VoiceAllocator::setPolyphonyCount(int voicesPerChip) {
    polyphonyCount_ = std::max(2, std::min(voicesPerChip, 32));
    polySlots_.resize(polyphonyCount_);
    updateActiveVoices();
}

// Add layer
void VoiceAllocator::addLayer(const LayerConfig& config) {
    layers_.push_back(config);
    layerStates_.resize(layers_.size());
}

// Remove layer
void VoiceAllocator::removeLayer(int layerIndex) {
    if (layerIndex >= 0 && layerIndex < static_cast<int>(layers_.size())) {
        layers_.erase(layers_.begin() + layerIndex);
        layerStates_.resize(layers_.size());
    }
}

// Update layer
void VoiceAllocator::updateLayer(int layerIndex, const LayerConfig& config) {
    if (layerIndex >= 0 && layerIndex < static_cast<int>(layers_.size())) {
        layers_[layerIndex] = config;
    }
}

// Get layer
const LayerConfig& VoiceAllocator::getLayer(int index) const {
    static const LayerConfig kDefaultLayer;
    if (index >= 0 && index < static_cast<int>(layers_.size())) {
        return layers_[index];
    }
    return kDefaultLayer;
}

// Note on
void VoiceAllocator::noteOn(int note, float velocity) {
    switch (mode_) {
        case Mode::kMono:
            noteOnMono(note, velocity);
            break;
        case Mode::kLayeredMultitimbral:
            noteOnLayered(note, velocity);
            break;
        case Mode::kPolyphonic:
            noteOnPoly(note, velocity);
            break;
    }
    updateActiveVoices();
}

// Note off
void VoiceAllocator::noteOff(int note) {
    switch (mode_) {
        case Mode::kMono:
            noteOffMono(note);
            break;
        case Mode::kLayeredMultitimbral:
            noteOffLayered(note);
            break;
        case Mode::kPolyphonic:
            noteOffPoly(note);
            break;
    }
    updateActiveVoices();
}

// Pitch bend
void VoiceAllocator::pitchBend(double semitones) {
    switch (mode_) {
        case Mode::kMono:
            monoState_.pitchBend = semitones;
            break;
        case Mode::kLayeredMultitimbral:
            // Apply to all active layers
            for (auto& state : layerStates_) {
                state.pitchBend = semitones;
            }
            break;
        case Mode::kPolyphonic:
            // Apply to all active poly slots
            for (auto& slot : polySlots_) {
                if (slot.midiNote != -1) {
                    slot.pitchBend = semitones;
                }
            }
            break;
    }
}

// Control change
void VoiceAllocator::controlChange(int cc, float value) {
    switch (cc) {
        case kCCSustain:
            // Sustain pedal
            switch (mode_) {
                case Mode::kMono: {
                    bool sustainOn = (value > 0.5f);
                    monoState_.sustain = sustainOn;
                    if (!sustainOn && monoState_.released) {
                        // Release the note that was held by sustain
                        monoState_.activeNote = -1;
                        monoState_.released = false;
                    }
                    break;
                }
                case Mode::kLayeredMultitimbral:
                    for (auto& state : layerStates_) {
                        bool sustainOn = (value > 0.5f);
                        state.sustain = sustainOn;
                        if (!sustainOn && state.released) {
                            state.activeNote = -1;
                            state.released = false;
                        }
                    }
                    break;
                case Mode::kPolyphonic:
                    for (auto& slot : polySlots_) {
                        if (slot.midiNote != -1) {
                            bool sustainOn = (value > 0.5f);
                            slot.sustain = sustainOn;
                            if (!sustainOn && slot.released) {
                                slot.midiNote = -1;
                                slot.released = false;
                            }
                        }
                    }
                    break;
            }
            break;
            
        case kCCAllNotesOff:
            allNotesOff();
            break;
            
        case kCCAllSoundOff:
            allSoundOff();
            break;
            
        case kCCResetAllControllers:
            // Reset all CC state
            break;
            
        default:
            // Forward other CCs to MIDI translator
            if (midiTranslator_) {
                midiTranslator_->translateControlChange(cc, value);
            }
            break;
    }
    updateActiveVoices();
}

// All notes off
void VoiceAllocator::allNotesOff() {
    switch (mode_) {
        case Mode::kMono:
            if (monoState_.activeNote != -1 && !monoState_.sustain) {
                monoState_.activeNote = -1;
            }
            break;
        case Mode::kLayeredMultitimbral:
            for (auto& state : layerStates_) {
                if (state.activeNote != -1 && !state.sustain) {
                    state.activeNote = -1;
                }
            }
            break;
        case Mode::kPolyphonic:
            for (auto& slot : polySlots_) {
                if (slot.midiNote != -1 && !slot.sustain) {
                    slot.midiNote = -1;
                }
            }
            break;
    }
    updateActiveVoices();
}

// All sound off
void VoiceAllocator::allSoundOff() {
    switch (mode_) {
        case Mode::kMono:
            monoState_.activeNote = -1;
            monoState_.sustain = false;
            break;
        case Mode::kLayeredMultitimbral:
            for (auto& state : layerStates_) {
                state.activeNote = -1;
                state.sustain = false;
            }
            break;
        case Mode::kPolyphonic:
            for (auto& slot : polySlots_) {
                slot.midiNote = -1;
                slot.sustain = false;
            }
            break;
    }
    updateActiveVoices();
}

// Is note active
bool VoiceAllocator::isNoteActive(int note) const {
    for (const auto& alloc : activeVoices_) {
        if (alloc.midiNote == note) {
            return true;
        }
    }
    return false;
}

// Get voice allocation for a specific note
const VoiceAllocation* VoiceAllocator::getVoiceAllocation(int note) const {
    for (const auto& alloc : activeVoices_) {
        if (alloc.midiNote == note) {
            return &alloc;
        }
    }
    return nullptr;
}

// Save state
void VoiceAllocator::saveState(IStream* stream) const {
    if (!stream) return;
    
    StateUtil::writeChunkHeader(stream, StateUtil::kVoiceAllocatorMagic, 1, 0);
    
    // Save mode
    uint32_t mode = static_cast<uint32_t>(mode_);
    stream->write(&mode, sizeof(mode));
    
    // Save polyphony count
    stream->write(&polyphonyCount_, sizeof(polyphonyCount_));
    
    // Save mono state
    stream->write(&monoState_, sizeof(monoState_));
    
    // Save poly slots
    uint32_t polyCount = static_cast<uint32_t>(polySlots_.size());
    stream->write(&polyCount, sizeof(polyCount));
    for (const auto& slot : polySlots_) {
        stream->write(&slot, sizeof(slot));
    }
    
    // Save layers
    uint32_t layerCount = static_cast<uint32_t>(layers_.size());
    stream->write(&layerCount, sizeof(layerCount));
    for (const auto& layer : layers_) {
        stream->write(&layer, sizeof(layer));
    }
    
    // Save layer states
    uint32_t layerStateCount = static_cast<uint32_t>(layerStates_.size());
    stream->write(&layerStateCount, sizeof(layerStateCount));
    for (const auto& state : layerStates_) {
        stream->write(&state, sizeof(state));
    }
}

// Restore state
void VoiceAllocator::restoreState(IStream* stream) {
    if (!stream) return;
    
    uint32_t magic, version, size;
    if (!StateUtil::readChunkHeader(stream, magic, version, size)) {
        return;
    }
    
    if (magic != StateUtil::kVoiceAllocatorMagic || version > 1) {
        return;
    }
    
    // Read mode
    uint32_t mode;
    if (stream->read(&mode, sizeof(mode)) != StreamResult::kOk) return;
    mode_ = static_cast<Mode>(mode);
    
    // Read polyphony count
    if (stream->read(&polyphonyCount_, sizeof(polyphonyCount_)) != StreamResult::kOk) return;
    
    // Read mono state
    if (stream->read(&monoState_, sizeof(monoState_)) != StreamResult::kOk) return;
    
    // Read poly slots
    uint32_t polyCount;
    if (stream->read(&polyCount, sizeof(polyCount)) != StreamResult::kOk) return;
    polySlots_.resize(polyCount);
    for (auto& slot : polySlots_) {
        if (stream->read(&slot, sizeof(slot)) != StreamResult::kOk) return;
    }
    
    // Read layers
    uint32_t layerCount;
    if (stream->read(&layerCount, sizeof(layerCount)) != StreamResult::kOk) return;
    layers_.resize(layerCount);
    for (auto& layer : layers_) {
        if (stream->read(&layer, sizeof(layer)) != StreamResult::kOk) return;
    }
    
    // Read layer states
    uint32_t layerStateCount;
    if (stream->read(&layerStateCount, sizeof(layerStateCount)) != StreamResult::kOk) return;
    layerStates_.resize(layerStateCount);
    for (auto& state : layerStates_) {
        if (stream->read(&state, sizeof(state)) != StreamResult::kOk) return;
    }
    
    updateActiveVoices();
}

// Reset
void VoiceAllocator::reset() {
    monoState_ = MonoVoiceState();
    for (auto& slot : polySlots_) {
        slot = PolyVoiceSlot();
    }
    for (auto& state : layerStates_) {
        state = MonoVoiceState();
    }
    activeVoices_.clear();
    sampleCounter_ = 0;
}

// Find free polyphonic voice slot
int VoiceAllocator::findFreePolySlot() {
    for (int i = 0; i < polyphonyCount_; ++i) {
        if (polySlots_[i].midiNote == -1) {
            return i;
        }
    }
    return -1; // No free slot
}

// Find oldest polyphonic voice slot
int VoiceAllocator::findOldestPolySlot() {
    int oldestIndex = 0;
    int64_t oldestTime = polySlots_[0].noteOnTime;
    
    for (int i = 1; i < polyphonyCount_; ++i) {
        if (polySlots_[i].noteOnTime < oldestTime) {
            oldestIndex = i;
            oldestTime = polySlots_[i].noteOnTime;
        }
    }
    
    return oldestIndex;
}

// Note on (monophonic mode)
void VoiceAllocator::noteOnMono(int note, float velocity) {
    monoState_.activeNote = note;
    monoState_.velocity = velocity;
    monoState_.noteOnTime = sampleCounter_;
    
    // If sustain was on, the note was being held, so send note off first
    if (monoState_.sustain) {
        // Note was being sustained, now new note comes in
        // This is a retrigger
    }
}

// Note off (monophonic mode)
void VoiceAllocator::noteOffMono(int note) {
    if (monoState_.activeNote == note) {
        if (!monoState_.sustain) {
            monoState_.activeNote = -1;
        } else {
            // Key released while sustaining — mark for release when sustain lifts
            monoState_.released = true;
        }
    }
}

// Note on (layered multitimbral mode)
void VoiceAllocator::noteOnLayered(int note, float velocity) {
    for (auto& state : layerStates_) {
        state.activeNote = note;
        state.velocity = velocity;
        state.noteOnTime = sampleCounter_;
    }
}

// Note off (layered multitimbral mode)
void VoiceAllocator::noteOffLayered(int note) {
    for (auto& state : layerStates_) {
        if (state.activeNote == note) {
            if (!state.sustain) {
                state.activeNote = -1;
            }
        }
    }
}

// Note on (polyphonic mode)
void VoiceAllocator::noteOnPoly(int note, float velocity) {
    int freeSlot = findFreePolySlot();
    
    if (freeSlot >= 0) {
        // Found free slot
        auto& slot = polySlots_[freeSlot];
        slot.midiNote = note;
        slot.velocity = velocity;
        slot.pitchBend = monoState_.pitchBend; // Use current pitch bend
        slot.noteOnTime = sampleCounter_;
        slot.sustain = false;
    } else {
        // No free slot, use LRU voice stealing
        int oldestSlot = findOldestPolySlot();
        auto& slot = polySlots_[oldestSlot];
        
        // Send note off for the stolen note
        if (slot.midiNote != -1 && midiTranslator_) {
            midiTranslator_->translateNoteOff(slot.midiNote);
        }
        
        // Assign new note
        slot.midiNote = note;
        slot.velocity = velocity;
        slot.pitchBend = monoState_.pitchBend;
        slot.noteOnTime = sampleCounter_;
        slot.sustain = false;
    }
}

// Note off (polyphonic mode)
void VoiceAllocator::noteOffPoly(int note) {
    for (auto& slot : polySlots_) {
        if (slot.midiNote == note) {
            if (!slot.sustain) {
                slot.midiNote = -1;
            } else {
                // Key released while sustaining — mark for release when sustain lifts
                slot.released = true;
            }
        }
    }
}

// Update active voices list
void VoiceAllocator::updateActiveVoices() {
    activeVoices_.clear();
    
    switch (mode_) {
        case Mode::kMono:
            if (monoState_.activeNote != -1) {
                VoiceAllocation alloc;
                alloc.midiNote = monoState_.activeNote;
                alloc.velocity = monoState_.velocity;
                alloc.pitchBend = monoState_.pitchBend;
                alloc.chipIndex = monoState_.activeChipIndex;
                alloc.voiceIndex = monoState_.activeVoiceIndex;
                alloc.noteOnTime = monoState_.noteOnTime;
                alloc.sustain = monoState_.sustain;
                activeVoices_.push_back(alloc);
            }
            break;
            
        case Mode::kLayeredMultitimbral:
            for (size_t i = 0; i < layerStates_.size(); ++i) {
                const auto& state = layerStates_[i];
                if (state.activeNote != -1) {
                    VoiceAllocation alloc;
                    alloc.midiNote = state.activeNote;
                    alloc.velocity = state.velocity;
                    alloc.pitchBend = state.pitchBend;
                    alloc.chipIndex = layers_[i].chipIndex;
                    alloc.voiceIndex = 0;
                    alloc.noteOnTime = state.noteOnTime;
                    alloc.sustain = state.sustain;
                    activeVoices_.push_back(alloc);
                }
            }
            break;
            
        case Mode::kPolyphonic:
            for (const auto& slot : polySlots_) {
                if (slot.midiNote != -1) {
                    VoiceAllocation alloc;
                    alloc.midiNote = slot.midiNote;
                    alloc.velocity = slot.velocity;
                    alloc.pitchBend = slot.pitchBend;
                    alloc.chipIndex = 0; // Will be set by chip engine
                    alloc.voiceIndex = &slot - &polySlots_[0];
                    alloc.noteOnTime = slot.noteOnTime;
                    alloc.sustain = slot.sustain;
                    activeVoices_.push_back(alloc);
                }
            }
            break;
    }
}

} // namespace TigerFlame
