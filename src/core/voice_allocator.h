// TigerFlame - Voice Allocator
// Manages voice allocation across multiple chips and modes

#pragma once

#include "parameter_model.h"
#include "stream.h"

#include <vector>
#include <cstdint>



namespace TigerFlame {

class MidiTranslator;

// Voice allocation for a single note
typedef int32_t VoiceId;

struct VoiceAllocation {
    int midiNote = -1;           // MIDI note number (0-127)
    float velocity = 0.0f;      // Normalized velocity (0.0-1.0)
    double pitchBend = 0.0;      // Pitch bend in semitones
    int32_t chipIndex = -1;      // Which chip is this voice on
    int32_t voiceIndex = -1;     // Which voice slot on that chip
    int64_t noteOnTime = -1;     // Sample counter when note was triggered
    bool sustain = false;        // Is sustain pedal holding this note
};

// State for monophonic mode
struct MonoVoiceState {
    int activeNote = -1;        // Currently playing note
    int activeChipIndex = -1;   // Which chip
    int activeVoiceIndex = 0;   // Which voice on that chip
    double pitchBend = 0.0;      // Current pitch bend
    float velocity = 0.0f;      // Current velocity
    int64_t noteOnTime = -1;    // Sample counter at note-on
    bool sustain = false;       // Sustain state
};

// State for polyphonic mode (per voice slot)
struct PolyVoiceSlot {
    int midiNote = -1;           // Note number
    float velocity = 0.0f;      // Normalized velocity
    double pitchBend = 0.0;      // Pitch bend at note-on
    int64_t noteOnTime = -1;     // Sample counter
    bool sustain = false;        // Sustain state
};

// Layer configuration
struct LayerConfig {
    int chipIndex = -1;         // Which chip
    int instrumentId = 0;       // Chip-specific instrument number
    float volume = 1.0f;        // Layer gain (0.0-1.0)
    bool enabled = true;        // Layer on/off
};

// Voice Allocator
// Manages distribution of MIDI notes across chip instances and voices
class VoiceAllocator {
public:
    // Voice allocation mode
enum class Mode {
        kMono,
        kLayeredMultitimbral,
        kPolyphonic
    };
    
    // Constructor
    VoiceAllocator();
    
    // Destructor
    ~VoiceAllocator();
    
    // Set the mode
    void setMode(Mode mode);
    Mode getMode() const { return mode_; }
    
    // Set sample rate
    void setSampleRate(double sampleRate) { sampleRate_ = sampleRate; }
    double getSampleRate() const { return sampleRate_; }
    
    // Set MIDI translator (for forwarding commands)
    void setMidiTranslator(MidiTranslator* translator) { midiTranslator_ = translator; }
    
    // Set polyphony count (for polyphonic mode)
    void setPolyphonyCount(int voicesPerChip);
    int getPolyphonyCount() const { return polyphonyCount_; }
    
    // Layer management (for layered multitimbral mode)
    void addLayer(const LayerConfig& config);
    void removeLayer(int layerIndex);
    void updateLayer(int layerIndex, const LayerConfig& config);
    int getLayerCount() const { return static_cast<int>(layers_.size()); }
    const LayerConfig& getLayer(int index) const;
    
    // MIDI event handling
    void noteOn(int note, float velocity);
    void noteOff(int note);
    void pitchBend(double semitones);
    void controlChange(int cc, float value);
    void allNotesOff();
    void allSoundOff();
    
    // Query active voices
    const std::vector<VoiceAllocation>& getActiveVoices() const { return activeVoices_; }
    int getVoiceCount() const { return static_cast<int>(activeVoices_.size()); }
    bool isNoteActive(int note) const;
    
    // Get voice allocation for a specific note
    const VoiceAllocation* getVoiceAllocation(int note) const;
    
    // State persistence
    void saveState(IStream* stream) const;
    void restoreState(IStream* stream);
    
    // Reset to initial state
    void reset();
    
    // Get current voice state for serialization
    const MonoVoiceState& getMonoState() const { return monoState_; }
    const std::vector<PolyVoiceSlot>& getPolyState() const { return polySlots_; }
    const std::vector<LayerConfig>& getLayers() const { return layers_; }
    
    // Increment sample counter (called by processor)
    void incrementSampleCounter(int32_t count = 1) { sampleCounter_ += count; }

private:
    // Voice allocation mode
    Mode mode_ = Mode::kMono;
    
    // Sample rate
    double sampleRate_ = 44100.0;
    
    // Sample counter for LRU voice stealing
    int64_t sampleCounter_ = 0;
    
    // MIDI translator for forwarding commands
    MidiTranslator* midiTranslator_ = nullptr;
    
    // Monophonic mode state
    MonoVoiceState monoState_;
    
    // Polyphonic mode state
    int polyphonyCount_ = 8; // Default: 8 voices
    std::vector<PolyVoiceSlot> polySlots_;
    
    // Layered multitimbral mode state
    std::vector<LayerConfig> layers_;
    std::vector<MonoVoiceState> layerStates_;
    
    // Current active voice allocations (for all modes)
    std::vector<VoiceAllocation> activeVoices_;
    
    // Mode-specific note on/off handling
    void noteOnMono(int note, float velocity);
    void noteOffMono(int note);
    void noteOnLayered(int note, float velocity);
    void noteOffLayered(int note);
    void noteOnPoly(int note, float velocity);
    void noteOffPoly(int note);
    
    // Find free polyphonic voice slot
    int findFreePolySlot();
    
    // Find oldest polyphonic voice slot (for LRU stealing)
    int findOldestPolySlot();
    
    // Update active voices list
    void updateActiveVoices();
};

} // namespace TigerFlame
