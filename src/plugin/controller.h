// TigerFlame VST3 Plugin - Edit Controller
// This class handles parameter management and UI interaction

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"

#include "ids.h"
#include "version.h"
#include "core/parameter_model.h"

namespace Steinberg {
    namespace Vst {
        class EditController;
        class Parameter;
    }
}

namespace TigerFlame {

// Edit Controller class
class Controller : public Steinberg::Vst::EditControllerEx1,
                   public Steinberg::Vst::IMidiMapping {
public:
    // Parameter descriptor for internal use (separate from core ParameterDescriptor)
    struct PluginParamDescriptor {
        ParamId id;
        const char* title;
        const char* shortTitle;
        const char* units;
        double defaultNormalizedValue;
        Steinberg::int32 stepCount;
        Steinberg::int32 flags;
    };

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
    
    // Unit info
    Steinberg::tresult PLUGIN_API getUnitInfo(
        Steinberg::int32 unitIndex,
        Steinberg::Vst::UnitInfo& info) override;
    
    // MIDI controller assignment (IMidiMapping)
    Steinberg::tresult PLUGIN_API getMidiControllerAssignment(
        Steinberg::int32 busIndex,
        Steinberg::int16 channel,
        Steinberg::Vst::CtrlNumber midiControllerNumber,
        Steinberg::Vst::ParamID& id) override;
    
    // View creation
    Steinberg::IPlugView* PLUGIN_API createView(
        Steinberg::FIDString name) override;
    
    // State persistence
    Steinberg::tresult PLUGIN_API setState(
        Steinberg::IBStream* stream) override;
    Steinberg::tresult PLUGIN_API getState(
        Steinberg::IBStream* stream) override;
    
    // Static factory method
    static Steinberg::FUnknown* createInstance(void* /*context*/) {
        return static_cast<Steinberg::Vst::IEditController*>(new Controller());
    }

    // Expose IMidiMapping through queryInterface
    OBJ_METHODS(Controller, EditControllerEx1)
    DEFINE_INTERFACES
        DEF_INTERFACE(IMidiMapping)
    END_DEFINE_INTERFACES(EditControllerEx1)
    REFCOUNT_METHODS(EditControllerEx1)

private:
    // Parameter descriptors
    static const PluginParamDescriptor kParameterDescriptors[];
    static constexpr Steinberg::int32 kParamCount =
        static_cast<Steinberg::int32>(ParamId::kParamCount);
    
    // Initialize parameters
    void initializeParameters();
    
    // Disallow copying
    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;
};

} // namespace TigerFlame
