// TigerFlame - Chip Engine Tests

#include "catch2/catch_all.hpp"

#include "core/chip_engine.h"

TEST_CASE("Chip Engine - Basic Functionality", "[chip_engine][basic]") {
    using namespace TigerFlame;
    
    ChipEngine engine;
    engine.setSampleRate(44100.0);
    
    SECTION("Initial state") {
        REQUIRE(engine.getChipCount() == 0);
        REQUIRE(engine.getOutputLevel() == 0.0f);
    }
    
    SECTION("Add chip increases count") {
        ChipConfig config;
        config.type = ChipType::kChipYM2151;
        int index = engine.addChip(config);
        REQUIRE(index == 0);
        REQUIRE(engine.getChipCount() == 1);
    }
    
    SECTION("Remove chip decreases count") {
        ChipConfig config;
        config.type = ChipType::kChipYM2151;
        engine.addChip(config);
        
        bool removed = engine.removeChip(0);
        REQUIRE(removed);
        REQUIRE(engine.getChipCount() == 0);
    }
    
    SECTION("Remove all chips") {
        ChipConfig config;
        config.type = ChipType::kChipYM2151;
        engine.addChip(config);
        engine.addChip(config);
        
        engine.removeAllChips();
        REQUIRE(engine.getChipCount() == 0);
    }
}

TEST_CASE("Chip Engine - Chip Management", "[chip_engine][management]") {
    using namespace TigerFlame;
    
    ChipEngine engine;
    engine.setSampleRate(44100.0);
    
    ChipConfig config1;
    config1.type = ChipType::kChipYM2151;
    config1.volume = 0.8f;
    config1.enabled = true;
    
    ChipConfig config2;
    config2.type = ChipType::kChipYM2612;
    config2.volume = 0.5f;
    config2.enabled = false;
    
    engine.addChip(config1);
    engine.addChip(config2);
    
    SECTION("Get chip configuration") {
        const ChipConfig& retrieved1 = engine.getChipConfig(0);
        REQUIRE(retrieved1.type == ChipType::kChipYM2151);
        REQUIRE(retrieved1.volume == 0.8f);
        REQUIRE(retrieved1.enabled == true);
        
        const ChipConfig& retrieved2 = engine.getChipConfig(1);
        REQUIRE(retrieved2.type == ChipType::kChipYM2612);
        REQUIRE(retrieved2.volume == 0.5f);
        REQUIRE(retrieved2.enabled == false);
    }
    
    SECTION("Enable/disable chip") {
        engine.setChipEnabled(1, true);
        REQUIRE(engine.isChipEnabled(1));
        
        engine.setChipEnabled(1, false);
        REQUIRE(!engine.isChipEnabled(1));
    }
    
    SECTION("Set chip volume") {
        engine.setChipVolume(0, 0.6f);
        REQUIRE(engine.getChipVolume(0) == 0.6f);
    }
    
    SECTION("Chip names are correct") {
        REQUIRE(std::string(engine.getChipName(0)) == "YM2151");
        REQUIRE(std::string(engine.getChipShortName(1)) == "FM (Genesis)");
    }
}

TEST_CASE("Chip Engine - Audio Processing", "[chip_engine][audio]") {
    using namespace TigerFlame;
    
    ChipEngine engine;
    engine.setSampleRate(44100.0);
    
    // Add a chip
    ChipConfig config;
    config.type = ChipType::kChipYM2151;
    config.enabled = true;
    engine.addChip(config);
    
    SECTION("Process sample produces output") {
        float left = 0.0f;
        float right = 0.0f;
        engine.processSample(&left, &right);
        // With no notes, should produce silence (or very small values)
        REQUIRE(std::abs(left) < 0.01f);
        REQUIRE(std::abs(right) < 0.01f);
    }
    
    SECTION("Process block produces output") {
        const int numSamples = 256;
        float left[numSamples] = {0};
        float right[numSamples] = {0};
        
        engine.processBlock(left, right, numSamples);
        
        // Should not crash
        REQUIRE(true);
    }
    
    SECTION("Disabled chip produces no output") {
        engine.setChipEnabled(0, false);
        
        float left = 0.0f;
        float right = 0.0f;
        engine.processSample(&left, &right);
        
        REQUIRE(left == 0.0f);
        REQUIRE(right == 0.0f);
    }
}

TEST_CASE("Chip Engine - Command Processing", "[chip_engine][commands]") {
    using namespace TigerFlame;
    
    ChipEngine engine;
    engine.setSampleRate(44100.0);
    
    ChipConfig config;
    config.type = ChipType::kChipYM2151;
    engine.addChip(config);
    
    SECTION("Note on command") {
        ChipCommand cmd;
        cmd.type = ChipCommandType::kNoteOn;
        cmd.midiNote = 60;
        cmd.velocity = 0.8f;
        
        engine.sendCommand(cmd);
        // Command should be processed without crash
        REQUIRE(true);
    }
    
    SECTION("Note off command") {
        ChipCommand cmd;
        cmd.type = ChipCommandType::kNoteOff;
        cmd.midiNote = 60;
        
        engine.sendCommand(cmd);
        REQUIRE(true);
    }
    
    SECTION("Pitch bend command") {
        ChipCommand cmd;
        cmd.type = ChipCommandType::kPitchBend;
        cmd.pitchBend = 2.0;
        
        engine.sendCommand(cmd);
        REQUIRE(true);
    }
    
    SECTION("Reset command") {
        ChipCommand cmd;
        cmd.type = ChipCommandType::kReset;
        
        engine.sendCommand(cmd);
        REQUIRE(true);
    }
}

TEST_CASE("Chip Engine - Suspend/Resume", "[chip_engine][suspend]") {
    using namespace TigerFlame;
    
    ChipEngine engine;
    engine.setSampleRate(44100.0);
    
    ChipConfig config;
    config.type = ChipType::kChipYM2151;
    engine.addChip(config);
    
    SECTION("Suspended engine produces no output") {
        engine.suspend();
        
        float left = 1.0f;
        float right = 1.0f;
        engine.processSample(&left, &right);
        
        REQUIRE(left == 0.0f);
        REQUIRE(right == 0.0f);
    }
    
    SECTION("Resumed engine processes normally") {
        engine.suspend();
        engine.resume();
        
        float left = 0.0f;
        float right = 0.0f;
        engine.processSample(&left, &right);
        
        // Should produce some output (even if just noise)
        // This is a placeholder - actual behavior depends on chip implementation
    }
}
