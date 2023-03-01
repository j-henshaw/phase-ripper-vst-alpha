#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/plugin-bindings/vst3editor.h"

using namespace Steinberg;

//class PlugController : public Vst::EditController, public Vst::IMidiMapping {
class PlugController : public Vst::EditController, public VSTGUI::VST3EditorDelegate {
public:

    PlugController();

    virtual ~PlugController();

    static FUnknown* createInstance(void*);

    tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;

    tresult PLUGIN_API setComponentState(IBStream* state) SMTG_OVERRIDE;
    
    IPlugView* PLUGIN_API createView(const char* name) SMTG_OVERRIDE;

    tresult PLUGIN_API getMidiControllerAssignment(int32 bus_index, int16 channel, Vst::CtrlNumber midi_controller_number, Vst::ParamID& id);// SMTG_OVERRIDE;

    tresult PLUGIN_API notify(Vst::IMessage* message) SMTG_OVERRIDE;

private:
    
    static const Vst::ParameterInfo bypass_parameter_info;

    static const Vst::ParameterInfo pitchbend_parameter_info;
    
    static const Vst::ParameterInfo mix_knob_parameter_info;
    
    static const Vst::ParameterInfo input_gain_parameter_info;
    
    static const Vst::ParameterInfo output_gain_parameter_info;
    
    static const Vst::ParameterInfo stereo_flip_flip_parameter_info;
    
    static const Vst::ParameterInfo pitchbend_range_parameter_info;
    
    static const Vst::ParameterInfo octave_transpose_parameter_info;
    
    static const Vst::ParameterInfo semitone_transpose_parameter_info;
    
    static const Vst::ParameterInfo cent_transpose_parameter_info;
    
    static const Vst::ParameterInfo pitch_tracking_parameter_info;
    
    static const Vst::ParameterInfo pitch_latency_parameter_info;
    
    static const Vst::ParameterInfo poly_count_parameter_info;
    
    static const Vst::ParameterInfo poly_law_parameter_info;
    
    static const Vst::ParameterInfo dc_offset_filter_parameter_info;
    
    DECLARE_FUNKNOWN_METHODS

};
