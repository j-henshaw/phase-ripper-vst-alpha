#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "DCOffsetFilter.hpp"
#include "NoteBank.hpp"
#include "HarmonicFlipper.hpp"
#include <queue>
#include "DiscreteAutomation.hpp"
#include "ContinuousAutomationHandler.hpp"
#include "plugids.hpp"

#include "pluginterfaces/vst/ivstevents.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "../include/StereoAudioOutputFramebuffer.hpp"
#include <q/utility/ring_buffer.hpp>
#include <map>
#include <atomic>
#include <thread>
#include <q/pitch/pitch_detector.hpp>
#include <iostream>

using namespace Steinberg;

class PlugProcessor : public Vst::AudioEffect {
public:

    PlugProcessor();

    tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;  

    tresult PLUGIN_API setBusArrangements(Vst::SpeakerArrangement* inputs, int32 input_count,
                                          Vst::SpeakerArrangement* outputs, int32 output_count) SMTG_OVERRIDE;

    tresult PLUGIN_API setupProcessing(Vst::ProcessSetup& setup) SMTG_OVERRIDE;

    tresult PLUGIN_API canProcessSampleSize(int32 symbolic_sample_size) SMTG_OVERRIDE;

    tresult PLUGIN_API process(Vst::ProcessData& data) SMTG_OVERRIDE;

    tresult PLUGIN_API setState(IBStream* state) SMTG_OVERRIDE;

    tresult PLUGIN_API getState(IBStream* state) SMTG_OVERRIDE;
    
    tresult PLUGIN_API setActive (TBool state) SMTG_OVERRIDE;

    uint32 PLUGIN_API getLatencySamples() SMTG_OVERRIDE;

    static FUnknown* createInstance(void*);

private:
    Vst::SampleRate sample_rate;
    
    int block_size;
    
    HarmonicFlipper flipper;

    NoteBank note_bank;
    
    NoteBank detected_bank;
    
    ContinuousAutomationHandler continuousAutos;
    
    DCOffsetFilter left_filter;
    DCOffsetFilter right_filter;
    bool dcFilterOn = true;

    bool bypass = false;
    
    double blockAvgL = 0;
    
    double blockAvgR = 0;
    
    bool pitchTracking = false;
    
    double detectedFreq = 0.0;
    
    short latency = 0;
    
    std::atomic<bool> updateLatency;

    std::atomic<bool> runLatencyChangeThread;

    std::thread latencyChangeNotifier;

    short pitchbendRange = 7;
    
    short octaveTranspose = 5;
    
    short semitoneTranspose = 13;
    
    HarmonicFlipInfo stereoFlip = {false, false};
    
    std::priority_queue<DiscreteAutomation> discrete_automation_heap;

    std::priority_queue<DiscreteAutomation> toggle_automations;

    static constexpr double PITCH_DETECTOR_HYSTERESIS = 0.1;

    std::unique_ptr<cycfi::q::pitch_detector> pitch_detector;
    
    //Helper methods
    bool isDiscreteAutomation(Vst::ParamID paramType);
    bool isContinuousAutomation(Vst::ParamID paramType);
    void heapDiscreteAutomations(Vst::IParamValueQueue* point_queue);
    std::pair<float, float> dryWetMix(float mixAmt);

    void latencyChangeNotifierFunc();
    
    //Discrete Automation Handling
    typedef std::function<void(const DiscreteAutomation& automation)> discrete_auto_fn;
    const std::map<Steinberg::Vst::ParamID, discrete_auto_fn> discrete_automation_functions = {
        
        //Pitchbend Range
        {kParamPitchbendRangeId, [&](const DiscreteAutomation& automation){
            note_bank.setPitchbendRange(static_cast<short>(automation.value));
            detected_bank.setPitchbendRange(static_cast<short>(automation.value));
        }},

        //Bypass
        {kParamBypassId, [&](const DiscreteAutomation& automation){
            bypass = !!automation.value;
        }},
    
        //Stereo Flip-Flip
        {kParamStereoFlipFlipId, [&](const DiscreteAutomation& automation){
            if (static_cast<short>(automation.value * 2) == 0){
                stereoFlip.active = false;
                stereoFlip.inverted = false;
            }else if (static_cast<short>(automation.value * 2) == 1){
                stereoFlip.active = true;
                stereoFlip.inverted = false;
            }else{
                stereoFlip.active = true;
                stereoFlip.inverted = true;
            }
        }},
    
        //Octave Transpose
        {kParamOctaveTransposeId, [&](const DiscreteAutomation& automation){
            //This math converts the param values to int
            octaveTranspose = automation.value * 10;
        }},
    
        //Semitone Transpose
        {kParamSemitoneTransposeId, [&](const DiscreteAutomation& automation){
            //This math converts the param values to int
            semitoneTranspose = automation.value * 24;
        }},
    
        //Latency Change
        {kParamPitchLatencyId, [&](const DiscreteAutomation& automation){
            //UPDATE DAW: https://sdk.steinberg.net/viewtopic.php?t=209
            //this also links to a vst3 'helpfile' I did not know existed
            latency = static_cast<short>(automation.value * 12);
            updateLatency.store(true, std::memory_order_relaxed);
        }},
    
        //Polyphony Count
        {kParamPolyCountId, [&](const DiscreteAutomation& automation){
            note_bank.setPolyCount(static_cast<short>(automation.value * 16));
            detected_bank.setPolyCount(static_cast<short>(automation.value * 16));
        }},
    
        //Poly Law
        {kParamPolyLawId, [&](const DiscreteAutomation& automation){
            note_bank.setPolyLaw(static_cast<NoteBank::PolyphonyLaw>(automation.value));
            detected_bank.setPolyLaw(static_cast<NoteBank::PolyphonyLaw>(automation.value));
        }},
    
        //Pitch Tacking On/Off
        {kParamPitchTrackingId, [&](const DiscreteAutomation& automation){
            pitchTracking = !!automation.value;
            detectedFreq = 0.0; //UPDATE THIS LINE WITH Q-LIB FUNC
            ///We might also do a continuous automation curve and update THAT curve below the discrete autos, separate from the other continuous autos
            //We also might also track continuously and simply switch between them
        }},
        
        //DC filter on/off
        {kParamDcOffsetFilterId, [&](const DiscreteAutomation& automation){
            dcFilterOn = !!automation.value;
        }},
    
        //Harmonics
        {kParamFirstSubharmonicId - 1, [&](const DiscreteAutomation& automation){
            short harmonic = automation.parameterID - kParamFirstHarmonicId + 1;
            flipper.setStatus(harmonic, static_cast<HarmonicFlipper::HarmonicStatus>(automation.value*2));
        }},
    
        //Subharmonics
        {kParamFirstSubharmonicId + HarmonicFlipper::numSubharmonics, [&](const DiscreteAutomation& automation){
            short harmonic = kParamFirstSubharmonicId - automation.parameterID - 1;
            flipper.setStatus(harmonic, static_cast<HarmonicFlipper::HarmonicStatus>(automation.value*2));
        }}
    };
    
};
