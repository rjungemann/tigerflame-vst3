// TigerFlame - MIDI Translator Tests

#include "catch2/catch_all.hpp"

#include "core/midi_translator.h"

TEST_CASE("MIDI Translator - Note Translation", "[midi_translator][notes]") {
    using namespace TigerFlame;
    
    MidiTranslator translator;
    
    SECTION("Note on produces correct ChipCommand") {
        translator.translateNoteOn(60, 0.8f);
        auto& commands = translator.getPendingCommands();
        REQUIRE(commands.size() == 1);
        REQUIRE(commands[0].type == ChipCommandType::kNoteOn);
        REQUIRE(commands[0].midiNote == 60);
        REQUIRE(commands[0].velocity == 0.8f);
    }
    
    SECTION("Note off produces correct ChipCommand") {
        translator.translateNoteOff(60);
        auto& commands = translator.getPendingCommands();
        REQUIRE(commands.size() == 1);
        REQUIRE(commands[0].type == ChipCommandType::kNoteOff);
        REQUIRE(commands[0].midiNote == 60);
    }
    
    SECTION("Pitch bend produces correct ChipCommand") {
        translator.translatePitchBend(2.0);
        auto& commands = translator.getPendingCommands();
        REQUIRE(commands.size() == 1);
        REQUIRE(commands[0].type == ChipCommandType::kPitchBend);
        REQUIRE(commands[0].pitchBend == 2.0);
    }
    
    SECTION("Note transpose affects note numbers") {
        translator.setNoteTranspose(12); // +1 octave
        translator.translateNoteOn(60, 0.8f);
        auto& commands = translator.getPendingCommands();
        REQUIRE(commands[0].midiNote == 72); // 60 + 12
    }
    
    SECTION("Velocity sensitivity affects velocity") {
        translator.setVelocitySensitivity(2.0f); // 200%
        translator.translateNoteOn(60, 0.5f);
        auto& commands = translator.getPendingCommands();
        REQUIRE(commands[0].velocity == 1.0f); // 0.5 * 2.0 = 1.0 (clamped)
    }
}

TEST_CASE("MIDI Translator - CC Mapping", "[midi_translator][cc]") {
    using namespace TigerFlame;
    
    MidiTranslator translator;
    
    SECTION("CC mapping stores correctly") {
        translator.mapCC(1, 0, 5, 0.0f, 1.0f); // Mod wheel to chip 0, param 5
        // Can't easily verify without accessing internal state
        // This is a placeholder for when we add getter methods
    }
    
    SECTION("Control change with mapped CC produces SetParameter command") {
        translator.mapCC(7, 0, 1, 0.0f, 1.0f); // Volume CC to chip 0, param 1
        translator.translateControlChange(7, 0.5f);
        auto& commands = translator.getPendingCommands();
        REQUIRE(commands.size() == 1);
        REQUIRE(commands[0].type == ChipCommandType::kSetParameter);
        REQUIRE(commands[0].chipIndex == 0);
        REQUIRE(commands[0].paramId == 1);
        REQUIRE(commands[0].paramValue == 0.5f);
    }
    
    SECTION("CC value is scaled to mapped range") {
        translator.mapCC(11, 0, 2, 0.0f, 2.0f); // Expression to param with range 0-2
        translator.translateControlChange(11, 0.5f); // 50% CC value
        auto& commands = translator.getPendingCommands();
        REQUIRE(commands[0].paramValue == 1.0f); // 0.5 * 2.0 = 1.0
    }
}

TEST_CASE("MIDI Translator - Command Queue", "[midi_translator][queue]") {
    using namespace TigerFlame;
    
    MidiTranslator translator;
    
    SECTION("Commands are queued") {
        translator.translateNoteOn(60, 0.8f);
        translator.translateNoteOn(64, 0.6f);
        translator.translateNoteOff(60);
        
        auto& commands = translator.getPendingCommands();
        REQUIRE(commands.size() == 3);
    }
    
    SECTION("Process queue clears commands") {
        translator.translateNoteOn(60, 0.8f);
        translator.translateNoteOn(64, 0.6f);
        
        translator.processQueue();
        auto& commands = translator.getPendingCommands();
        REQUIRE(commands.empty());
    }
    
    SECTION("Clear queue removes all commands") {
        translator.translateNoteOn(60, 0.8f);
        translator.translateNoteOn(64, 0.6f);
        
        translator.clearQueue();
        auto& commands = translator.getPendingCommands();
        REQUIRE(commands.empty());
    }
}
