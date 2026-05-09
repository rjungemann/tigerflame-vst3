// TigerFlame VST3 Plugin - Entry Point
// This file defines the VST3 plugin factory and entry point

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "public.sdk/source/main/pluginfactory.h"

#include "ids.h"
#include "processor.h"
#include "controller.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

// Define the plugin factory
BEGIN_FACTORY_DEF("TigerFlame", "https://github.com/rjungemann/tigerflame-vst3", "info@tigerflame.com")

// Register processor (AudioEffect)
DEF_CLASS2(INLINE_UID(0xA1B2C3D4, 0xE5F67890, 0x12345678, 0x90ABCDEF),
    PClassInfo::kManyInstances,
    kVstAudioEffectClass,
    "TigerFlame",
    kDistributable,
    "Instrument|Synth",
    "1.0.0",
    kVstVersionString,
    TigerFlame::Processor::createInstance)

// Register controller (EditController)
DEF_CLASS2(INLINE_UID(0xFEDCBA09, 0x87654321, 0xFEDCBA98, 0x76543210),
    PClassInfo::kManyInstances,
    kVstComponentControllerClass,
    "TigerFlame Controller",
    0,
    "",
    "1.0.0",
    kVstVersionString,
    TigerFlame::Controller::createInstance)

END_FACTORY

// Main entry point - called by VST3 host
bool InitModule() {
    return true;
}

bool DeinitModule() {
    return true;
}
