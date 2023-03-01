//
//  NoteBank.cpp
//
//
//  Created by Moon Rock Sound on 12/18/19.
//

#include "../include/NoteBank.hpp"
#include <cmath>
#include <algorithm>

using namespace Steinberg;
using namespace Steinberg::Vst;

NoteBank::NoteBank() {
    voices.reserve(MaxVoices);
}

const double NoteBank::twlthRt = std::pow(2, 1.0/12);
const double NoteBank::a_neg2 = 6.875;


void NoteBank::noteOn(Steinberg::int16 pitch, double pitchbend, Steinberg::int32 note_id) {
    this->noteOn(pitch_to_fundamental(pitch), pitchbend, note_id);
}

//Overloaded to accept double instead of MIDI note
void NoteBank::noteOn(double fundamental, double pitchbend, Steinberg::int32 note_id) {
    double pitchbend_factor = pitchbend_to_pitchbend_factor(pitchbend);
    NoteInfo note = {fundamental, pitchbend_factor, 0, note_id};
    if (voices.size() < voices.capacity()) {
        voices.push_back(note);
    }
}

void NoteBank::noteOff(Steinberg::int16 pitch, Steinberg::int32 note_id) {
    if (note_id == -1) {
        for (int i = 0; i < voices.size(); ++i) {
            double fund = pitch_to_fundamental(pitch);
            // pitch_to_fundamental is used to calculate both sides of double comparison so floating-point error is not an issue here
            if (voices[i].noteID == note_id && voices[i].fundamental == fund) {
                voices.erase(voices.begin()+i);
                break;
            }
        }
    } else {
        for (int i = 0; i < voices.size(); ++i) {
            if (voices[i].noteID == note_id) {
                voices.erase(voices.begin()+i);
                break;
            }
        }
    }
}

void NoteBank::setPitchbend(double pitchbend) {
    double pitchbend_factor = pitchbend_to_pitchbend_factor(pitchbend);
    for (NoteInfo& note : voices) {
        note.pitchbend = pitchbend_factor;
    }
}

void NoteBank::setPitchbendRange(short pitchbend_range) {
    this->pitchbend_range = pitchbend_range;
}

double NoteBank::getPitchbendRange() const {
    return pitchbend_range;
}

void NoteBank::setPolyLaw(PolyphonyLaw law){
    poly_law = law;
}

void NoteBank::setPolyCount(short count){
    polyphony_count = count;
}

void NoteBank::updateTime(int sampleRate) {
    for(int t = 0; t < voices.size(); ++t){
        voices.at(t).time += (1.0/sampleRate);
    }
}

void NoteBank::clearVoiceBank(){
    voices.clear();
}

void NoteBank::handlePitchTracking(double frequency, double pitchbend){
    double pitchbend_factor = pitchbend_to_pitchbend_factor(pitchbend);
    
    //Clear all but one note
    if(voices.size() != 1){
        voices.clear();
        noteOn(frequency, pitchbend, 1);
    }
    //Otherwise, update the freq
    else {
        voices.front().fundamental = frequency;
        voices.front().pitchbend = pitchbend_factor;
    }
}

NoteBank::iterator NoteBank::begin() {
    return iterator(this, 0);
}

NoteBank::iterator NoteBank::end() {
    return iterator(this, voices.size()<polyphony_count?voices.size():polyphony_count);
}

NoteBank::const_iterator NoteBank::cbegin() const {
    return const_iterator(this, 0);
}

NoteBank::const_iterator NoteBank::cend() const {
    return const_iterator(this, voices.size()<polyphony_count?voices.size():polyphony_count);
}

double NoteBank::pitch_to_fundamental(Steinberg::int16 pitch) const {
    return a_neg2 * std::pow(twlthRt, (pitch+3));
}

double NoteBank::pitchbend_to_pitchbend_factor(double pitchbend) const {
    pitchbend *= 2;
    pitchbend -= 1;
    pitchbend = std::pow(twlthRt, (pitchbend * pitchbend_range));
    return pitchbend;
}

NoteBank::iterator::iterator() {
    note_bank = nullptr;
    i = NoteBank::MaxVoices;
}

NoteBank::iterator::iterator(NoteBank* note_bank, std::size_t i) {
    this->note_bank = note_bank;
    this->i = i;
}

bool NoteBank::iterator::operator==(const NoteBank::iterator& rhs) const {
    return i == rhs.i && note_bank == rhs.note_bank;
}

bool NoteBank::iterator::operator!=(const NoteBank::iterator& rhs) const {
    return i != rhs.i || note_bank != rhs.note_bank;
}

NoteInfo& NoteBank::iterator::operator*() const {
    switch (note_bank->poly_law) {
        case poly_drop:
            return note_bank->voices[note_bank->voices.size()-i-1];
            break;
        case poly_stop:
            return note_bank->voices[i];
            break;
    }
}

NoteInfo* NoteBank::iterator::operator->() const {
    return &(this->operator*());
}

NoteBank::iterator& NoteBank::iterator::operator++() {
    i++;
    return *this;
}

NoteBank::iterator NoteBank::iterator::operator++(int) {
    NoteBank::iterator it = *this;
    it.i++;
    return it;
}

NoteBank::const_iterator::const_iterator() {
    note_bank = nullptr;
    i = NoteBank::MaxVoices;
}

NoteBank::const_iterator::const_iterator(const NoteBank* note_bank, std::size_t i) {
    this->note_bank = note_bank;
    this->i = i;
}

bool NoteBank::const_iterator::operator==(const const_iterator& rhs) const {
    return i == rhs.i && note_bank == rhs.note_bank;
}

bool NoteBank::const_iterator::operator!=(const const_iterator& rhs) const {
    return i != rhs.i || note_bank != rhs.note_bank;
}

const NoteInfo& NoteBank::const_iterator::operator*() const {
    switch (note_bank->poly_law) {
        case poly_drop:
            return note_bank->voices[note_bank->voices.size()-i-1];
            break;
        case poly_stop:
            return note_bank->voices[i];
            break;
    }
}

const NoteInfo* NoteBank::const_iterator::operator->() const {
    return &(this->operator*());
}

NoteBank::const_iterator& NoteBank::const_iterator::operator++() {
    i++;
    return *this;
}

NoteBank::const_iterator NoteBank::const_iterator::operator++(int) {
    NoteBank::const_iterator it = *this;
    it.i++;
    return it;
}

