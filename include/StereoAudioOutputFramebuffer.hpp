#pragma once
#include <cstdint>
#include "pluginterfaces/vst/vsttypes.h"

struct StereoAudioOutputFramebuffer{
    Steinberg::Vst::Sample64* left;
    Steinberg::Vst::Sample64* right;
    Steinberg::int32 numSamples;
    Steinberg::Vst::SampleRate sampleRate;
};
