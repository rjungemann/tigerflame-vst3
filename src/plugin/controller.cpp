// TigerFlame VST3 Plugin - Edit Controller Implementation

#include "controller.h"

#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstunits.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "pluginterfaces/base/fstrdefs.h"
#include "pluginterfaces/base/ibstream.h"

#include <cstring>
#include <cstdio>
#include <algorithm>

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace TigerFlame {

// Parameter descriptors
const Controller::PluginParamDescriptor Controller::kParameterDescriptors[] = {
    {ParamId::kVoiceMode,           "Voice Mode",           "Mode",       "",          0.0,   3,  ParameterInfo::kIsList},
    {ParamId::kMasterVolume,        "Master Volume",        "Volume",     "dB",        0.8,   0,  ParameterInfo::kCanAutomate},
    {ParamId::kMasterTuning,        "Master Tuning",        "Tuning",     "cents",     0.5,   0,  ParameterInfo::kCanAutomate},
    {ParamId::kMasterPan,           "Master Pan",           "Pan",        "",          0.5,   0,  ParameterInfo::kCanAutomate},
    {ParamId::kChipSelect,          "Chip Type",            "Chip",       "",          0.0,   20, ParameterInfo::kIsList | ParameterInfo::kCanAutomate},
    {ParamId::kChipCount,           "Chip Count",           "Chips",      "",          0.0,   8,  ParameterInfo::kCanAutomate},
    {ParamId::kNoteTranspose,       "Transpose",            "Transpose",  "semitones", 0.5,   0,  ParameterInfo::kCanAutomate},
    {ParamId::kPitchBendRange,      "Pitch Bend Range",     "Bend Range", "semitones", 0.333, 0,  ParameterInfo::kCanAutomate},
    {ParamId::kVelocitySensitivity, "Velocity Sensitivity", "Velocity",   "%",         1.0,   0,  ParameterInfo::kCanAutomate},
    {ParamId::kPolyphonyCount,      "Polyphony",            "Poly",       "voices",    0.25,  32, ParameterInfo::kCanAutomate},
    {ParamId::kLayerCount,          "Layers",               "Layers",     "",          0.0,   8,  ParameterInfo::kCanAutomate},
    {ParamId::kEffectChainBypass,   "Effects Bypass",       "FX Bypass",  "",          0.0,   1,  ParameterInfo::kIsBypass},
    {ParamId::kEffect1Type,         "Effect 1 Type",        "FX1 Type",   "",          0.0,   5,  ParameterInfo::kIsList},
    {ParamId::kEffect1Bypass,       "Effect 1 Bypass",      "FX1 Bypass", "",          0.0,   1,  ParameterInfo::kIsBypass},
    {ParamId::kEffect2Type,         "Effect 2 Type",        "FX2 Type",   "",          0.0,   5,  ParameterInfo::kIsList},
    {ParamId::kEffect2Bypass,       "Effect 2 Bypass",      "FX2 Bypass", "",          0.0,   1,  ParameterInfo::kIsBypass},
    {ParamId::kEffect3Type,         "Effect 3 Type",        "FX3 Type",   "",          0.0,   5,  ParameterInfo::kIsList},
    {ParamId::kEffect3Bypass,       "Effect 3 Bypass",      "FX3 Bypass", "",          0.0,   1,  ParameterInfo::kIsBypass},
    {ParamId::kEffect4Type,         "Effect 4 Type",        "FX4 Type",   "",          0.0,   5,  ParameterInfo::kIsList},
    {ParamId::kEffect4Bypass,       "Effect 4 Bypass",      "FX4 Bypass", "",          0.0,   1,  ParameterInfo::kIsBypass},
};

// Constructor
Controller::Controller() : EditControllerEx1() {
}

// Destructor
Controller::~Controller() {
}

// Initialize
tresult PLUGIN_API Controller::initialize(FUnknown* context) {
    tresult result = EditControllerEx1::initialize(context);
    if (result != kResultOk) {
        return result;
    }
    initializeParameters();
    return kResultOk;
}

// Terminate
tresult PLUGIN_API Controller::terminate() {
    return EditControllerEx1::terminate();
}

// Initialize parameters
void Controller::initializeParameters() {
    for (int32 i = 0; i < kParamCount; ++i) {
        const PluginParamDescriptor& desc = kParameterDescriptors[i];

        TChar wTitle[128]      = {};
        TChar wShortTitle[128] = {};
        TChar wUnits[128]      = {};
        str8ToStr16(wTitle,      desc.title,      128);
        str8ToStr16(wShortTitle, desc.shortTitle,  128);
        if (desc.units && desc.units[0]) {
            str8ToStr16(wUnits, desc.units, 128);
        }

        // Use ParameterContainer::addParameter(title, units, stepCount, default, flags, tag)
        parameters.addParameter(
            wTitle,
            wUnits,
            desc.stepCount,
            desc.defaultNormalizedValue,
            desc.flags,
            static_cast<ParamID>(desc.id)
        );
    }
}

// Set parameter normalized
tresult PLUGIN_API Controller::setParamNormalized(ParamID tag, ParamValue value) {
    return EditControllerEx1::setParamNormalized(tag, value);
}

// Get parameter string by value
tresult PLUGIN_API Controller::getParamStringByValue(
    ParamID tag,
    ParamValue valueNormalized,
    String128 string) {

    for (int32 i = 0; i < kParamCount; ++i) {
        const PluginParamDescriptor& desc = kParameterDescriptors[i];
        if (static_cast<ParamID>(desc.id) != tag) {
            continue;
        }

        char buf[64] = {};
        switch (desc.id) {
            case ParamId::kVoiceMode: {
                const char* names[] = {"Mono", "Layered", "Polyphonic"};
                int idx = std::max(0, std::min(static_cast<int>(valueNormalized * 3), 2));
                str8ToStr16(string, names[idx], 128);
                return kResultOk;
            }
            case ParamId::kChipSelect: {
                const char* chips[] = {
                    "YM2151","YM2612","SN76489","OPL2","OPL3","QSound","C140","POKEY","VRC6"
                };
                int idx = std::max(0, std::min(static_cast<int>(valueNormalized * 8), 8));
                str8ToStr16(string, chips[idx], 128);
                return kResultOk;
            }
            case ParamId::kEffect1Type:
            case ParamId::kEffect2Type:
            case ParamId::kEffect3Type:
            case ParamId::kEffect4Type: {
                const char* effects[] = {"None","QSound","Reverb","Delay","Distortion","EQ"};
                int idx = std::max(0, std::min(static_cast<int>(valueNormalized * 5), 5));
                str8ToStr16(string, effects[idx], 128);
                return kResultOk;
            }
            case ParamId::kMasterVolume: {
                snprintf(buf, sizeof(buf), "%.1f dB",
                         static_cast<float>(valueNormalized) * 20.0f - 20.0f);
                str8ToStr16(string, buf, 128);
                return kResultOk;
            }
            case ParamId::kMasterTuning: {
                snprintf(buf, sizeof(buf), "%.0f cents",
                         (static_cast<float>(valueNormalized) - 0.5f) * 200.0f);
                str8ToStr16(string, buf, 128);
                return kResultOk;
            }
            case ParamId::kMasterPan: {
                snprintf(buf, sizeof(buf), "%.0f%%",
                         (static_cast<float>(valueNormalized) - 0.5f) * 200.0f);
                str8ToStr16(string, buf, 128);
                return kResultOk;
            }
            default: {
                snprintf(buf, sizeof(buf), "%.0f%%", valueNormalized * 100.0);
                str8ToStr16(string, buf, 128);
                return kResultOk;
            }
        }
    }

    return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string);
}

// Get parameter value by string
tresult PLUGIN_API Controller::getParamValueByString(
    ParamID tag,
    TChar* string,
    ParamValue& valueNormalized) {

    return EditControllerEx1::getParamValueByString(tag, string, valueNormalized);
}

// Get unit info
tresult PLUGIN_API Controller::getUnitInfo(int32 unitIndex, UnitInfo& info) {
    if (unitIndex == 0) {
        info.id = kRootUnitId;
        info.parentUnitId = kNoParentUnitId;
        str8ToStr16(info.name, "Global", 128);
        info.programListId = kNoProgramListId;
        return kResultOk;
    }
    return kResultFalse;
}

// Get MIDI controller assignment
tresult PLUGIN_API Controller::getMidiControllerAssignment(
    int32 busIndex, int16 channel,
    CtrlNumber midiControllerNumber, ParamID& id) {

    switch (midiControllerNumber) {
        case kCtrlVolume:
            id = static_cast<ParamID>(ParamId::kMasterVolume);
            return kResultOk;
        case kCtrlPan:
            id = static_cast<ParamID>(ParamId::kMasterPan);
            return kResultOk;
        case kCtrlExpression:
            id = static_cast<ParamID>(ParamId::kVelocitySensitivity);
            return kResultOk;
        default:
            return kResultFalse;
    }
}

// Create view
IPlugView* PLUGIN_API Controller::createView(FIDString /*name*/) {
    return nullptr;
}

// Set state
tresult PLUGIN_API Controller::setState(IBStream* stream) {
    if (!stream) {
        return kResultFalse;
    }
    for (int32 i = 0; i < kParamCount; ++i) {
        float value = 0.f;
        int32 bytesRead = 0;
        if (stream->read(&value, sizeof(value), &bytesRead) == kResultOk &&
            bytesRead == static_cast<int32>(sizeof(value))) {
            setParamNormalized(static_cast<ParamID>(kParameterDescriptors[i].id), value);
        }
    }
    return kResultOk;
}

// Get state
tresult PLUGIN_API Controller::getState(IBStream* stream) {
    if (!stream) {
        return kResultFalse;
    }
    for (int32 i = 0; i < kParamCount; ++i) {
        float value = static_cast<float>(
            getParamNormalized(static_cast<ParamID>(kParameterDescriptors[i].id)));
        int32 bytesWritten = 0;
        if (stream->write(&value, sizeof(value), &bytesWritten) != kResultOk) {
            return kResultFalse;
        }
    }
    return kResultOk;
}

} // namespace TigerFlame
