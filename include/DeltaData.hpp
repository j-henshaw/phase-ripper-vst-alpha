#pragma once
#include "pluginterfaces/vst/vsttypes.h"

struct DeltaData {
    //Value the parameter should be altered by each sample
    const double delta;
    
    //How many samples this delta persists
    Steinberg::int32 duration;

};
