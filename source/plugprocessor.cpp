#include "../include/plugprocessor.hpp"
#include "../include/plugids.hpp"
#include <atomic>
#include <cmath>
#include <chrono>
#include <iostream>

//-----------------------------------------------------------------------------
FUnknown* PlugProcessor::createInstance(void*) {
    return static_cast<Vst::IAudioProcessor*>(new PlugProcessor());
}

//-----------------------------------------------------------------------------
PlugProcessor::PlugProcessor() : updateLatency(false){
    setControllerClass(ControllerUID);
    
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::initialize(FUnknown* context) {
    if(AudioEffect::initialize(context) != kResultTrue) {
        return kResultFalse;
   }

    addAudioInput(STR16("AudioInput"), Vst::SpeakerArr::kStereo);
    addAudioOutput(STR16("AudioOutput"), Vst::SpeakerArr::kStereo);

    return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::setBusArrangements(Vst::SpeakerArrangement* inputs, int32 input_count, Vst::SpeakerArrangement* outputs, int32 output_count) {
    if (input_count != 1 || output_count != 1 || inputs[0] != Vst::SpeakerArr::kStereo || outputs[0] != Vst::SpeakerArr::kStereo) {
        // plugin only supports one stereo input bus and one stereo output bus.
        return kResultFalse;
    }
    return AudioEffect::setBusArrangements(inputs, input_count, outputs, output_count);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::setupProcessing(Vst::ProcessSetup& setup) {
    sample_rate = setup.sampleRate;
    //TODO low freq bound should depend on latency
    pitch_detector = std::make_unique<cycfi::q::pitch_detector>(20, 20000, sample_rate, PITCH_DETECTOR_HYSTERESIS);
    
    //Update filters with new sample_rate
    left_filter.updateSampleRate(sample_rate);
    right_filter.updateSampleRate(sample_rate);
    
    return AudioEffect::setupProcessing(setup);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::canProcessSampleSize(int32 symbolic_sample_size) {
    if (symbolic_sample_size == Vst::kSample64) {
        return kResultTrue;
    }
    return kResultFalse;
}

//-----------------------------------------------------------------------------
///LOOKS LIKE WE WERE MISSING THIS METHOD, NOT SURE IF THAT WAS AFFECTING ANYTHING
tresult PLUGIN_API PlugProcessor::setActive (TBool state)
{
    if (state) // Initialize
    {
        runLatencyChangeThread.store(true, std::memory_order_release);
        latencyChangeNotifier = std::thread(&PlugProcessor::latencyChangeNotifierFunc, this);
        // Allocate Memory Here
        // Ex: algo.create ();
    }
    else // Release
    {
        runLatencyChangeThread.store(false, std::memory_order_release);
        if (latencyChangeNotifier.joinable()) {
            latencyChangeNotifier.join();
        }
        // Free Memory if still allocated
        // Ex: if(algo.isCreated ()) { algo.destroy (); }
    }
    return AudioEffect::setActive (state);
}

//-----------------------------------------------------------------------------
///PROBABLY HAVE TO UPDATE THIS ONE AT SOME POINT ONCE WE UNDERSTAND BETTER HOW IT WORKS
uint32 PLUGIN_API PlugProcessor::getLatencySamples(){
    if (latency == 0) return 0;
    return std::pow(2,latency);
}

bool PlugProcessor::isDiscreteAutomation(Vst::ParamID parameter) {
    return ! (isContinuousAutomation(parameter));
}

bool PlugProcessor::isContinuousAutomation(Vst::ParamID parameter){
    return ( parameter < 100 );
}

//Equal power computation
std::pair<float, float> PlugProcessor::dryWetMix(float mixAmt) {
    float mixTarget = mixAmt * 1000;
    
    float temp = ((321000 + 679 * mixTarget) * mixTarget) / 1000000;
    float dryScale = 1000000 - temp * temp;
    dryScale /= 1000000;
    
    temp = ((321000 + 679 * (1000.0 - mixTarget)) * (1000.0 - mixTarget)) / 1000000;
    float wetScale = 1000000 - temp * temp;
    wetScale /= 1000000;
    return {dryScale, wetScale};
}

void PlugProcessor::heapDiscreteAutomations(Vst::IParamValueQueue* point_queue){
    Vst::ParamValue status;
    int32 sample_offset;
    //for each automation in the queue, add it to the discrete_automation_heap priority_queue
    for(int h = 0; h < point_queue->getPointCount(); ++h){
        point_queue->getPoint(h, sample_offset, status);
        //TODO does sample_offset need to change for insertion int ring-buffer?
        const DiscreteAutomation thisAuto = {point_queue->getParameterId(), status, sample_offset};
        discrete_automation_heap.push(thisAuto);
    }
}
//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::process(Vst::ProcessData& data) {
    if(data.numInputs != 1 || data.numOutputs != 1) {
        return kResultTrue;
    }
    
    block_size = data.numSamples;
    
    //###########################//
    // AUTOMATION INITIALIZATION //
    //###########################//

    //For each automation queue
    if(data.inputParameterChanges){
        for (int32 i = 0; i < data.inputParameterChanges->getParameterCount(); ++i) {
            //Grab test statistic
            Vst::IParamValueQueue* this_param_queue = data.inputParameterChanges->getParameterData(i);
            Vst::ParamID paramType = this_param_queue->getParameterId();

            //If it's a continuous automation, build the queue
            if (isContinuousAutomation(paramType)){
                continuousAutos.pushAutomationData(this_param_queue, block_size);
            }

            //If it's discrete, heap them up
            else if (isDiscreteAutomation(paramType)){
                heapDiscreteAutomations(this_param_queue);
            }

        }
        
        //After all continuous auto data is pushed, we can:
        continuousAutos.fillZeroDeltas(block_size);
        
        //After moving through all automation queues, move discretes from heap to ring buffer
        while (discrete_automation_heap.size() > 0) {
            //TODO make toggle_automations a ring-buffer (but not w/ q-lib)
            toggle_automations.push(discrete_automation_heap.top());
            discrete_automation_heap.pop();
        }
    }

//—————————————————————————————————————————————————————————————————————————————————————————————//
    //#####################//
    // BIG PER-SAMPLE LOOP //
    //#####################//
    for (int32 i = 0; i < block_size; ++i) {
        
        //Apply input gain
        data.inputs[0].channelBuffers64[0][i] *= (continuousAutos.getParameterValue(kParamInputGainId) * 2);
        data.inputs[0].channelBuffers64[1][i] *= (continuousAutos.getParameterValue(kParamInputGainId) * 2);

        //TODO add to DiscreteAutomation
        //################//
        // ADD/DROP NOTES //
        //################//
        //Keep notebank updated regardless of whether we're pitch tracking
        //For each Event
        if (data.inputEvents) {
            for(int32 v = 0; v < data.inputEvents->getEventCount(); ++v){
                Vst::Event e;
                data.inputEvents->getEvent(v, e);
                
                //If we're on the right sample
                if(e.sampleOffset - i == 0) {
                    //Note Off
                    if(e.type == Vst::Event::kNoteOffEvent) {
                        note_bank.noteOff(e.noteOff.pitch, e.noteOff.noteId);
                    }
                    //Note On: Pitchbend is default, update it later
                    if(e.type == Vst::Event::kNoteOnEvent) {
                        note_bank.noteOn(e.noteOn.pitch, continuousAutos.getParameterValue(kParamPitchbendId), e.noteOn.noteId);
                    }
                }
            }
        }
        
        //PITCH TRACKING
        ///THIS IS REAL BAD :D GOTTA IMPLEMENT FOR REAL STILL
        //ALSO: Do we want to do this whether or not we're pitch tracking? would make for smoother
        //transition between states, but would be unnecessarily power-hungry when not in use
        if(pitchTracking){
            detectedFreq = pitch_detector->get_frequency();
            //We may need to implement a continuous automation curve
            //Including both methods I wrote here, neither is good I think,
            //just so we can remember what I did
            //ALSO TO CONSIDER: Stereo signals?
            //detected_bank.noteOn(detectedFreq, continuousAutos.getParameterValue(kParamPitchbendId), 1);
            detected_bank.handlePitchTracking(detectedFreq, continuousAutos.getParameterValue(kParamPitchbendId));
        }
               
        //####################//
        // CNTNS. AUTOMATIONS //
        //####################//
        continuousAutos.updateParameters();
        
        //Now that state data is updated for this sample:
        //Update notebank's pitchbend
        note_bank.setPitchbend(continuousAutos.getParameterValue(kParamPitchbendId));
        detected_bank.setPitchbend(continuousAutos.getParameterValue(kParamPitchbendId));
        
        
        
        //####################//
        // TOGGLE AUTOMATIONS //
        //####################//
        //While the top toggle automation is supposed to happen this sample
        while(toggle_automations.size() > 0 && toggle_automations.top().sampleOffset == i){
            //Pop it off
            DiscreteAutomation automation = toggle_automations.top();
            toggle_automations.pop();
            
            //Perform the automation: Using the iterator allows us to handle the fact that the
            //Harmonic & Subharmonic IDs are ranges, with lower_bound
            auto map_iter = discrete_automation_functions.lower_bound(automation.parameterID);
            map_iter->second(automation);
        }
        
        
        //Now that both discrete and continuous transpose data is updated:
        //Set new MIDI Transpose in the harmonic flipper
        //Math for converting stored values to +/- semitones is done on this line
        flipper.setMidiTranspose( ((octaveTranspose - 5) * 12) + (semitoneTranspose - 12) + (continuousAutos.getParameterValue(kParamCentTransposeId) * 2 - 1) );

        
        
        //############//
        // SET OUTPUT //
        //############//
        if(!bypass){
            //Do the flips—takes into account whether we are pitch tracking or not via ternary expression
            short modifier = flipper.process(pitchTracking ? detected_bank:note_bank);
            data.outputs[0].channelBuffers64[0][i] = data.inputs[0].channelBuffers64[0][i] * modifier;
            data.outputs[0].channelBuffers64[1][i] = data.inputs[0].channelBuffers64[1][i] * modifier;
            note_bank.updateTime(sample_rate);
            detected_bank.updateTime(sample_rate);
        
            //Apply Stereo flip-flip
            if (stereoFlip.inverted){
                data.outputs[0].channelBuffers64[0][i] *= -1;
            }else if (stereoFlip.active){
                data.outputs[0].channelBuffers64[1][i] *= -1;
            }
            
            //Apply DC Offset Filter
            if (dcFilterOn){
                data.outputs[0].channelBuffers64[0][i] -= left_filter.filterNewSample(data.outputs[0].channelBuffers64[0][i]);
                data.outputs[0].channelBuffers64[1][i] -= right_filter.filterNewSample(data.outputs[0].channelBuffers64[1][i]);
            }
            
            //Apply Mix Amt--equal power equation found elsewhere
            const auto drywet = dryWetMix(continuousAutos.getParameterValue(kParamMixId));
            const float dryScale = drywet.first;
            const float wetScale = drywet.second;

            (*pitch_detector)(data.inputs[0].channelBuffers64[0][i] + data.inputs[0].channelBuffers64[1][i]);

            data.outputs[0].channelBuffers64[0][i] = data.inputs[0].channelBuffers64[0][i] * dryScale + data.outputs[0].channelBuffers64[0][i] * wetScale;
            data.outputs[0].channelBuffers64[1][i] = data.inputs[0].channelBuffers64[1][i] * dryScale + data.outputs[0].channelBuffers64[1][i] * wetScale;
            
            //Apply Output Gain
            data.outputs[0].channelBuffers64[0][i] *= (continuousAutos.getParameterValue(kParamOutputGainId) * 2);
            data.outputs[0].channelBuffers64[1][i] *= (continuousAutos.getParameterValue(kParamOutputGainId) * 2);
            
            //Apply Haxx (REMEMBER: This is because of how Reaper reads the effect type—have to look into this)
            data.outputs[0].channelBuffers64[0][i] -= data.inputs[0].channelBuffers64[0][i];
            data.outputs[0].channelBuffers64[1][i] -= data.inputs[0].channelBuffers64[1][i];
            
        }else {
            data.outputs[0].channelBuffers64[0][i] = data.inputs[0].channelBuffers64[0][i];
            data.outputs[0].channelBuffers64[1][i] = data.inputs[0].channelBuffers64[1][i];
        }
    }
    
    
    //TODO remove this
    std::cout << detectedFreq << std::endl;
//—————————————————————————————————————————————————————————————————————————————————————————————//

    //We did it!!
    return kResultTrue;
}

//TODO: Fix this to work with new ContinuousAutomationHandler, update to include all params
tresult PLUGIN_API PlugProcessor::setState(IBStream* state) {
    if(!state) {
        return kResultFalse;
    }

    IBStreamer streamer(state, kLittleEndian);

    bool bypass_value;
    if (!streamer.readBool(bypass_value)) {
        return kResultFalse;
    }

    double pitchbend_value;
    if (!streamer.readDouble(pitchbend_value)) {
        return kResultFalse;
    }
    
    int8 harmonic_states[flipper.numHarmonics];
    for (short i = 0; i < flipper.numHarmonics; i++) {
        if (!streamer.readInt8(harmonic_states[i])) {
            return kResultFalse;
        }
    }

    int8 subharmonic_states[flipper.numHarmonics];
    for (short i = 0; i < flipper.numSubharmonics; i++) {
        if (!streamer.readInt8(subharmonic_states[i])) {
            return kResultFalse;
        }
    }

    bypass = bypass_value;
    continuousAutos.setParameterValue(kParamPitchbendId, pitchbend_value);
    for (short i = 0; i < flipper.numHarmonics; i++ ) {
        flipper.setStatus(i+1, static_cast<HarmonicFlipper::HarmonicStatus>(harmonic_states[i]));
    }
    for (short i = 0; i < flipper.numSubharmonics; i++ ) {
        flipper.setStatus(-1-i, static_cast<HarmonicFlipper::HarmonicStatus>(subharmonic_states[i]));
    }

    return kResultTrue;
}

tresult PLUGIN_API PlugProcessor::getState(IBStream* state) {
    IBStreamer streamer(state, kLittleEndian);

    streamer.writeBool(bypass);
    streamer.writeDouble(continuousAutos.getParameterValue(kParamPitchbendId));
    for (short i = 0; i < flipper.numHarmonics; i++) {
        streamer.writeInt8(static_cast<int8>(flipper.getStatus(i+1)));
    }
    for (short i = 0; i < flipper.numSubharmonics; i++) {
        streamer.writeInt8(static_cast<int8>(flipper.getStatus(-1-i)));
    }
    return kResultTrue;
}

void PlugProcessor::latencyChangeNotifierFunc() {
    while (runLatencyChangeThread.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (updateLatency.load(std::memory_order_relaxed)) {
            Vst::IMessage* message = allocateMessage();
            message->setMessageID("LatencyChanged");
            sendMessage(message);
            updateLatency.store(false, std::memory_order_relaxed);
        }
    }
}
