#include "../include/HarmonicFlipper.hpp"
#include <cmath>
#include <string>
#include <stdexcept>

//Setting active/inactive/invert/uninvert Method
void HarmonicFlipper::setStatus(short harmonic, HarmonicStatus status){
    validateHarmonic(harmonic);
    //For positive harmonics
    if(harmonic > 0){
        switch (status){
            case HarmonicFlipper::Inactive:
                harmonics[harmonic-1].active = false;
                harmonics[harmonic-1].inverted = false;
                break;
        
            case HarmonicFlipper::Active:
                harmonics[harmonic-1].active = true;
                harmonics[harmonic-1].inverted = false;
                break;
        
            case HarmonicFlipper::Inverted:
                harmonics[harmonic-1].active = true;
                harmonics[harmonic-1].inverted = true;
                break;
        }
        return;
    }
    
    //For subharmonics
    switch (status){
        case HarmonicFlipper::Inactive:
            subharmonics[-harmonic-1].active = false;
            subharmonics[-harmonic-1].inverted = false;
            break;
    
        case HarmonicFlipper::Active:
            subharmonics[-harmonic-1].active = true;
            subharmonics[-harmonic-1].inverted = false;
            break;
    
        case HarmonicFlipper::Inverted:
            subharmonics[-harmonic-1].active = true;
            subharmonics[-harmonic-1].inverted = true;
            break;
    }
}

HarmonicFlipper::HarmonicStatus HarmonicFlipper::getStatus(short harmonic) const {
    validateHarmonic(harmonic);

    if (harmonic > 0) {
        if(!harmonics[harmonic-1].active) {
            return HarmonicStatus::Inactive;
        }
        if (!harmonics[harmonic-1].inverted) {
            return HarmonicStatus::Active;
        }
        return HarmonicStatus::Inverted;
    }
    if(!subharmonics[-harmonic-1].active) {
        return HarmonicStatus::Inactive;
    }
    if (!subharmonics[-harmonic-1].inverted) {
        return HarmonicStatus::Active;
    }
    return HarmonicStatus::Inverted;
}

//Private validation method for harmonics
void HarmonicFlipper::validateHarmonic(short harmonic) const {
    if(harmonic == 0 || harmonic > numHarmonics || harmonic < -(numSubharmonics))
    throw std::invalid_argument("Invalid Harmonic: " + std::to_string(harmonic) +
                                "! Harmonics can be (1 to " + std::to_string(numHarmonics) +
                                ") or (-1 to -" + std::to_string(numSubharmonics) + ")");
}

//This method is called each sample, and is passed an updated NoteBank
//for sample-accurate automation & note on/off. It then determines whether
//The harmonics for each note have been flipped, and returns the product
//of all active harmonics. This can be used to set the output frame.
short HarmonicFlipper::process(const NoteBank& voices){
    short modifier = 1; //It's a surprise tool that will help us later
    
    //#######################//
    // MODIFY HARMONIC STATE //
    //#######################//
    
    //For each note, we will check each harmonic's state
    for(auto note = voices.cbegin(); note != voices.cend(); ++note){
        //Harmonics
        for(short i = 0; i < numHarmonics; ++i){
            //If the timer shows an odd number of cycles have passed,
            //and this harmonic is active, flip our modifier, including inversion
            if(static_cast<int>(note->time * note->pitchbend * transposeMultiplier * note->fundamental * (i+1)) %2 == 1)
                modifier *= (harmonics[i].active ? -1:1);
            modifier *= harmonics[i].inverted ? -1:1;
        }
        //Subharmonics
        for(short i = 0; i < numSubharmonics; ++i){
            //use subharm+2 because we don't want to repeat the fundamental
            if(static_cast<int>(note->time * note->pitchbend * transposeMultiplier * note->fundamental / (i+2)) %2 == 1)
                modifier *= subharmonics[i].active ? -1:1;
            modifier *= subharmonics[i].inverted ? -1:1;
        }
    }

    return modifier;
}

void HarmonicFlipper::setMidiTranspose(double numSteps){
    transposeMultiplier = std::pow(2, numSteps/12);
}


