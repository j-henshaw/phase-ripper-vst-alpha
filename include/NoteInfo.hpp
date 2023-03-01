#pragma once
#include "pluginterfaces/vst/vsttypes.h"

struct NoteInfo {
    // fundamental frequency of the note in hertz
    double fundamental;

    // current pitchbend factor
    // I think setting this with:
    // (12thRoot(2)^(bendAmt * pitchBendRange))
    // Will work if -1 < bendAmt < 1
    double pitchbend = 1;
    
    // time since start of note in seconds
    double time;
    
    //Note ID for comparing on/off data
    Steinberg::int32 noteID;
    
};
