#ifndef ContinuousAutomationHandler_hpp
#define ContinuousAutomationHandler_hpp

#include "DeltaData.hpp"
#include "plugids.hpp"
#include <map>
#include <queue>

//Unknown which, if any, of these is needed
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "../include/StereoAudioOutputFramebuffer.hpp"

//Are we?
using namespace Steinberg;
using namespace Steinberg::Vst;

class ContinuousAutomationHandler {
public:

    ContinuousAutomationHandler();
    void pushAutomationData(Vst::IParamValueQueue* point_queue, int block_size);
    void fillZeroDeltas(int block_size);
    void updateParameters();
    double getParameterValue(Vst::ParamID parameter);
    void setParameterValue(Vst::ParamID parameter, double value);
    
    //IF MORE continuous parameters are added, simply add their parameter IDs here and change the size
    static constexpr std::array<Vst::ParamID, 5> CONTINUOUS_PARAMETER_IDS = {
        kParamPitchbendId,
        kParamMixId,
        kParamOutputGainId,
        kParamCentTransposeId,
        kParamInputGainId
    };

private:
    std::map<Vst::ParamID, std::queue<DeltaData>> delta_queues;
    std::map<Vst::ParamID, double> state_data;
    std::map<Vst::ParamID, bool> deltas_this_block;
    
    void pushContinuousAutomationSegment(Vst::ParamID parameter, double delta, int duration);

};


#endif /* ContinuousAutomationHandler_hpp */

