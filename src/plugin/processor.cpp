// TigerFlame VST3 Plugin - Audio Processor Implementation

#include "processor.h"

#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstevents.h"

#include "core/chip_engine.h"
#include "core/voice_allocator.h"
#include "core/midi_translator.h"
#include "core/state.h"
#include "core/plugin_state.h"
#include "core/effect_chain.h"

#include <cstring>
#include <algorithm>

namespace Steinberg {
    namespace Vst {
        using namespace Steinberg;
    }
}

namespace TigerFlame {

// Constructor
Processor::Processor() : AudioEffect() {
    // Initialize with default values
    setControllerClassId(TigerFlameControllerUID);
    
    // We're an instrument (no audio input, stereo audio output)
    // Bus configuration will be set in setBusArrangements
}

// Destructor
Processor::~Processor() {
    cleanupCore();
}

// Initialize
Steinberg::tresult PLUGIN_API Processor::initialize(Steinberg::FUnknown* context) {
    tresult result = AudioEffect::initialize(context);
    if (result != kResultOk) {
        return result;
    }
    
    // Initialize core components
    initializeCore();
    
    return kResultOk;
}

// Terminate
Steinberg::tresult PLUGIN_API Processor::terminate() {
    cleanupCore();
    return AudioEffect::terminate();
}

// Set bus arrangements
Steinberg::tresult PLUGIN_API Processor::setBusArrangements(
    Steinberg::Vst::SpeakerArrangement* inputs,
    int32 numIns,
    Steinberg::Vst::SpeakerArrangement* outputs,
    int32 numOuts) {
    
    // We're an instrument plugin: 0 audio inputs, 1 stereo output
    if (numIns != 0 || numOuts != 1) {
        return kResultFalse;
    }
    
    // Check output arrangement is stereo
    if (outputs && numOuts > 0) {
        if (outputs[0] != Steinberg::Vst::kSpeakerArrStereo) {
            return kResultFalse;
        }
    }
    
    return kResultOk;
}

// Setup processing
Steinberg::tresult PLUGIN_API Processor::setupProcessing(
    Steinberg::Vst::ProcessSetup& setup) {
    
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
Steinberg::tresult PLUGIN_API Processor::setProcessing(bool state) {
    if (state != processingActive_) {
        processingActive_ = state;
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
Steinberg::tresult PLUGIN_API Processor::canProcessSampleSize(
    int32 symbolicSampleSize) {
    
    // Support 32-bit and 64-bit float
    if (symbolicSampleSize == Steinberg::Vst::kSample32 ||
        symbolicSampleSize == Steinberg::Vst::kSample64) {
        return kResultTrue;
    }
    return kResultFalse;
}

// Process
Steinberg::tresult PLUGIN_API Processor::process(
    Steinberg::Vst::ProcessData& data) {
    
    if (!processingActive_) {
        return kResultOk;
    }
    
    // Check if we need to process
    if (data.numInputs == 0 || data.numOutputs == 0) {
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
            Steinberg::Vst::IParameterValueQueue* queue = 
                data.inputParameterChanges->getParameterData(i);
            if (queue) {
                processParameterChanges(queue, data.numSamples);
            }
        }
    }
    
    // Process audio based on sample type
    if (data.symbolicSampleSize == Steinberg::Vst::kSample32) {
        processAudio<float>(data, data.inputs, data.outputs, data.numSamples);
    } else if (data.symbolicSampleSize == Steinberg::Vst::kSample64) {
        processAudio<double>(data, data.inputs, data.outputs, data.numSamples);
    }
    
    elapsedSamples_ += data.numSamples;
    
    return kResultOk;
}

// Process audio (template implementation)
template<typename SampleType>
void Processor::processAudio(
    Steinberg::Vst::ProcessData& data,
    SampleType* const* inputs,
    SampleType* const* outputs,
    int32 numSamples) {
    
    // Clear output buffers
    SampleType* outL = outputs[0];
    SampleType* outR = outputs[1];
    
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
    Steinberg::Vst::IEventList* events,
    int32 numSamples) {
    
    if (!midiTranslator_ || !voiceAllocator_) {
        return;
    }
    
    Steinberg::Vst::Event e;
    int32 eventCount = events->getEventCount();
    
    for (int32 i = 0; i < eventCount; ++i) {
        if (events->getEvent(i, e) != kResultOk) {
            continue;
        }
        
        // Only process MIDI events
        if (e.type != Steinberg::Vst::Event::kNoteOnEvent &&
            e.type != Steinberg::Vst::Event::kNoteOffEvent &&
            e.type != Steinberg::Vst::Event::kPolyPressureEvent &&
            e.type != Steinberg::Vst::Event::kNoteExpressionCountEvent &&
            e.type != Steinberg::Vst::Event::kNoteExpressionValueEvent &&
            e.type != Steinberg::Vst::Event::kPitchBendEvent &&
            e.type != Steinberg::Vst::Event::kDataEvent) {
            continue;
        }
        
        // Note On
        if (e.type == Steinberg::Vst::Event::kNoteOnEvent) {
            Steinberg::Vst::NoteOnEvent* noteOn = static_cast<Steinberg::Vst::NoteOnEvent*>(e.data);
            if (noteOn && noteOn->velocity > 0.0f) {
                int note = noteOn->pitch;
                float velocity = noteOn->velocity;
                voiceAllocator_->noteOn(note, velocity);
            }
        }
        
        // Note Off
        else if (e.type == Steinberg::Vst::Event::kNoteOffEvent) {
            Steinberg::Vst::NoteOffEvent* noteOff = static_cast<Steinberg::Vst::NoteOffEvent*>(e.data);
            if (noteOff) {
                int note = noteOff->pitch;
                float velocity = noteOff->velocity;
                voiceAllocator_->noteOff(note);
            }
        }
        
        // Pitch Bend
        else if (e.type == Steinberg::Vst::Event::kPitchBendEvent) {
            Steinberg::Vst::PitchBendEvent* pitchBend = static_cast<Steinberg::Vst::PitchBendEvent*>(e.data);
            if (pitchBend) {
                // Convert pitch bend value to semitones
                // VST3 pitch bend: -1.0 to +1.0 = -2 to +2 semitones typically
                double semitones = pitchBend->pitch * pluginState_.getPitchBendRange();
                voiceAllocator_->pitchBend(semitones);
            }
        }
        
        // Data Event (MIDI CC, etc.)
        else if (e.type == Steinberg::Vst::Event::kDataEvent) {
            Steinberg::Vst::DataEvent* dataEvent = static_cast<Steinberg::Vst::DataEvent*>(e.data);
            if (dataEvent && dataEvent->size == 3 && dataEvent->data[0] == 0xB0) {
                // MIDI CC event
                int cc = dataEvent->data[1] & 0x7F;
                int value = dataEvent->data[2] & 0x7F;
                float normalizedValue = value / 127.0f;
                voiceAllocator_->controlChange(cc, normalizedValue);
            }
        }
    }
}

// Process parameter changes
void Processor::processParameterChanges(
    Steinberg::Vst::IParameterValueQueue* queue,
    int32 numSamples) {
    
    Steinberg::Vst::ParamValue value;
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

// Get info
Steinberg::tresult PLUGIN_API Processor::getInfo(
    Steinberg::PClassInfo& info) const {
    info.cardinality = Steinberg::PClassInfo::kManyInstances;
    info.className = "TigerFlame Processor";
    info.classID = TigerFlameProcessorUID;
    return kResultOk;
}

// Get controller class ID
Steinberg::tresult PLUGIN_API Processor::getControllerClassId(
    Steinberg::TUID& classId) const {
    classId = TigerFlameControllerUID;
    return kResultOk;
}

// Set state
Steinberg::tresult PLUGIN_API Processor::setState(
    Steinberg::IBStream* stream) {
    
    if (!stream) {
        return kResultFalse;
    }
    
    // Save plugin state
    pluginState_.restoreFromStream(stream);
    
    // Update core components
    if (chipEngine_) {
        chipEngine_->restoreState(stream);
    }
    if (voiceAllocator_) {
        voiceAllocator_->restoreState(stream);
    }
    if (effectChain_) {
        effectChain_->restoreState(stream);
    }
    
    return kResultOk;
}

// Get state
Steinberg::tresult PLUGIN_API Processor::getState(
    Steinberg::IBStream* stream) const {
    
    if (!stream) {
        return kResultFalse;
    }
    
    // Save plugin state
    pluginState_.saveToStream(stream);
    
    // Save core components
    if (chipEngine_) {
        chipEngine_->saveState(stream);
    }
    if (voiceAllocator_) {
        voiceAllocator_->saveState(stream);
    }
    if (effectChain_) {
        effectChain_->saveState(stream);
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
