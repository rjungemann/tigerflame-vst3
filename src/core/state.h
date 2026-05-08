// TigerFlame - Core State Management
// Handles shared state between components and serialization

#pragma once

#include "stream.h"
#include <cstdint>

namespace TigerFlame {

class VoiceAllocator;
class ChipEngine;
class MidiTranslator;

// State serialization helpers
namespace StateUtil {

// Magic bytes for state chunks
constexpr uint32_t kVoiceAllocatorMagic = 0x564F4943; // "VOIC"
constexpr uint32_t kChipEngineMagic = 0x43484950; // "CHIP"
constexpr uint32_t kMidiTranslatorMagic = 0x4D494449; // "MIDI"

// Write a chunk header
void writeChunkHeader(IStream* stream, uint32_t magic, uint32_t version, uint32_t size);

// Read a chunk header
bool readChunkHeader(IStream* stream, uint32_t& magic, uint32_t& version, uint32_t& size);

// Skip a chunk
void skipChunk(IStream* stream, uint32_t size);

} // namespace StateUtil

// Shared state container
// Holds references to all core components for easy access
struct SharedState {
    VoiceAllocator* voiceAllocator = nullptr;
    ChipEngine* chipEngine = nullptr;
    MidiTranslator* midiTranslator = nullptr;
    
    // Sample rate
    double sampleRate = 44100.0;
    
    // Set components
    void setComponents(
        VoiceAllocator* va,
        ChipEngine* ce,
        MidiTranslator* mt,
        double sr = 44100.0) {
        voiceAllocator = va;
        chipEngine = ce;
        midiTranslator = mt;
        sampleRate = sr;
    }
};

} // namespace TigerFlame
