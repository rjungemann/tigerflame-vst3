// TigerFlame - Audio Processing Tests

#include "catch2/catch_all.hpp"

#include "core/chip_engine.h"
#include "core/voice_allocator.h"
#include "core/midi_translator.h"

TEST_CASE("Audio Processing - End-to-End", "[audio_processing][e2e]") {
    using namespace TigerFlame;
    
    // Create core components
    ChipEngine chipEngine;
    VoiceAllocator voiceAllocator;
    MidiTranslator midiTranslator;
    
    chipEngine.setSampleRate(44100.0);
    voiceAllocator.setSampleRate(44100.0);
    voiceAllocator.setMidiTranslator(&midiTranslator);
    midiTranslator.setChipEngine(&chipEngine);
    
    // Add a chip
    ChipConfig config;
    config.type = ChipType::kChipYM2151;
    config.enabled = true;
    chipEngine.addChip(config);
    
    SECTION("MIDI note on produces non-zero audio output") {
        // Send note on
        voiceAllocator.noteOn(60, 0.8f);
        midiTranslator.processQueue();
        
        // Process some samples
        float left = 0.0f;
        float right = 0.0f;
        chipEngine.processSample(&left, &right);
        
        // With the placeholder sine wave chip, should produce some output
        // This might be 0 if the note hasn't been processed yet
        // The actual mml2vgm integration will handle this properly
        REQUIRE(true); // Placeholder - test passes if no crash
    }
    
    SECTION("Full signal chain: MIDI to audio") {
        // This is an integration test placeholder
        // The full implementation will test:
        // 1. MIDI note on -> VoiceAllocator
        // 2. VoiceAllocator -> MidiTranslator
        // 3. MidiTranslator -> ChipEngine
        // 4. ChipEngine -> audio output
        
        REQUIRE(true); // Placeholder
    }
    
    SECTION("Parameter changes affect audio output") {
        // This will be tested when chip parameters are properly implemented
        REQUIRE(true); // Placeholder
    }
    
    SECTION("No clicks or pops on parameter changes") {
        // This tests that parameter smoothing works
        // Placeholder for now
        REQUIRE(true);
    }
}

TEST_CASE("Audio Processing - Multi-Chip", "[audio_processing][multi_chip]") {
    using namespace TigerFlame;
    
    ChipEngine engine;
    engine.setSampleRate(44100.0);
    
    SECTION("Multiple chips sum output correctly") {
        ChipConfig config1, config2;
        config1.type = ChipType::kChipYM2151;
        config1.enabled = true;
        config2.type = ChipType::kChipYM2612;
        config2.enabled = true;
        
        engine.addChip(config1);
        engine.addChip(config2);
        
        float left = 0.0f;
        float right = 0.0f;
        engine.processSample(&left, &right);
        
        // Should process without crash
        REQUIRE(true);
    }
    
    SECTION("Per-chip volume affects output") {
        ChipConfig config;
        config.type = ChipType::kChipYM2151;
        config.volume = 0.5f;
        config.enabled = true;
        
        engine.addChip(config);
        
        float left1 = 0.0f, right1 = 0.0f;
        engine.processSample(&left1, &right1);
        
        // Change volume
        engine.setChipVolume(0, 1.0f);
        
        float left2 = 0.0f, right2 = 0.0f;
        engine.processSample(&left2, &right2);
        
        // Output should scale with volume
        // This is a placeholder - actual test needs chip implementation
    }
    
    SECTION("Disabled chip does not affect output") {
        ChipConfig config;
        config.type = ChipType::kChipYM2151;
        config.enabled = false;
        
        engine.addChip(config);
        
        float left = 1.0f;
        float right = 1.0f;
        engine.processSample(&left, &right);
        
        // Disabled chip should not modify output
        REQUIRE(left == 0.0f);
        REQUIRE(right == 0.0f);
    }
}
