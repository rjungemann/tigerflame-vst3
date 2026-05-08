// TigerFlame VST3 Plugin - Entry Point
// This file defines the VST3 plugin factory and entry point

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "public.sdk/source/main/pluginfactory.h"

#include "ids.h"
#include "processor.h"
#include "controller.h"

// Define the plugin factory
BEGIN_FACTORY_DEF("TigerFlame", "https://github.com/rjungemann/tigerflame-vst3", "info@tigerflame.com")

// Register plugin classes
DEF_CLASS2(INLINE_UID_FROM_FUID(TigerFlameProcessorUID),
    TigerFlame::Processor,
    TigerFlame::Processor::createInstance)

DEF_CLASS2(INLINE_UID_FROM_FUID(TigerFlameControllerUID),
    TigerFlame::Controller,
    TigerFlame::Controller::createInstance)

// Plugin version
DECLARE_UID(TigerFlameVersionUID, 0xA1B2C3D4, 0xE5F67890, 0x12345678, 0x90ABCDEF0)

END_FACTORY

// Main entry point - called by VST3 host
bool InitModule() {
    return true;
}

bool DeinitModule() {
    return true;
}

// Export factory for VST3 host
// On macOS: _Z16GetPluginFactoryv (for C++ mangled name)
// On Windows: ?GetPluginFactory@@YAPEAVIAFactory@@XZ

extern "C" {
    __attribute__((visibility("default"))) 
    Steinberg::IPluginFactory* GetPluginFactory() {
        return TigerFlameFactory::instance();
    }
}
