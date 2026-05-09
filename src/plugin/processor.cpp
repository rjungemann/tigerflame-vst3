// TigerFlame VST3 Plugin - Audio Processor Implementation

#include "processor.h"

#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/base/fstrdefs.h"
#include "pluginterfaces/base/ibstream.h"

#include "core/chip_engine.h"
#include "core/voice_allocator.h"
#include "core/midi_translator.h"
#include "core/state.h"
#include "core/plugin_state.h"
#include "core/effect_chain.h"
#include "core/stream.h"

#include <cstring>
#include <algorithm>

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace TigerFlame {

// Adapter: wraps Steinberg::IBStream as TigerFlame::IStream
class IBStreamAdapter : public IStream {
public:
    explicit IBStreamAdapter(Steinberg::IBStream* s) : stream_(s) {}

    StreamResult write(const void* buffer, size_t size) override {
        int32 written = 0;
        return (stream_->write(const_cast<void*>(buffer),
                               static_cast<int32>(size), &written) == Steinberg::kResultOk)
            ? StreamResult::kOk : StreamResult::kError;
    }

    StreamResult read(void* buffer, size_t size) override {
        int32 bytesRead = 0;
        return (stream_->read(buffer, static_cast<int32>(size), &bytesRead) == Steinberg::kResultOk)
            ? StreamResult::kOk : StreamResult::kError;
    }

    StreamResult seek(int64_t position) override {
        int64 result = 0;
        return (stream_->seek(position, Steinberg::IBStream::kIBSeekSet, &result) == Steinberg::kResultOk)
            ? StreamResult::kOk : StreamResult::kError;
    }

    int64_t tell() const override {
        int64 pos = 0;
        stream_->tell(&pos);
        return pos;
    }

    int64_t getSize() const override { return -1; }

private:
    Steinberg::IBStream* stream_;
};

// Constructor
Processor::Processor() : AudioEffect() {
    // Initialize with default values
    setControllerClass(TigerFlameControllerUID);
    
    // We're an instrument (no audio input, stereo audio output)
    // Bus configuration will be set in setBusArrangements
}

// Destructor
Processor::~Processor() {
    cleanupCore();
}

// Initialize
tresult PLUGIN_API Processor::initialize(FUnknown* context) {
    tresult result = AudioEffect::initialize(context);
    if (result != kResultOk) {
        return result;
    }
    
    // Initialize core components
    initializeCore();
    
    return kResultOk;
}

// Terminate
tresult PLUGIN_API Processor::terminate() {
    cleanupCore();
    return AudioEffect::terminate();
}

// Set bus arrangements
tresult PLUGIN_API Processor::setBusArrangements(
    SpeakerArrangement* inputs,
    int32 numIns,
    SpeakerArrangement* outputs,
    int32 numOuts) {
    
    // We're an instrument plugin: 0 audio inputs, 1 stereo output
    if (numIns != 0 || numOuts != 1) {
        return kResultFalse;
    }
    
    // Check output arrangement is stereo
    if (outputs && numOuts > 0) {
        if (outputs[0] != SpeakerArr::kStereo) {
            return kResultFalse;
        }
    }
    
    return kResultOk;
}

// Setup processing
tresult PLUGIN_API Processor::setupProcessing(
    ProcessSetup& setup) {
    
    sampleRate_ = setup.sampleRate;
    blockSize_ = setup.maxSamplesPerBlock;
    
    // Allocate temporary buffer
    if (tempBufferSize_ < blockSize_ * 2) {
        delete[] tempBuffer_;
        tempBuffer_ = new float[blockSize_ * 2];
        tempBufferSize_ = blockSize_ * 2;
        std::memset(tempBuffer_, 0, tempBufferSize_ * sizeof(float));
    }
    
    // Initialize core components with new sample rate
    if (chipEngine_) {
        chipEngine_->setSampleRate(sampleRate_);
    }
    
    elapsedSamples_ = 0;
    
    return AudioEffect::setupProcessing(setup);
}

// Set processing state
tresult PLUGIN_API Processor::setProcessing(TBool state) {
    if (state != processingActive_) {
        processingActive_ = state != 0;
        if (state) {
            // Processing starting
            if (chipEngine_) {
                chipEngine_->resume();
            }
        } else {
            // Processing stopping
            if (chipEngine_) {
                chipEngine_->suspend();
            }
        }
    }
    return AudioEffect::setProcessing(state);
}

// Can process sample size
tresult PLUGIN_API Processor::canProcessSampleSize(
    int32 symbolicSampleSize) {
    
    // Support 32-bit and 64-bit float
    if (symbolicSampleSize == kSample32 ||
        symbolicSampleSize == kSample64) {
        return kResultTrue;
    }
    return kResultFalse;
}

// Process
tresult PLUGIN_API Processor::process(
    ProcessData& data) {
    
    if (!processingActive_) {
        return kResultOk;
    }
    
    // Check if we need to process
    if (data.numOutputs == 0) {
        return kResultOk;
    }
    
    // Process MIDI events
    if (data.inputEvents) {
        processMidiEvents(data.inputEvents, data.numSamples);
    }
    
    // Process parameter changes
    if (data.inputParameterChanges) {
        int32 numParams = data.inputParameterChanges->getParameterCount();
        for (int32 i = 0; i < numParams; ++i) {
            IParamValueQueue* queue = 
                data.inputParameterChanges->getParameterData(i);
            if (queue) {
                processParameterChanges(queue, data.numSamples);
            }
        }
    }
    
    // Process audio based on sample type
    if (data.symbolicSampleSize == kSample32) {
        processAudio<float>(data, data.outputs[0].channelBuffers32, data.numSamples);
    } else if (data.symbolicSampleSize == kSample64) {
        processAudio<double>(data, data.outputs[0].channelBuffers64, data.numSamples);
    }
    
    elapsedSamples_ += data.numSamples;
    
    return kResultOk;
}

// Process audio (template implementation)
template<typename SampleType>
void Processor::processAudio(
    ProcessData& data,
    SampleType* const* outputs,
    int32 numSamples) {
    
    // Clear output buffers
    SampleType* outL = (outputs && data.outputs[0].numChannels > 0) ? outputs[0] : nullptr;
    SampleType* outR = (outputs && data.outputs[0].numChannels > 1) ? outputs[1] : nullptr;
    
    if (outL) {
        std::memset(outL, 0, numSamples * sizeof(SampleType));
    }
    if (outR) {
        std::memset(outR, 0, numSamples * sizeof(SampleType));
    }
    
    // Process each sample
    for (int32 i = 0; i < numSamples; ++i) {
        float left = 0.0f;
        float right = 0.0f;
        
        // Process chip engine
        if (chipEngine_) {
            chipEngine_->processSample(&left, &right);
        }
        
        // Process effect chain
        if (effectChain_) {
            effectChain_->process(&left, &right);
        }
        
        // Apply master volume from plugin state
        float masterVolume = pluginState_.getMasterVolume();
        left *= masterVolume;
        right *= masterVolume;
        
        // Convert to output type
        if (outL) {
            outL[i] = static_cast<SampleType>(left);
        }
        if (outR) {
            outR[i] = static_cast<SampleType>(right);
        }
    }
}

// Process MIDI events
void Processor::processMidiEvents(
    IEventList* events,
    int32 numSamples) {
    
    if (!midiTranslator_ || !voiceAllocator_) {
        return;
    }
    
    Event e;
    int32 eventCount = events->getEventCount();
    
    for (int32 i = 0; i < eventCount; ++i) {
        if (events->getEvent(i, e) != kResultOk) {
            continue;
        }
        
        // Note On
        if (e.type == Event::kNoteOnEvent) {
            if (e.noteOn.velocity > 0.0f) {
                voiceAllocator_->noteOn(e.noteOn.pitch, e.noteOn.velocity);
            }
        }
        
        // Note Off
        else if (e.type == Event::kNoteOffEvent) {
            voiceAllocator_->noteOff(e.noteOff.pitch);
        }
        
        // Data Event (MIDI CC, etc.)
        else if (e.type == Event::kDataEvent) {
            if (e.data.size == 3 && e.data.bytes[0] == 0xB0) {
                // MIDI CC event
                int cc = e.data.bytes[1] & 0x7F;
                int value = e.data.bytes[2] & 0x7F;
                float normalizedValue = value / 127.0f;
                voiceAllocator_->controlChange(cc, normalizedValue);
            }
        }
    }
}

// Process parameter changes
void Processor::processParameterChanges(
    IParamValueQueue* queue,
    int32 numSamples) {
    
    ParamValue value;
    int32 sampleOffset;
    int32 pointCount = queue->getPointCount();
    
    for (int32 i = 0; i < pointCount; ++i) {
        if (queue->getPoint(i, sampleOffset, value) == kResultOk) {
            // Update plugin state with new parameter value
            // The parameter ID is stored in the queue
            // We'll handle this in the controller connection
        }
    }
}

// Set state
tresult PLUGIN_API Processor::setState(
    IBStream* stream) {
    
    if (!stream) {
        return kResultFalse;
    }
    
    IBStreamAdapter adapter(stream);
    
    // Restore plugin state
    pluginState_.restoreFromStream(&adapter);
    
    // Update core components
    if (chipEngine_) {
        chipEngine_->restoreState(&adapter);
    }
    if (voiceAllocator_) {
        voiceAllocator_->restoreState(&adapter);
    }
    if (effectChain_) {
        effectChain_->restoreState(&adapter);
    }
    
    return kResultOk;
}

// Get state
tresult PLUGIN_API Processor::getState(
    IBStream* stream) {
    
    if (!stream) {
        return kResultFalse;
    }
    
    IBStreamAdapter adapter(stream);
    
    // Save plugin state
    pluginState_.saveToStream(&adapter);
    
    // Save core components
    if (chipEngine_) {
        chipEngine_->saveState(&adapter);
    }
    if (voiceAllocator_) {
        voiceAllocator_->saveState(&adapter);
    }
    if (effectChain_) {
        effectChain_->saveState(&adapter);
    }
    
    return kResultOk;
}

// Initialize core components
void Processor::initializeCore() {
    // Create voice allocator
    voiceAllocator_ = new VoiceAllocator();
    voiceAllocator_->setSampleRate(sampleRate_);
    
    // Create MIDI translator
    midiTranslator_ = new MidiTranslator();
    
    // Create chip engine
    chipEngine_ = new ChipEngine();
    chipEngine_->setSampleRate(sampleRate_);
    
    // Create effect chain
    effectChain_ = new EffectChain();
    effectChain_->setSampleRate(sampleRate_);
    
    // Connect components
    // Voice allocator -> MIDI translator -> Chip engine
    voiceAllocator_->setMidiTranslator(midiTranslator_);
    midiTranslator_->setChipEngine(chipEngine_);
}

// Cleanup core components
void Processor::cleanupCore() {
    delete effectChain_;
    effectChain_ = nullptr;
    
    delete chipEngine_;
    chipEngine_ = nullptr;
    
    delete midiTranslator_;
    midiTranslator_ = nullptr;
    
    delete voiceAllocator_;
    voiceAllocator_ = nullptr;
    
    delete[] tempBuffer_;
    tempBuffer_ = nullptr;
    tempBufferSize_ = 0;
}

} // namespace TigerFlame
