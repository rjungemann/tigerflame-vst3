// TigerFlame VST3 Plugin - Edit Controller Implementation

#include "controller.h"

#include "pluginterfaces/vst/ivstparameters.h"
#include "pluginterfaces/vst/ivstunits.h"

#include "base/source/fstreamer.h"

#include <cstring>

namespace Steinberg {
    namespace Vst {
        using namespace Steinberg;
    }
}

namespace TigerFlame {

// Parameter descriptors
const Controller::ParameterDescriptor Controller::kParameterDescriptors[] = {
    // Voice Mode
    {ParamId::kVoiceMode, "Voice Mode", "Mode", "", 0.0, 3, ParameterInfo::kIsList},
    
    // Master Volume
    {ParamId::kMasterVolume, "Master Volume", "Volume", "dB", 0.8, 0, ParameterInfo::kIsAutomatable},
    
    // Master Tuning
    {ParamId::kMasterTuning, "Master Tuning", "Tuning", "cents", 0.5, 0, ParameterInfo::kIsAutomatable},
    
    // Master Pan
    {ParamId::kMasterPan, "Master Pan", "Pan", "", 0.5, 0, ParameterInfo::kIsAutomatable},
    
    // Chip Select
    {ParamId::kChipSelect, "Chip Type", "Chip", "", 0.0, 20, ParameterInfo::kIsList | ParameterInfo::kIsAutomatable},
    
    // Chip Count
    {ParamId::kChipCount, "Chip Count", "Chips", "", 0.0, 8, ParameterInfo::kIsAutomatable},
    
    // Note Transpose
    {ParamId::kNoteTranspose, "Transpose", "Transpose", "semitones", 0.5, 0, ParameterInfo::kIsAutomatable},
    
    // Pitch Bend Range
    {ParamId::kPitchBendRange, "Pitch Bend Range", "Bend Range", "semitones", 0.333, 0, ParameterInfo::kIsAutomatable},
    
    // Velocity Sensitivity
    {ParamId::kVelocitySensitivity, "Velocity Sensitivity", "Velocity", "%", 1.0, 0, ParameterInfo::kIsAutomatable},
    
    // Polyphony Count
    {ParamId::kPolyphonyCount, "Polyphony", "Poly", "voices", 0.25, 32, ParameterInfo::kIsAutomatable},
    
    // Layer Count
    {ParamId::kLayerCount, "Layers", "Layers", "", 0.0, 8, ParameterInfo::kIsAutomatable},
    
    // Effect Chain Bypass
    {ParamId::kEffectChainBypass, "Effects Bypass", "FX Bypass", "", 0.0, 1, ParameterInfo::kIsBypass},
    
    // Effect 1
    {ParamId::kEffect1Type, "Effect 1 Type", "FX1 Type", "", 0.0, 5, ParameterInfo::kIsList},
    {ParamId::kEffect1Bypass, "Effect 1 Bypass", "FX1 Bypass", "", 0.0, 1, ParameterInfo::kIsBypass},
    
    // Effect 2
    {ParamId::kEffect2Type, "Effect 2 Type", "FX2 Type", "", 0.0, 5, ParameterInfo::kIsList},
    {ParamId::kEffect2Bypass, "Effect 2 Bypass", "FX2 Bypass", "", 0.0, 1, ParameterInfo::kIsBypass},
    
    // Effect 3
    {ParamId::kEffect3Type, "Effect 3 Type", "FX3 Type", "", 0.0, 5, ParameterInfo::kIsList},
    {ParamId::kEffect3Bypass, "Effect 3 Bypass", "FX3 Bypass", "", 0.0, 1, ParameterInfo::kIsBypass},
    
    // Effect 4
    {ParamId::kEffect4Type, "Effect 4 Type", "FX4 Type", "", 0.0, 5, ParameterInfo::kIsList},
    {ParamId::kEffect4Bypass, "Effect 4 Bypass", "FX4 Bypass", "", 0.0, 1, ParameterInfo::kIsBypass},
};

// Constructor
Controller::Controller() : EditControllerEx1() {
    parameters_ = nullptr;
}

// Destructor
Controller::~Controller() {
    if (parameters_) {
        for (int32 i = 0; i < kParamCount; ++i) {
            delete parameters_[i];
        }
        delete[] parameters_;
        parameters_ = nullptr;
    }
}

// Initialize
Steinberg::tresult PLUGIN_API Controller::initialize(
    Steinberg::FUnknown* context) {
    
    tresult result = EditControllerEx1::initialize(context);
    if (result != kResultOk) {
        return result;
    }
    
    // Initialize parameters
    initializeParameters();
    
    return kResultOk;
}

// Terminate
Steinberg::tresult PLUGIN_API Controller::terminate() {
    return EditControllerEx1::terminate();
}

// Initialize parameters
void Controller::initializeParameters() {
    if (parameters_) {
        return; // Already initialized
    }
    
    parameters_ = new Steinberg::Vst::Parameter*[kParamCount];
    
    for (int32 i = 0; i < kParamCount; ++i) {
        const ParameterDescriptor& desc = kParameterDescriptors[i];
        
        // Create parameter with appropriate flags
        Steinberg::Vst::Parameter* param = new Steinberg::Vst::Parameter(
            USTRING(desc.title),
            static_cast<Steinberg::Vst::ParamID>(desc.id),
            desc.defaultNormalizedValue,
            desc.flags
        );
        
        // Set units
        if (desc.units && desc.units[0] != '\0') {
            param->setUnitID(Steinberg::Vst::UnitID(desc.units));
        }
        
        // Set step count
        if (desc.stepCount > 0) {
            param->setStepCount(desc.stepCount);
        }
        
        parameters_[i] = param;
        addParameter(param);
    }
}

// Get parameter object
Steinberg::Vst::Parameter* Controller::getParameterObject(int32 index) {
    if (index >= 0 && index < kParamCount) {
        return parameters_[index];
    }
    return nullptr;
}

// Get parameter ID from index
ParamId Controller::getParameterId(int32 index) const {
    if (index >= 0 && index < kParamCount) {
        return kParameterDescriptors[index].id;
    }
    return ParamId::kParamCount;
}

// Get parameter index from ID
int32 Controller::getParameterIndex(ParamId id) const {
    for (int32 i = 0; i < kParamCount; ++i) {
        if (kParameterDescriptors[i].id == id) {
            return i;
        }
    }
    return -1;
}

// Set parameter normalized
Steinberg::tresult PLUGIN_API Controller::setParamNormalized(
    Steinberg::Vst::ParamID tag,
    Steinberg::Vst::ParamValue value) {
    
    // Update the parameter
    tresult result = EditControllerEx1::setParamNormalized(tag, value);
    
    // Notify the processor of parameter changes
    if (result == kResultOk) {
        // The parameter change will be picked up by the processor
        // through the parameter queue in the process call
    }
    
    return result;
}

// Get parameter string by value
Steinberg::tresult PLUGIN_API Controller::getParamStringByValue(
    Steinberg::Vst::ParamID tag,
    Steinberg::Vst::ParamValue valueNormalized,
    Steinberg::Vst::String128 string) {
    
    // Find parameter descriptor
    for (const auto& desc : kParameterDescriptors) {
        if (static_cast<Steinberg::Vst::ParamID>(desc.id) == tag) {
            switch (desc.id) {
                case ParamId::kVoiceMode: {
                    int mode = static_cast<int>(valueNormalized * 3);
                    switch (mode) {
                        case 0: std::strcpy(string, "Mono"); break;
                        case 1: std::strcpy(string, "Layered"); break;
                        case 2: std::strcpy(string, "Polyphonic"); break;
                        default: std::strcpy(string, "Unknown"); break;
                    }
                    return kResultOk;
                }
                
                case ParamId::kChipSelect: {
                    // Chip type names
                    const char* chipNames[] = {
                        "YM2151", "YM2612", "SN76489", "OPL2", "OPL3",
                        "QSound", "C140", "POKEY", "VRC6"
                    };
                    int index = static_cast<int>(valueNormalized * 8);
                    index = std::max(0, std::min(index, 7));
                    std::strcpy(string, chipNames[index]);
                    return kResultOk;
                }
                
                case ParamId::kEffect1Type:
                case ParamId::kEffect2Type:
                case ParamId::kEffect3Type:
                case ParamId::kEffect4Type: {
                    const char* effectNames[] = {
                        "None", "QSound", "Reverb", "Delay", "Distortion", "EQ"
                    };
                    int index = static_cast<int>(valueNormalized * 5);
                    index = std::max(0, std::min(index, 5));
                    std::strcpy(string, effectNames[index]);
                    return kResultOk;
                }
                
                case ParamId::kMasterVolume: {
                    float db = valueNormalized * 20.0f - 20.0f; // -20dB to 0dB
                    sprintf(string, "%.1f dB", db);
                    return kResultOk;
                }
                
                case ParamId::kMasterTuning: {
                    float cents = (valueNormalized - 0.5f) * 200.0f; // -100 to +100 cents
                    sprintf(string, "%.0f cents", cents);
                    return kResultOk;
                }
                
                case ParamId::kMasterPan: {
                    float pan = (valueNormalized - 0.5f) * 200.0f; // -100 to +100%
                    sprintf(string, "%.0f%%", pan);
                    return kResultOk;
                }
                
                default:
                    // For continuous parameters, show percentage
                    sprintf(string, "%.0f%%", valueNormalized * 100.0f);
                    return kResultOk;
            }
        }
    }
    
    return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string);
}

// Get parameter value by string
Steinberg::tresult PLUGIN_API Controller::getParamValueByString(
    Steinberg::Vst::ParamID tag,
    Steinberg::Vst::TChar* string,
    Steinberg::Vst::ParamValue& valueNormalized) {
    
    // Find parameter descriptor
    for (const auto& desc : kParameterDescriptors) {
        if (static_cast<Steinberg::Vst::ParamID>(desc.id) == tag) {
            switch (desc.id) {
                case ParamId::kVoiceMode: {
                    if (std::strcmp(string, "Mono") == 0) valueNormalized = 0.0;
                    else if (std::strcmp(string, "Layered") == 0) valueNormalized = 0.333f;
                    else if (std::strcmp(string, "Polyphonic") == 0) valueNormalized = 0.666f;
                    else return kResultFalse;
                    return kResultOk;
                }
                
                default:
                    // Try to parse as number
                    try {
                        valueNormalized = static_cast<float>(std::atof(string)) / 100.0f;
                        return kResultOk;
                    } catch (...) {
                        return kResultFalse;
                    }
            }
        }
    }
    
    return EditControllerEx1::getParamValueByString(tag, string, valueNormalized);
}

// Get parameter info
Steinberg::tresult PLUGIN_API Controller::getParamInfo(
    int32 index,
    Steinberg::Vst::ParameterInfo& info) {
    
    if (index >= 0 && index < kParamCount) {
        const ParameterDescriptor& desc = kParameterDescriptors[index];
        
        info.id = nullptr; // Will be set by the parameter
        info.title = USTRING(desc.title);
        info.shortTitle = USTRING(desc.shortTitle);
        info.units = USTRING(desc.units);
        info.stepCount = desc.stepCount;
        info.defaultNormalizedValue = desc.defaultNormalizedValue;
        info.unitID = desc.units ? Steinberg::Vst::UnitID(desc.units) : 0;
        info.flags = desc.flags;
        
        return kResultOk;
    }
    
    return kResultFalse;
}

// Get unit info
Steinberg::tresult PLUGIN_API Controller::getUnitInfo(
    int32 unitIndex,
    Steinberg::Vst::UnitInfo& info) {
    
    // Define units
    switch (unitIndex) {
        case Steinberg::Vst::kRootUnitId:
            info.id = kRootUnitId;
            info.parentUnitId = -1;
            info.name = USTRING("Global");
            info.programListId = -1;
            return kResultOk;
            
        case Steinberg::Vst::kModulationUnitId:
            info.id = kModulationUnitId;
            info.parentUnitId = kRootUnitId;
            info.name = USTRING("Modulation");
            info.programListId = -1;
            return kResultOk;
            
        default:
            return kResultFalse;
    }
}

// Get MIDI controller assignment
Steinberg::tresult PLUGIN_API Controller::getMidiControllerAssignment(
    int32 busIndex,
    int16 channel,
    Steinberg::Vst::CtrlNumber midiControllerNumber,
    Steinberg::Vst::ParamID& id) {
    
    // Map standard MIDI CCs to parameters
    switch (midiControllerNumber) {
        case Steinberg::Vst::kCtrlModulationWheel:
            id = ParamId::kVoiceMode; // Or map to modulation
            return kResultOk;
            
        case Steinberg::Vst::kCtrlVolume:
            id = ParamId::kMasterVolume;
            return kResultOk;
            
        case Steinberg::Vst::kCtrlPan:
            id = ParamId::kMasterPan;
            return kResultOk;
            
        case Steinberg::Vst::kCtrlExpression:
            // Could map to velocity sensitivity
            id = ParamId::kVelocitySensitivity;
            return kResultOk;
            
        case Steinberg::Vst::kCtrlSustain:
            // Sustain pedal - handled by voice allocator
            return kResultFalse;
            
        default:
            return kResultFalse;
    }
}

// Program management
int32 PLUGIN_API Controller::getProgramIndex() {
    return 0; // Single program for now
}

void PLUGIN_API Controller::setProgramIndex(int32 index) {
    // No-op for now
}

Steinberg::tresult PLUGIN_API Controller::getProgramNameIndexed(
    int32 index,
    Steinberg::Vst::String128 name) {
    
    if (index == 0) {
        std::strcpy(name, "Default");
        return kResultOk;
    }
    return kResultFalse;
}

// Create view
Steinberg::IPlugView* PLUGIN_API Controller::createView(
    Steinberg::FIDString name) {
    
    // For now, return nullptr until UI is implemented
    // This will be implemented in Phase 5
    return nullptr;
}

// Set state
Steinberg::tresult PLUGIN_API Controller::setState(
    Steinberg::IBStream* stream) {
    
    if (!stream) {
        return kResultFalse;
    }
    
    // Read state for each parameter
    for (int32 i = 0; i < kParamCount; ++i) {
        float value;
        if (stream->read(&value, sizeof(value)) == kResultOk) {
            setParamNormalized(kParameterDescriptors[i].id, value);
        }
    }
    
    return kResultOk;
}

// Get state
Steinberg::tresult PLUGIN_API Controller::getState(
    Steinberg::IBStream* stream) const {
    
    if (!stream) {
        return kResultFalse;
    }
    
    // Write state for each parameter
    for (int32 i = 0; i < kParamCount; ++i) {
        float value = getParamNormalized(kParameterDescriptors[i].id);
        if (stream->write(&value, sizeof(value)) != kResultOk) {
            return kResultFalse;
        }
    }
    
    return kResultOk;
}

} // namespace TigerFlame
