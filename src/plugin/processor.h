// TigerFlame VST3 Plugin - Audio Processor
// This is the main audio processing class for the VST3 plugin

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include "core/plugin_state.h"

#include "ids.h"
#include "version.h"

// Forward declarations
namespace TigerFlame {
    class ChipEngine;
    class VoiceAllocator;
    class MidiTranslator;
    class EffectChain;
}

namespace TigerFlame {

// Audio processor class
class Processor : public Steinberg::Vst::AudioEffect {
public:
    // Constructor
    Processor();
    
    // Destructor
    ~Processor() override;
    
    // IAudioProcessor interface
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API terminate() override;
    Steinberg::tresult PLUGIN_API setBusArrangements(
        Steinberg::Vst::SpeakerArrangement* inputs,
        Steinberg::int32 numIns,
        Steinberg::Vst::SpeakerArrangement* outputs,
        Steinberg::int32 numOuts) override;
    Steinberg::tresult PLUGIN_API setupProcessing(
        Steinberg::Vst::ProcessSetup& setup) override;
    Steinberg::tresult PLUGIN_API setProcessing(Steinberg::TBool state) override;
    Steinberg::tresult PLUGIN_API canProcessSampleSize(
        Steinberg::int32 symbolicSampleSize) override;
    
    // IProcess interface
    Steinberg::tresult PLUGIN_API process(
        Steinberg::Vst::ProcessData& data) override;
    
    // IComponent interface
    Steinberg::tresult PLUGIN_API getControllerClassId(
        Steinberg::TUID classId) override;
    
    // State persistence
    Steinberg::tresult PLUGIN_API setState(
        Steinberg::IBStream* stream) override;
    Steinberg::tresult PLUGIN_API getState(
        Steinberg::IBStream* stream) override;
    
    // Static factory method
    static Steinberg::FUnknown* createInstance(void* /*context*/) {
        return static_cast<Steinberg::Vst::IAudioProcessor*>(new Processor());
    }
    
    // Process audio (template for float/double)
    template<typename SampleType>
    void processAudio(
        Steinberg::Vst::ProcessData& data,
        SampleType* const* outputs,
        Steinberg::int32 numSamples);
    
    // MIDI event processing
    void processMidiEvents(
        Steinberg::Vst::IEventList* events,
        Steinberg::int32 numSamples);
    
    // Parameter changes processing
    void processParameterChanges(
        Steinberg::Vst::IParamValueQueue* queue,
        Steinberg::int32 numSamples);
    
    // Get sample rate
    double getSampleRate() const { return sampleRate_; }
    
    // Get block size
    Steinberg::int32 getBlockSize() const { return blockSize_; }
    
    // Accessors for UI
    ChipEngine* getChipEngine() { return chipEngine_; }
    const ChipEngine* getChipEngine() const { return chipEngine_; }
    
    VoiceAllocator* getVoiceAllocator() { return voiceAllocator_; }
    const VoiceAllocator* getVoiceAllocator() const { return voiceAllocator_; }
    
    EffectChain* getEffectChain() { return effectChain_; }
    const EffectChain* getEffectChain() const { return effectChain_; }
    
    PluginState* getPluginState() { return &pluginState_; }
    const PluginState* getPluginState() const { return &pluginState_; }

private:
    // Initialize core components
    void initializeCore();
    void cleanupCore();
    
    // Plugin state
    PluginState pluginState_;
    
    // Core components
    ChipEngine* chipEngine_ = nullptr;
    VoiceAllocator* voiceAllocator_ = nullptr;
    MidiTranslator* midiTranslator_ = nullptr;
    EffectChain* effectChain_ = nullptr;
    
    // Audio processing state
    double sampleRate_ = 44100.0;
    Steinberg::int32 blockSize_ = 512;
    Steinberg::int64 elapsedSamples_ = 0;
    
    // Buffer for accumulating samples
    float* tempBuffer_ = nullptr;
    Steinberg::int32 tempBufferSize_ = 0;
    
    // Process flags
    bool processingActive_ = false;
    bool wasProcessing_ = false;
    
    // Disallow copying
    Processor(const Processor&) = delete;
    Processor& operator=(const Processor&) = delete;
};

} // namespace TigerFlame
