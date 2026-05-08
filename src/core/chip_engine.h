// TigerFlame - Chip Engine
// Manages multiple sound chip instances and coordinates synthesis

#pragma once

#include "parameter_model.h"
#include "midi_translator.h"

#include "stream.h"

#include <vector>
#include <memory>
#include <cstdint>



namespace TigerFlame {

// Chip Instance base class
class ChipInstance {
public:
    virtual ~ChipInstance() = default;
    
    virtual void setSampleRate(double sampleRate) = 0;
    virtual void processSample(float* left, float* right) = 0;
    virtual void noteOn(int note, float velocity) = 0;
    virtual void noteOff(int note) = 0;
    virtual void pitchBend(double semitones) = 0;
    virtual void setParameter(int paramId, float value) = 0;
    virtual float getParameter(int paramId) const = 0;
    virtual void reset() = 0;
    virtual int getParamCount() const = 0;
    virtual const char* getName() const = 0;
    virtual const char* getShortName() const = 0;
};

// Chip configuration
struct ChipConfig {
    ChipType type = ChipType::kChipYM2151;
    float volume = 1.0f;
    bool enabled = true;
    int voiceCount = 8; // For polyphonic chips
    int paramCount = 0; // Number of parameters for this chip
};

// Chip Engine
// Manages multiple sound chip instances and produces audio output
class ChipEngine {
public:
    // Constructor
    ChipEngine();
    
    // Destructor
    ~ChipEngine();
    
    // Set sample rate
    void setSampleRate(double sampleRate);
    double getSampleRate() const { return sampleRate_; }
    
    // Chip management
    int addChip(const ChipConfig& config);
    bool removeChip(int chipIndex);
    void removeAllChips();
    int getChipCount() const { return static_cast<int>(chips_.size()); }
    
    // Get chip configuration
    const ChipConfig& getChipConfig(int chipIndex) const;
    ChipConfig& getChipConfig(int chipIndex);
    
    // Enable/disable chip
    void setChipEnabled(int chipIndex, bool enabled);
    bool isChipEnabled(int chipIndex) const;
    
    // Set chip volume
    void setChipVolume(int chipIndex, float volume);
    float getChipVolume(int chipIndex) const;
    
    // Send command to chip(s)
    void sendCommand(const ChipCommand& cmd);
    
    // Process a single sample
    void processSample(float* left, float* right);
    
    // Process a block of samples
    void processBlock(float* left, float* right, int numSamples);
    
    // Reset chip(s)
    void resetChip(int chipIndex);
    void resetAllChips();
    
    // Suspend/resume processing
    void suspend();
    void resume();
    
    // Get/set parameter for a chip
    float getParameter(int chipIndex, int paramId) const;
    void setParameter(int chipIndex, int paramId, float value);
    
    // Chip type information
    int getChipParamCount(int chipIndex) const;
    const char* getChipName(int chipIndex) const;
    const char* getChipShortName(int chipIndex) const;
    
    // Note on/off helpers
    void noteOn(int chipIndex, int note, float velocity);
    void noteOff(int chipIndex, int note);
    void pitchBend(int chipIndex, double semitones);
    void setChipParameter(int chipIndex, int paramId, float value);
    
    // State persistence
    void saveState(IStream* stream) const;
    void restoreState(IStream* stream);
    
    // Reset to initial state
    void reset();
    
    // Get output level (for CPU load monitoring)
    float getOutputLevel() const { return outputLevel_; }

private:
    // Sample rate
    double sampleRate_ = 44100.0;
    
    // Chip instances
    std::vector<ChipConfig> chipConfigs_;
    std::vector<ChipInstance*> chips_;
    
    // Processing state
    bool suspended_ = false;
    
    // Output level (for metering)
    float outputLevel_ = 0.0f;
    
    // Temporary buffer for block processing
    float tempLeft_ = 0.0f;
    float tempRight_ = 0.0f;
    
    // Create a chip instance based on type
    ChipInstance* createChipInstance(const ChipConfig& config);
    
    // Destroy a chip instance
    void destroyChipInstance(ChipInstance* chip);
};

} // namespace TigerFlame
