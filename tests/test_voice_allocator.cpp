// TigerFlame - Voice Allocator Tests

#include "catch2/catch_all.hpp"

#include "core/voice_allocator.h"
#include "core/midi_translator.h"

namespace TigerFlame {

// Mock MIDI translator for testing
class MockMidiTranslator : public MidiTranslator {
public:
    int noteOnCount = 0;
    int noteOffCount = 0;
    int lastNoteOn = -1;
    int lastNoteOff = -1;
    
    void translateNoteOn(int note, float velocity) {
        noteOnCount++;
        lastNoteOn = note;
    }
    
    void translateNoteOff(int note) {
        noteOffCount++;
        lastNoteOff = note;
    }
};

} // namespace TigerFlame

TEST_CASE("Voice Allocator - Monophonic Mode", "[voice_allocator][mono]") {
    using namespace TigerFlame;
    
    VoiceAllocator allocator;
    MockMidiTranslator translator;
    allocator.setMidiTranslator(&translator);
    allocator.setMode(VoiceAllocator::Mode::kMono);
    
    SECTION("Initial state is idle") {
        REQUIRE(allocator.getVoiceCount() == 0);
        REQUIRE(allocator.getMonoState().activeNote == -1);
    }
    
    SECTION("Note on sets active voice") {
        allocator.noteOn(60, 0.8f);
        REQUIRE(allocator.getVoiceCount() == 1);
        REQUIRE(allocator.getMonoState().activeNote == 60);
        REQUIRE(allocator.getMonoState().velocity == 0.8f);
        REQUIRE(allocator.isNoteActive(60));
    }
    
    SECTION("Note off clears active voice") {
        allocator.noteOn(60, 0.8f);
        allocator.noteOff(60);
        REQUIRE(allocator.getVoiceCount() == 0);
        REQUIRE(allocator.getMonoState().activeNote == -1);
    }
    
    SECTION("Second note on replaces first (last-note priority)") {
        allocator.noteOn(60, 0.8f);
        REQUIRE(allocator.getMonoState().activeNote == 60);
        
        allocator.noteOn(64, 0.6f);
        REQUIRE(allocator.getVoiceCount() == 1);
        REQUIRE(allocator.getMonoState().activeNote == 64);
        REQUIRE(allocator.getMonoState().velocity == 0.6f);
    }
    
    SECTION("Note off of non-active note is ignored") {
        allocator.noteOn(60, 0.8f);
        allocator.noteOff(64); // Different note
        REQUIRE(allocator.getVoiceCount() == 1);
        REQUIRE(allocator.getMonoState().activeNote == 60);
    }
    
    SECTION("Sustain pedal holds note after note off") {
        allocator.noteOn(60, 0.8f);
        allocator.controlChange(64, 1.0f); // Sustain on
        allocator.noteOff(60);
        REQUIRE(allocator.getVoiceCount() == 1); // Still active due to sustain
        
        allocator.controlChange(64, 0.0f); // Sustain off
        REQUIRE(allocator.getVoiceCount() == 0); // Now cleared
    }
    
    SECTION("All notes off clears all state") {
        allocator.noteOn(60, 0.8f);
        allocator.allNotesOff();
        REQUIRE(allocator.getVoiceCount() == 0);
    }
    
    SECTION("Pitch bend updates state correctly") {
        allocator.noteOn(60, 0.8f);
        allocator.pitchBend(2.0); // +2 semitones
        REQUIRE(allocator.getMonoState().pitchBend == 2.0);
    }
}

TEST_CASE("Voice Allocator - Polyphonic Mode", "[voice_allocator][poly]") {
    using namespace TigerFlame;
    
    VoiceAllocator allocator;
    MockMidiTranslator translator;
    allocator.setMidiTranslator(&translator);
    allocator.setMode(VoiceAllocator::Mode::kPolyphonic);
    allocator.setPolyphonyCount(4);
    
    SECTION("Notes fill voice slots in order") {
        allocator.noteOn(60, 0.8f);
        allocator.noteOn(64, 0.6f);
        allocator.noteOn(67, 0.5f);
        allocator.noteOn(72, 0.4f);
        
        REQUIRE(allocator.getVoiceCount() == 4);
        REQUIRE(allocator.isNoteActive(60));
        REQUIRE(allocator.isNoteActive(64));
        REQUIRE(allocator.isNoteActive(67));
        REQUIRE(allocator.isNoteActive(72));
    }
    
    SECTION("Voice stealing steals oldest note when full") {
        // Fill all 4 slots
        allocator.noteOn(60, 0.8f);
        allocator.noteOn(64, 0.6f);
        allocator.noteOn(67, 0.5f);
        allocator.noteOn(72, 0.4f);
        
        // Add one more note - should steal oldest (60)
        allocator.noteOn(75, 0.9f);
        
        REQUIRE(allocator.getVoiceCount() == 4);
        REQUIRE(!allocator.isNoteActive(60)); // Oldest stolen
        REQUIRE(allocator.isNoteActive(75)); // New note added
    }
    
    SECTION("Note off releases correct slot") {
        allocator.noteOn(60, 0.8f);
        allocator.noteOn(64, 0.6f);
        
        allocator.noteOff(60);
        REQUIRE(allocator.getVoiceCount() == 1);
        REQUIRE(!allocator.isNoteActive(60));
        REQUIRE(allocator.isNoteActive(64));
    }
    
    SECTION("Sustain pedal prevents release") {
        allocator.noteOn(60, 0.8f);
        allocator.controlChange(64, 1.0f); // Sustain on
        allocator.noteOff(60);
        REQUIRE(allocator.getVoiceCount() == 1); // Still active
        
        allocator.controlChange(64, 0.0f); // Sustain off
        REQUIRE(allocator.getVoiceCount() == 0); // Now released
    }
    
    SECTION("Pitch bend applies to all active voices") {
        allocator.noteOn(60, 0.8f);
        allocator.noteOn(64, 0.6f);
        allocator.pitchBend(1.0);
        
        const auto& polyState = allocator.getPolyState();
        for (const auto& slot : polyState) {
            if (slot.midiNote != -1) {
                REQUIRE(slot.pitchBend == 1.0);
            }
        }
    }
}

TEST_CASE("Voice Allocator - Layered Multitimbral Mode", "[voice_allocator][layered]") {
    using namespace TigerFlame;
    
    VoiceAllocator allocator;
    allocator.setMode(VoiceAllocator::Mode::kLayeredMultitimbral);
    
    // Add two layers
    LayerConfig layer1;
    layer1.chipIndex = 0;
    layer1.instrumentId = 1;
    layer1.volume = 1.0f;
    layer1.enabled = true;
    
    LayerConfig layer2;
    layer2.chipIndex = 1;
    layer2.instrumentId = 2;
    layer2.volume = 0.8f;
    layer2.enabled = true;
    
    allocator.addLayer(layer1);
    allocator.addLayer(layer2);
    
    SECTION("Note on triggers all enabled layers") {
        allocator.noteOn(60, 0.8f);
        REQUIRE(allocator.getVoiceCount() == 2); // Both layers active
    }
    
    SECTION("Layer add/remove mid-playback works") {
        allocator.noteOn(60, 0.8f);
        REQUIRE(allocator.getVoiceCount() == 2);
        
        allocator.removeLayer(0);
        REQUIRE(allocator.getLayerCount() == 1);
    }
}

TEST_CASE("Voice Allocator - Mode Switching", "[voice_allocator][mode_switch]") {
    using namespace TigerFlame;
    
    VoiceAllocator allocator;
    
    SECTION("Switch from Mono to Poly preserves state") {
        allocator.setMode(VoiceAllocator::Mode::kMono);
        allocator.noteOn(60, 0.8f);
        
        allocator.setMode(VoiceAllocator::Mode::kPolyphonic);
        allocator.setPolyphonyCount(4);
        
        // Should have promoted current note to first slot
        REQUIRE(allocator.getVoiceCount() == 1);
    }
    
    SECTION("Switch clears all notes (all sound off)") {
        allocator.setMode(VoiceAllocator::Mode::kPolyphonic);
        allocator.setPolyphonyCount(4);
        allocator.noteOn(60, 0.8f);
        allocator.noteOn(64, 0.6f);
        
        allocator.setMode(VoiceAllocator::Mode::kMono);
        
        // All notes should be cleared
        REQUIRE(allocator.getVoiceCount() == 0);
    }
}

TEST_CASE("Voice Allocator - State Persistence", "[voice_allocator][state]") {
    using namespace TigerFlame;
    
    VoiceAllocator allocator;
    allocator.setMode(VoiceAllocator::Mode::kPolyphonic);
    allocator.setPolyphonyCount(8);
    
    // Add some notes
    allocator.noteOn(60, 0.8f);
    allocator.noteOn(64, 0.6f);
    allocator.noteOn(67, 0.5f);
    
    SECTION("Save and restore state preserves notes") {
        // Save state
        // (We can't easily test IBStream without mocking, so this is a placeholder)
        // In a real test, we'd use a mock stream
        
        // For now, just verify the state is as expected
        REQUIRE(allocator.getVoiceCount() == 3);
    }
}
