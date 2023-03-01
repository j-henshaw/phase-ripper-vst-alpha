#pragma once
#include "pluginterfaces/vst/vsttypes.h"

struct DiscreteAutomation {
    //Type of automation this struct represents
    Steinberg::Vst::ParamID parameterID;
    
    //Value the parameter should be set to at this time
    Steinberg::Vst::ParamValue value;
    
    //Sample offset from the beginning of this block
    Steinberg::int32 sampleOffset;
    
    //Overloading < operator to return the struct with the highest *priority*,
    //meaning the smallest automation offset
    inline bool operator< (const DiscreteAutomation& right) const {
        return sampleOffset > right.sampleOffset;
    }
};
