#include "../include/plugcontroller.hpp"
#include "../include/plugids.hpp"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "base/source/fstreamer.h"

using namespace Steinberg;

PlugController::PlugController() {
    FUNKNOWN_CTOR
}

PlugController::~PlugController() {
    FUNKNOWN_DTOR
}

IMPLEMENT_REFCOUNT(PlugController)

tresult PlugController::queryInterface(const char* iid, void** obj) {
    tresult result = EditController::queryInterface(iid, obj);
    /*
    if (result == kNoInterface) {
        QUERY_INTERFACE(iid, obj, Steinberg::Vst::IMidiMapping::iid, IMidiMapping)
    }*/

    return result;
}

FUnknown* PlugController::createInstance(void*) {
    return static_cast<Vst::IEditController*>(new PlugController());
}

tresult PLUGIN_API PlugController::initialize(FUnknown* context) {
    tresult result = EditController::initialize(context);
    if (result != kResultTrue) {
        return kResultFalse;
    }
        
    parameters.addParameter(bypass_parameter_info);
    parameters.addParameter(pitchbend_parameter_info);
    parameters.addParameter(mix_knob_parameter_info);
    parameters.addParameter(output_gain_parameter_info);
    parameters.addParameter(stereo_flip_flip_parameter_info);
    parameters.addParameter(pitchbend_range_parameter_info);
    parameters.addParameter(octave_transpose_parameter_info);
    parameters.addParameter(semitone_transpose_parameter_info);
    parameters.addParameter(cent_transpose_parameter_info);
    parameters.addParameter(pitch_tracking_parameter_info);
    parameters.addParameter(pitch_latency_parameter_info);
    parameters.addParameter(input_gain_parameter_info);
    parameters.addParameter(dc_offset_filter_parameter_info);

    if (HarmonicFlipper::numHarmonics > 99) {
        throw "HarmonicFlipper::numHarmonics must be <= 99.";
    }
    for (short i = 0; i < HarmonicFlipper::numHarmonics; i++) {
        Vst::TChar harmonic_number_high = (i+1)/10 + 48;
        Vst::TChar harmonic_number_low = (i+1) % 10 + 48;
        Vst::ParameterInfo param_info = {kParamFirstHarmonicId + i,
                                STR16("harmonic 00"),
                                STR16("h00"),
                                STR16(""),
                                2, // stepCount
                                0, // defaultNormalizedValue
                                Vst::kRootUnitId, // unitId
                                Vst::ParameterInfo::kCanAutomate};
        param_info.title[9] = harmonic_number_high;
        param_info.title[10] = harmonic_number_low;
        param_info.shortTitle[1] = harmonic_number_high;
        param_info.shortTitle[2] = harmonic_number_low;
        parameters.addParameter(param_info);
    }

    if (HarmonicFlipper::numSubharmonics > 99) {
        throw "HarmonicFlipper::numSubharmonics must be <= 99.";
    }
    for (short i = 0; i < HarmonicFlipper::numSubharmonics; i++) {
        Vst::TChar harmonic_number_high = (i+1)/10 + 48;
        Vst::TChar harmonic_number_low = (i+1) % 10 + 48;
        Vst::ParameterInfo param_info = {kParamFirstSubharmonicId + i,
                                STR16("subharmonic 00"),
                                STR16("sh00"),
                                STR16(""),
                                2, // stepCount
                                0, // defaultNormalizedValue
                                Vst::kRootUnitId, // unitId
                                Vst::ParameterInfo::kCanAutomate};
        param_info.title[12] = harmonic_number_high;
        param_info.title[13] = harmonic_number_low;
        param_info.shortTitle[2] = harmonic_number_high;
        param_info.shortTitle[3] = harmonic_number_low;
        parameters.addParameter(param_info);
    }

    return kResultTrue;
}

//TODO: Probably change this
IPlugView* PLUGIN_API PlugController::createView (const char* name)
{
    // someone wants my editor
    if (name && strcmp (name, "editor") == 0)
    {
        auto* view = new VSTGUI::VST3Editor (this, "view", "plug.uidesc");
        return view;
    }
    return nullptr;
}

tresult PLUGIN_API PlugController::setComponentState(IBStream* state) {

    if (!state) {
        return kResultFalse;
    }

    IBStreamer streamer(state, kLittleEndian);

    bool bypass;
    if(!streamer.readBool(bypass)) {
        return kResultFalse;
    }

    double pitchbend;
    if(!streamer.readDouble(pitchbend)) {
        return kResultFalse;
    }

    int8 harmonic_states[HarmonicFlipper::numHarmonics];
    for (short i = 0; i < HarmonicFlipper::numHarmonics; i++) {
        if (!streamer.readInt8(harmonic_states[i])) {
            return kResultFalse;
        }
    }

    int8 subharmonic_states[HarmonicFlipper::numSubharmonics];
    for (short i = 0; i < HarmonicFlipper::numSubharmonics; i++) {
        if (!streamer.readInt8(subharmonic_states[i])) {
            return kResultFalse;
        }
    }

    setParamNormalized(kParamBypassId, bypass ? 1 : 0);
    setParamNormalized(kParamPitchbendId, pitchbend);

    for (short i = 0; i < HarmonicFlipper::numHarmonics; i++) {
        setParamNormalized(kParamFirstHarmonicId + i, harmonic_states[i]);
    }

    for (short i = 0; i < HarmonicFlipper::numSubharmonics; i++) {
        setParamNormalized(kParamFirstSubharmonicId + i, subharmonic_states[i]);
    }

    return kResultTrue;
}

tresult PLUGIN_API PlugController::getMidiControllerAssignment(int32 bus_index, int16 channel, Vst::CtrlNumber midi_controller_number, Vst::ParamID& id) {

    if (midi_controller_number == Vst::kPitchBend) {
        //TODO channel selector stuff...
        id = kParamPitchbendId;
        return kResultTrue;
    }

    return kResultFalse;
}

tresult PLUGIN_API PlugController::notify(Vst::IMessage* message) {
    std::string message_id = message->getMessageID();
    if (message_id == "LatencyChanged") {
        componentHandler->restartComponent(Vst::kLatencyChanged);
    }
    return EditController::notify(message);
}

const Vst::ParameterInfo PlugController::bypass_parameter_info = {
    kParamBypassId, // id
    STR16("bypass"), // title
    STR16("bypass"), // shortTitle
    STR16(""), // units
    1, // stepCount
    0, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass // flags
};

const Vst::ParameterInfo PlugController::pitchbend_parameter_info = {
    kParamPitchbendId, // id
    STR16("pitchbend"), // title
    STR16("bend"), // shortTitle
    STR16(""), // units
    0, // stepCount
    0.5, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::mix_knob_parameter_info = {
    kParamMixId, // id
    STR16("mix knob"), // title
    STR16("mix"), // shortTitle
    STR16("%"), // units
    0, // stepCount
    1.0, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::input_gain_parameter_info = {
    kParamInputGainId, // id
    STR16("input gain"), // title
    STR16("in"), // shortTitle
    STR16(""), // units
    0, // stepCount
    0.5, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::output_gain_parameter_info = {
    kParamOutputGainId, // id
    STR16("output gain"), // title
    STR16("out"), // shortTitle
    STR16(""), // units
    0, // stepCount
    0.5, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::stereo_flip_flip_parameter_info = {
    kParamStereoFlipFlipId, // id
    STR16("stereo flip-flip"), // title
    STR16("wide"), // shortTitle
    STR16(""), // units
    2, // stepCount
    0/2, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::pitchbend_range_parameter_info = {
    kParamPitchbendRangeId, // id
    STR16("pitchbend range +/-"), // title
    STR16("range"), // shortTitle
    STR16("semitones"), // units
    24, // stepCount
    7/24, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::octave_transpose_parameter_info = {
    kParamOctaveTransposeId, // id
    STR16("octave"), // title
    STR16("8va"), // shortTitle
    STR16(""), // units
    10, // stepCount
    5/10, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::semitone_transpose_parameter_info = {
    kParamSemitoneTransposeId, // id
    STR16("semitone"), // title
    STR16("semi"), // shortTitle
    STR16(""), // units
    24, // stepCount
    12/24, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::cent_transpose_parameter_info = {
    kParamCentTransposeId, // id
    STR16("cent"), // title
    STR16("cent"), // shortTitle
    STR16(""), // units
    0, // stepCount
    0.5, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::pitch_tracking_parameter_info = {
    kParamPitchTrackingId, // id
    STR16("pitch tracking"), // title
    STR16("tracking"), // shortTitle
    STR16(""), // units
    1, // stepCount
    0, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass // flags
};

const Vst::ParameterInfo PlugController::pitch_latency_parameter_info = {
    kParamPitchLatencyId, // id
    STR16("pitch detection latency"), // title
    STR16("latency"), // shortTitle
    STR16(""), // units
    12, // stepCount
    0/12, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::poly_count_parameter_info = {
    kParamPolyCountId, // id
    STR16("polyphony count"), // title
    STR16("poly count"), // shortTitle
    STR16(""), // units
    16, // stepCount
    8/16, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::poly_law_parameter_info = {
    kParamPolyLawId, // id
    STR16("polyphony add/drop law"), // title
    STR16("poly law"), // shortTitle
    STR16(""), // units
    1, // stepCount
    0, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};

const Vst::ParameterInfo PlugController::dc_offset_filter_parameter_info = {
    kParamDcOffsetFilterId, // id
    STR16("remove DC offset"), // title
    STR16("DC filt."), // shortTitle
    STR16(""), // units
    1, // stepCount
    1, // defaultNormalizedValue
    Vst::kRootUnitId, // unitId
    Vst::ParameterInfo::kCanAutomate // flags
};
