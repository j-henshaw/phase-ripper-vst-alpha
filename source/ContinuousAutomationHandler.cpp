#include "../include/ContinuousAutomationHandler.hpp"

//----------------------------------------------------------------------
//Constructor
ContinuousAutomationHandler::ContinuousAutomationHandler(){
    //Build maps with each continuous parameter
    for (Vst::ParamID parameter : CONTINUOUS_PARAMETER_IDS){
        delta_queues[parameter] = {};
        state_data[parameter] = 0.5;
        deltas_this_block[parameter] = false;
        

    }
     //MixAmt default isn't 0.5
    state_data.at(kParamMixId) = 1;
}

//----------------------------------------------------------------------

//Used any time a new point queue comes in for a continuous parameter
void ContinuousAutomationHandler::pushAutomationData(Vst::IParamValueQueue* point_queue, int block_size){
    //There will be deltas this block
    deltas_this_block.at(point_queue->getParameterId()) = true;
    
    //initialization
    int32 point = 0;
    int32 offset = 0;
    double targetValue;
    
    int previousOffset = 0;
    double previousTarget = getParameterValue(point_queue->getParameterId());
    
    //While we have points to deal with
    while( point_queue->getPointCount() > point ){
        //Get data for upcoming automation point
        point_queue->getPoint(point, offset, targetValue);
        
        //Calculate delta
        const double delta = (targetValue - previousTarget) / (offset - previousOffset + 1);
        
        //update continuous queue
        pushContinuousAutomationSegment(point_queue->getParameterId(), delta, offset - previousOffset + 1);
        
        //update for next loop
        point_queue->getPoint(point, previousOffset, previousTarget);
        ++point;
    }
    
    //If there are remaining zero-delta samples in this block, add them
    if (offset < block_size){
        pushContinuousAutomationSegment(point_queue->getParameterId(), 0, block_size - offset);
    }
}


//After all point queus are loaded into the Handler, use this to fill in zero-deltas for
//any parameter that isn't receiving deltas this block, for the entire block size.
//CAUTION: Use only once each processing call, only after loading in all automations received
void ContinuousAutomationHandler::fillZeroDeltas(int block_size){
    for (Vst::ParamID parameter : CONTINUOUS_PARAMETER_IDS){
        if (!(deltas_this_block.at(parameter))){
            pushContinuousAutomationSegment(parameter, 0, block_size);
        } else deltas_this_block.at(parameter) = false;
    }
}


//Used once each sample, this progresses all state data by their respective deltas, including zero deltas
void ContinuousAutomationHandler::updateParameters(){
    for (Vst::ParamID parameter : CONTINUOUS_PARAMETER_IDS){
        //Clear any zero-duration deltas
        if (delta_queues.at(parameter).front().duration == 0) {
            delta_queues.at(parameter).pop();
        }
        
        //Update state data by respective delta
        state_data.at(parameter) += delta_queues.at(parameter).front().delta;
        //decrement the duration by 1 sample
        -- delta_queues.at(parameter).front().duration;
    }
}


//Returns the associated state data
double ContinuousAutomationHandler::getParameterValue(Vst::ParamID parameter){
    return state_data.at(parameter);
}

//Sets the state data for given parameter to the provided value
void ContinuousAutomationHandler::setParameterValue(Vst::ParamID parameter, double value){
    state_data.at(parameter) = value;
}

//----------------------------------------------------------------------

//Private helper method to push delta segments
void ContinuousAutomationHandler::pushContinuousAutomationSegment(Vst::ParamID parameter, double delta, int duration){
    //Create and add new segment
    DeltaData newSegment = {delta, duration};
    delta_queues.at(parameter).push(newSegment);
}

