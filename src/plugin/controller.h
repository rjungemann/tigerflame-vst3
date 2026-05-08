// TigerFlame VST3 Plugin - Edit Controller
// This class handles parameter management and UI interaction

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "public.sdk/source/vst/vstparameters.h"

#include "ids.h"
#include "version.h"
#include "param_ids.h"

namespace Steinberg {
    namespace Vst {
        class EditController;
        class Parameter;
    }
}

namespace TigerFlame {

// Parameter descriptor for parameter registration
struct ParameterDescriptor {
    ParamId id;
    const char* title;
    const char* shortTitle;
    const char* units;
    double defaultNormalizedValue;
    Steinberg::int32 stepCount;
    Steinberg::int32 flags;
};

// Edit Controller class
class Controller : public Steinberg::Vst::EditControllerEx1 {
public:
    // Constructor
    Controller();
    
    // Destructor
    ~Controller() override;
    
    // IEditController interface
    Steinberg::tresult PLUGIN_API initialize(
        Steinberg::FUnknown* context) override;
    Steinberg::tresult PLUGIN_API terminate() override;
    
    // Parameter management
    Steinberg::tresult PLUGIN_API setParamNormalized(
        Steinberg::Vst::ParamID tag,
        Steinberg::Vst::ParamValue value) override;
    Steinberg::tresult PLUGIN_API getParamStringByValue(
        Steinberg::Vst::ParamID tag,
        Steinberg::Vst::ParamValue valueNormalized,
        Steinberg::Vst::String128 string) override;
    Steinberg::tresult PLUGIN_API getParamValueByString(
        Steinberg::Vst::ParamID tag,
        Steinberg::Vst::TChar* string,
        Steinberg::Vst::ParamValue& valueNormalized) override;
    
    // Parameter info
    Steinberg::tresult PLUGIN_API getParamInfo(
        Steinberg::int32 index,
        Steinberg::Vst::ParameterInfo& info) override;
    
    // Unit info
    Steinberg::tresult PLUGIN_API getUnitInfo(
        Steinberg::int32 unitIndex,
        Steinberg::Vst::UnitInfo& info) override;
    
    // MIDI controller assignment
    Steinberg::tresult PLUGIN_API getMidiControllerAssignment(
        Steinberg::int32 busIndex,
        Steinberg::int16 channel,
        Steinberg::Vst::CtrlNumber midiControllerNumber,
        Steinberg::Vst::ParamID& id) override;
    
    // Program management
    Steinberg::int32 PLUGIN_API getProgramIndex() override;
    void PLUGIN_API setProgramIndex(Steinberg::int32 index) override;
    Steinberg::tresult PLUGIN_API getProgramNameIndexed(
        Steinberg::int32 index,
        Steinberg::Vst::String128 name) override;
    
    // View creation
    Steinberg::IPlugView* PLUGIN_API createView(
        Steinberg::FIDString name) override;
    
    // State persistence
    Steinberg::tresult PLUGIN_API setState(
        Steinberg::IBStream* stream) override;
    Steinberg::tresult PLUGIN_API getState(
        Steinberg::IBStream* stream) const override;
    
    // Static factory method
    static Steinberg::FUnknown* createInstance(void* /*context*/) {
        return static_cast<Steinberg::Vst::IEditController*>(new Controller());
    }
    
    // Get parameter count
    Steinberg::int32 getParameterCount() const override { return kParamCount; }
    
    // Get parameter object
    Steinberg::Vst::Parameter* getParameterObject(Steinberg::int32 index) override;
    
    // Get parameter ID from index
    ParamId getParameterId(Steinberg::int32 index) const;
    
    // Get parameter index from ID
    Steinberg::int32 getParameterIndex(ParamId id) const;

private:
    // Parameter descriptors
    static const ParameterDescriptor kParameterDescriptors[];
    
    // Initialize parameters
    void initializeParameters();
    
    // Parameter storage
    Steinberg::Vst::Parameter** parameters_ = nullptr;
    
    // Parameter count
    static constexpr Steinberg::int32 kParamCount = static_cast<Steinberg::int32>(ParamId::kParamCount);
    
    // Disallow copying
    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;
};

} // namespace TigerFlame
