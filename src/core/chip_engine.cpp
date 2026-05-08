// TigerFlame - Chip Engine Implementation

#include "chip_engine.h"
#include "chip_bindings.h"
#include "state.h"

#include <algorithm>
#include <cmath>

namespace TigerFlame {

// Chip names
static const char* kChipNames[] = {
    "YM2151", "YM2612", "SN76489", "OPL2", "OPL3",
    "QSound", "C140", "POKEY", "VRC6"
};

static const char* kChipShortNames[] = {
    "FM (Arcade)", "FM (Genesis)", "PSG (SMS)", "FM (SB)", "FM (SB Pro)",
    "QSound", "Wave (Namco)", "POKEY (Atari)", "VRC6 (NES)"
};

// Chip parameter counts (placeholder values)
static const int kChipParamCounts[] = {
    64, 64, 16, 32, 64, 32, 32, 16, 16
};

// Constructor
ChipEngine::ChipEngine() {
    reset();
}

// Destructor
ChipEngine::~ChipEngine() {
    removeAllChips();
}

// Set sample rate
void ChipEngine::setSampleRate(double sampleRate) {
    sampleRate_ = sampleRate;
    for (auto* chip : chips_) {
        chip->setSampleRate(sampleRate);
    }
}

// Add chip
int ChipEngine::addChip(const ChipConfig& config) {
    ChipInstance* chip = createChipInstance(config);
    if (!chip) {
        return -1;
    }
    
    int index = static_cast<int>(chips_.size());
    chips_.push_back(chip);
    chipConfigs_.push_back(config);
    
    // Set sample rate
    chip->setSampleRate(sampleRate_);
    
    return index;
}

// Remove chip
bool ChipEngine::removeChip(int chipIndex) {
    if (chipIndex < 0 || chipIndex >= static_cast<int>(chips_.size())) {
        return false;
    }
    
    destroyChipInstance(chips_[chipIndex]);
    chips_.erase(chips_.begin() + chipIndex);
    chipConfigs_.erase(chipConfigs_.begin() + chipIndex);
    
    return true;
}

// Remove all chips
void ChipEngine::removeAllChips() {
    for (auto* chip : chips_) {
        destroyChipInstance(chip);
    }
    chips_.clear();
    chipConfigs_.clear();
}

// Get chip configuration
const ChipConfig& ChipEngine::getChipConfig(int chipIndex) const {
    static const ChipConfig kDefaultConfig;
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chipConfigs_.size())) {
        return chipConfigs_[chipIndex];
    }
    return kDefaultConfig;
}

ChipConfig& ChipEngine::getChipConfig(int chipIndex) {
    static ChipConfig kDefaultConfig;
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chipConfigs_.size())) {
        return chipConfigs_[chipIndex];
    }
    return kDefaultConfig;
}

// Enable/disable chip
void ChipEngine::setChipEnabled(int chipIndex, bool enabled) {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chipConfigs_.size())) {
        chipConfigs_[chipIndex].enabled = enabled;
    }
}

bool ChipEngine::isChipEnabled(int chipIndex) const {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chipConfigs_.size())) {
        return chipConfigs_[chipIndex].enabled;
    }
    return false;
}

// Set chip volume
void ChipEngine::setChipVolume(int chipIndex, float volume) {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chipConfigs_.size())) {
        chipConfigs_[chipIndex].volume = std::max(0.0f, std::min(volume, 1.0f));
    }
}

float ChipEngine::getChipVolume(int chipIndex) const {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chipConfigs_.size())) {
        return chipConfigs_[chipIndex].volume;
    }
    return 1.0f;
}

// Send command
void ChipEngine::sendCommand(const ChipCommand& cmd) {
    if (suspended_) return;
    
    switch (cmd.type) {
        case ChipCommandType::kNoteOn:
            if (cmd.chipIndex == -1) {
                // Send to all enabled chips
                for (int i = 0; i < static_cast<int>(chips_.size()); ++i) {
                    if (isChipEnabled(i)) {
                        noteOn(i, cmd.midiNote, cmd.velocity);
                    }
                }
            } else if (cmd.chipIndex >= 0 && cmd.chipIndex < static_cast<int>(chips_.size())) {
                if (isChipEnabled(cmd.chipIndex)) {
                    noteOn(cmd.chipIndex, cmd.midiNote, cmd.velocity);
                }
            }
            break;
            
        case ChipCommandType::kNoteOff:
            if (cmd.chipIndex == -1) {
                for (int i = 0; i < static_cast<int>(chips_.size()); ++i) {
                    if (isChipEnabled(i)) {
                        noteOff(i, cmd.midiNote);
                    }
                }
            } else if (cmd.chipIndex >= 0 && cmd.chipIndex < static_cast<int>(chips_.size())) {
                if (isChipEnabled(cmd.chipIndex)) {
                    noteOff(cmd.chipIndex, cmd.midiNote);
                }
            }
            break;
            
        case ChipCommandType::kPitchBend:
            if (cmd.chipIndex == -1) {
                for (int i = 0; i < static_cast<int>(chips_.size()); ++i) {
                    if (isChipEnabled(i)) {
                        pitchBend(i, cmd.pitchBend);
                    }
                }
            } else if (cmd.chipIndex >= 0 && cmd.chipIndex < static_cast<int>(chips_.size())) {
                if (isChipEnabled(cmd.chipIndex)) {
                    pitchBend(cmd.chipIndex, cmd.pitchBend);
                }
            }
            break;
            
        case ChipCommandType::kSetParameter:
            if (cmd.chipIndex >= 0 && cmd.chipIndex < static_cast<int>(chips_.size())) {
                if (isChipEnabled(cmd.chipIndex)) {
                    setChipParameter(cmd.chipIndex, cmd.paramId, cmd.paramValue);
                }
            }
            break;
            
        case ChipCommandType::kControlChange:
            // Forward to all chips or specific chip
            break;
            
        case ChipCommandType::kReset:
            if (cmd.chipIndex == -1) {
                resetAllChips();
            } else if (cmd.chipIndex >= 0 && cmd.chipIndex < static_cast<int>(chips_.size())) {
                resetChip(cmd.chipIndex);
            }
            break;
            
        case ChipCommandType::kAllNotesOff:
            // Send all notes off to all chips
            for (int i = 0; i < static_cast<int>(chips_.size()); ++i) {
                if (isChipEnabled(i)) {
                    // Chips will handle all notes off internally
                }
            }
            break;
    }
}

// Process a single sample
void ChipEngine::processSample(float* left, float* right) {
    if (suspended_) {
        *left = 0.0f;
        *right = 0.0f;
        return;
    }
    
    float l = 0.0f;
    float r = 0.0f;
    
    for (size_t i = 0; i < chips_.size(); ++i) {
        if (isChipEnabled(static_cast<int>(i))) {
            float chipLeft = 0.0f;
            float chipRight = 0.0f;
            chips_[i]->processSample(&chipLeft, &chipRight);
            
            // Apply chip volume
            float volume = getChipVolume(static_cast<int>(i));
            l += chipLeft * volume;
            r += chipRight * volume;
        }
    }
    
    // Track output level
    outputLevel_ = 0.9f * outputLevel_ + 0.1f * std::max(std::abs(l), std::abs(r));
    
    *left = l;
    *right = r;
}

// Process a block of samples
void ChipEngine::processBlock(float* left, float* right, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        processSample(&left[i], &right[i]);
    }
}

// Reset chip
void ChipEngine::resetChip(int chipIndex) {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chips_.size())) {
        chips_[chipIndex]->reset();
    }
}

// Reset all chips
void ChipEngine::resetAllChips() {
    for (auto* chip : chips_) {
        chip->reset();
    }
}

// Suspend
void ChipEngine::suspend() {
    suspended_ = true;
}

// Resume
void ChipEngine::resume() {
    suspended_ = false;
}

// Get parameter
float ChipEngine::getParameter(int chipIndex, int paramId) const {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chips_.size())) {
        return chips_[chipIndex]->getParameter(paramId);
    }
    return 0.0f;
}

// Set parameter
void ChipEngine::setParameter(int chipIndex, int paramId, float value) {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chips_.size())) {
        chips_[chipIndex]->setParameter(paramId, value);
    }
}

// Get chip parameter count
int ChipEngine::getChipParamCount(int chipIndex) const {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chips_.size())) {
        return chips_[chipIndex]->getParamCount();
    }
    return 0;
}

// Get chip name
const char* ChipEngine::getChipName(int chipIndex) const {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chipConfigs_.size())) {
        ChipType type = chipConfigs_[chipIndex].type;
        int index = static_cast<int>(type);
        if (index >= 0 && index < static_cast<int>(sizeof(kChipNames) / sizeof(kChipNames[0]))) {
            return kChipNames[index];
        }
    }
    return "Unknown";
}

// Get chip short name
const char* ChipEngine::getChipShortName(int chipIndex) const {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chipConfigs_.size())) {
        ChipType type = chipConfigs_[chipIndex].type;
        int index = static_cast<int>(type);
        if (index >= 0 && index < static_cast<int>(sizeof(kChipShortNames) / sizeof(kChipShortNames[0]))) {
            return kChipShortNames[index];
        }
    }
    return "Unknown";
}

// Note on helper
void ChipEngine::noteOn(int chipIndex, int note, float velocity) {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chips_.size()) && isChipEnabled(chipIndex)) {
        chips_[chipIndex]->noteOn(note, velocity);
    }
}

// Note off helper
void ChipEngine::noteOff(int chipIndex, int note) {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chips_.size()) && isChipEnabled(chipIndex)) {
        chips_[chipIndex]->noteOff(note);
    }
}

// Pitch bend helper
void ChipEngine::pitchBend(int chipIndex, double semitones) {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chips_.size()) && isChipEnabled(chipIndex)) {
        chips_[chipIndex]->pitchBend(semitones);
    }
}

// Set chip parameter helper
void ChipEngine::setChipParameter(int chipIndex, int paramId, float value) {
    if (chipIndex >= 0 && chipIndex < static_cast<int>(chips_.size()) && isChipEnabled(chipIndex)) {
        chips_[chipIndex]->setParameter(paramId, value);
    }
}

// Save state
void ChipEngine::saveState(IStream* stream) const {
    if (!stream) return;
    
    StateUtil::writeChunkHeader(stream, StateUtil::kChipEngineMagic, 1, 0);
    
    // Save number of chips
    uint32_t chipCount = static_cast<uint32_t>(chips_.size());
    stream->write(&chipCount, sizeof(chipCount));
    
    // Save each chip
    for (uint32_t i = 0; i < chipCount; ++i) {
        // Save chip config
        stream->write(&chipConfigs_[i], sizeof(chipConfigs_[i]));
        
        // Save chip state (via virtual method)
        // This will be implemented by each chip instance
    }
}

// Restore state
void ChipEngine::restoreState(IStream* stream) {
    if (!stream) return;
    
    uint32_t magic, version, size;
    if (!StateUtil::readChunkHeader(stream, magic, version, size)) {
        return;
    }
    
    if (magic != StateUtil::kChipEngineMagic || version > 1) {
        return;
    }
    
    // Remove all existing chips
    removeAllChips();
    
    // Read number of chips
    uint32_t chipCount;
    if (stream->read(&chipCount, sizeof(chipCount)) != StreamResult::kOk) return;
    
    // Read each chip
    for (uint32_t i = 0; i < chipCount; ++i) {
        ChipConfig config;
        if (stream->read(&config, sizeof(config)) != StreamResult::kOk) return;
        
        int index = addChip(config);
        if (index < 0) return;
        
        // Restore chip state
        // This will be implemented by each chip instance
    }
}

// Reset
void ChipEngine::reset() {
    suspended_ = false;
    outputLevel_ = 0.0f;
    removeAllChips();
}

// Create chip instance
ChipInstance* ChipEngine::createChipInstance(const ChipConfig& config) {
    // For now, create a placeholder chip instance
    // In Phase 2, this will connect to mml2vgm
    
    class PlaceholderChip : public ChipInstance {
    public:
        PlaceholderChip(const ChipConfig& cfg) : config_(cfg) {
            // Simple sine wave oscillator as placeholder
            phase_ = 0.0;
            frequency_ = 0.0;
            amplitude_ = 0.0f;
        }
        
        void setSampleRate(double sampleRate) override {
            sampleRate_ = sampleRate;
        }
        
        void processSample(float* left, float* right) override {
            // Simple sine wave
            phase_ += frequency_ * 2.0 * 3.1415926535 / sampleRate_;
            if (phase_ > 2.0 * 3.1415926535) {
                phase_ -= 2.0 * 3.1415926535;
            }
            
            float sample = static_cast<float>(std::sin(phase_) * amplitude_);
            *left += sample;
            *right += sample;
        }
        
        void noteOn(int note, float velocity) override {
            // Convert MIDI note to frequency
            frequency_ = 440.0 * std::pow(2.0, (note - 69) / 12.0);
            amplitude_ = velocity * 0.5f;
            phase_ = 0.0;
        }
        
        void noteOff(int note) override {
            amplitude_ = 0.0f;
        }
        
        void pitchBend(double semitones) override {
            // Apply pitch bend
            frequency_ *= std::pow(2.0, semitones / 12.0);
        }
        
        void setParameter(int paramId, float value) override {
            // Placeholder: no parameters yet
        }
        
        float getParameter(int paramId) const override {
            return 0.0f;
        }
        
        void reset() override {
            phase_ = 0.0;
            frequency_ = 0.0;
            amplitude_ = 0.0f;
        }
        
        int getParamCount() const override {
            return 0;
        }
        
        const char* getName() const override {
            return kChipNames[static_cast<int>(config_.type)];
        }
        
        const char* getShortName() const override {
            return kChipShortNames[static_cast<int>(config_.type)];
        }
    
    private:
        ChipConfig config_;
        double sampleRate_ = 44100.0;
        double phase_ = 0.0;
        double frequency_ = 0.0;
        float amplitude_ = 0.0f;
    };
    
    return new PlaceholderChip(config);
}

// Destroy chip instance
void ChipEngine::destroyChipInstance(ChipInstance* chip) {
    delete chip;
}

} // namespace TigerFlame
