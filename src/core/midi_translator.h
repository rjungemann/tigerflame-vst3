// TigerFlame - MIDI Translator
// Converts MIDI events to chip-specific commands

#pragma once

#include "parameter_model.h"
#include "stream.h"

#include <vector>
#include <functional>
#include <cstdint>

namespace TigerFlame {

class ChipEngine;

// Chip command structure
enum class ChipCommandType : uint8_t {
    kNoteOn,
    kNoteOff,
    kSetParameter,
    kPitchBend,
    kControlChange,
    kReset,
    kAllNotesOff
};

struct ChipCommand {
    int chipIndex = -1;           // Which chip to send to (-1 = all chips)
    ChipCommandType type = ChipCommandType::kNoteOn;
    
    // For note on/off
    int midiNote = -1;
    float velocity = 0.0f;
    
    // For parameter changes
    int paramId = -1;
    float paramValue = 0.0f;
    
    // For pitch bend
    double pitchBend = 0.0;
    
    // For CC
    int ccNumber = -1;
    float ccValue = 0.0f;
};

// Callback type for command dispatch
using ChipCommandCallback = std::function<void(const ChipCommand&)>;

// MIDI Translator
// Converts MIDI events to chip-specific commands
class MidiTranslator {
public:
    // Constructor
    MidiTranslator();
    
    // Destructor
    ~MidiTranslator();
    
    // Set chip engine for command dispatch
    void setChipEngine(ChipEngine* engine);
    ChipEngine* getChipEngine() const { return chipEngine_; }
    
    // Set command callback (alternative to chip engine)
    void setOnCommand(ChipCommandCallback callback) { onCommand_ = callback; }
    
    // Register chip type for parameter mapping
    void registerChip(int chipIndex, ChipType type, int paramCount);
    
    // Translate MIDI events to chip commands
    void translateNoteOn(int note, float velocity);
    void translateNoteOff(int note);
    void translatePitchBend(double semitones);
    void translateControlChange(int cc, float value);
    void translateAftertouch(int note, float pressure);
    void translatePolyPressure(int note, float pressure);
    
    // Map MIDI CC to chip parameter
    void mapCC(int cc, int chipIndex, int paramId, float minValue = 0.0f, float maxValue = 1.0f);
    
    // Get mapped parameter value
    float getMappedCCValue(int cc) const;
    
    // Set note transpose
    void setNoteTranspose(int semitones) { noteTranspose_ = semitones; }
    int getNoteTranspose() const { return noteTranspose_; }
    
    // Set velocity sensitivity
    void setVelocitySensitivity(float sensitivity) { velocitySensitivity_ = sensitivity; }
    float getVelocitySensitivity() const { return velocitySensitivity_; }
    
    // Process queued commands (called per sample block)
    void processQueue();
    
    // Clear pending commands
    void clearQueue();
    
    // Get pending commands (for debugging)
    const std::vector<ChipCommand>& getPendingCommands() const { return commandQueue_; }
    
    // State persistence
    void saveState(IStream* stream) const;
    void restoreState(IStream* stream);

private:
    // Chip engine reference
    ChipEngine* chipEngine_ = nullptr;
    
    // Command callback
    ChipCommandCallback onCommand_;
    
    // Pending commands queue
    std::vector<ChipCommand> commandQueue_;
    
    // CC mapping: CC number -> (chipIndex, paramId, min, max)
    struct CCMapping {
        int chipIndex = -1;
        int paramId = -1;
        float minValue = 0.0f;
        float maxValue = 1.0f;
    };
    std::vector<CCMapping> ccMappings_;
    
    // Chip parameter counts (for validation)
    std::vector<int> chipParamCounts_;
    
    // Note transpose (-48 to +48 semitones)
    int noteTranspose_ = 0;
    
    // Velocity sensitivity (0.0 - 2.0)
    float velocitySensitivity_ = 1.0f;
    
    // Dispatch command to chip engine or callback
    void dispatchCommand(const ChipCommand& cmd);
};

} // namespace TigerFlame
