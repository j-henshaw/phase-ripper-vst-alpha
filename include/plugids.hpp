#pragma once 

#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/base/funknown.h"
#include "HarmonicFlipper.hpp"


using namespace Steinberg;

enum PhaseRipperParams : Vst::ParamID {
    //First, continuous autos (MUST BE UNDER 100)
    kParamPitchbendId = 1,
    kParamMixId = 2,
    kParamCentTransposeId = 3,
    kParamOutputGainId = 4,
    
    //Now, discrete (DO NOT GO BELOW 100)
    kParamPitchbendRangeId = 100,
    kParamBypassId = 101,
    kParamStereoFlipFlipId = 102,
    kParamOctaveTransposeId = 103,
    kParamSemitoneTransposeId = 104,
    kParamPitchLatencyId = 105,
    kParamPolyCountId = 106,
    kParamPolyLawId = 107,
    kParamPitchTrackingId = 108,
    kParamInputGainId = 109,
    kParamDcOffsetFilterId = 110,
    kParamFirstHarmonicId = 1000,
    kParamFirstSubharmonicId = kParamFirstHarmonicId + HarmonicFlipper::numHarmonics
};

static const FUID ProcessorUID(0xae8b2851, 0x3f374a15, 0x983059a5, 0x493fa01e);

static const FUID ControllerUID(0x9092ffb0, 0xece34a9f, 0x9e8c84cf, 0x3fd217f3);
