// TigerFlame - Plugin State Tests

#include "catch2/catch_all.hpp"

#include "core/plugin_state.h"
#include "core/parameter_model.h"

TEST_CASE("Plugin State - Default Values", "[plugin_state][defaults]") {
    using namespace TigerFlame;
    
    PluginState state;
    
    SECTION("Voice mode defaults to Mono") {
        REQUIRE(state.getVoiceMode() == VoiceMode::kMono);
    }
    
    SECTION("Master volume defaults to 80%") {
        REQUIRE(state.getMasterVolume() == 0.8f);
    }
    
    SECTION("Chip type defaults to YM2151") {
        REQUIRE(state.getChipType() == ChipType::kChipYM2151);
    }
    
    SECTION("Pitch bend range defaults to 2 semitones") {
        REQUIRE(state.getPitchBendRange() == 2.0f);
    }
}

TEST_CASE("Plugin State - Parameter Access", "[plugin_state][access]") {
    using namespace TigerFlame;
    
    PluginState state;
    
    SECTION("Get and set normalized values") {
        state.setNormalizedValue(ParamId::kMasterVolume, 0.5f);
        REQUIRE(state.getNormalizedValue(ParamId::kMasterVolume) == 0.5f);
    }
    
    SECTION("Get and set domain values") {
        state.setValue(ParamId::kMasterTuning, 50.0); // 50 cents
        REQUIRE(state.getValue<float>(ParamId::kMasterTuning) == 50.0f);
    }
    
    SECTION("Voice mode conversion") {
        state.setVoiceMode(VoiceMode::kPolyphonic);
        REQUIRE(state.getVoiceMode() == VoiceMode::kPolyphonic);
        REQUIRE(state.getNormalizedValue(ParamId::kVoiceMode) > 0.5f);
    }
    
    SECTION("Chip type conversion") {
        state.setChipType(ChipType::kChipOPL3);
        REQUIRE(state.getChipType() == ChipType::kChipOPL3);
    }
}

TEST_CASE("Plugin State - Vector Conversion", "[plugin_state][vector]") {
    using namespace TigerFlame;
    
    PluginState state;
    
    SECTION("To vector creates correct size") {
        auto vec = state.toVector();
        REQUIRE(vec.size() == static_cast<size_t>(ParamId::kParamCount));
    }
    
    SECTION("From vector sets all values") {
        std::vector<float> values(static_cast<size_t>(ParamId::kParamCount), 0.5f);
        state.fromVector(values);
        
        for (int i = 0; i < static_cast<int>(ParamId::kParamCount); ++i) {
            REQUIRE(state.getNormalizedValue(static_cast<ParamId>(i)) == 0.5f);
        }
    }
}

TEST_CASE("Plugin State - Reset", "[plugin_state][reset]") {
    using namespace TigerFlame;
    
    PluginState state;
    
    // Modify some values
    state.setMasterVolume(0.3f);
    state.setVoiceMode(VoiceMode::kPolyphonic);
    state.setChipType(ChipType::kChipSN76489);
    
    SECTION("Reset restores defaults") {
        state.reset();
        
        REQUIRE(state.getMasterVolume() == 0.8f);
        REQUIRE(state.getVoiceMode() == VoiceMode::kMono);
        REQUIRE(state.getChipType() == ChipType::kChipYM2151);
    }
}

TEST_CASE("Plugin State - Parameter Ranges", "[plugin_state][ranges]") {
    using namespace TigerFlame;
    
    SECTION("Master volume clamped to 0-1") {
        PluginState state;
        state.setMasterVolume(-0.5f);
        REQUIRE(state.getMasterVolume() == 0.0f);
        
        state.setMasterVolume(1.5f);
        REQUIRE(state.getMasterVolume() == 1.0f);
    }
    
    SECTION("Master tuning clamped to 0-1") {
        PluginState state;
        state.setMasterTuning(-0.1f);
        REQUIRE(state.getMasterTuning() == 0.0f);
        
        state.setMasterTuning(1.1f);
        REQUIRE(state.getMasterTuning() == 1.0f);
    }
}
