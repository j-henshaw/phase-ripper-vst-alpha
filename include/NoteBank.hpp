#pragma once

#include <vector>
#include "NoteInfo.hpp"
#include "StereoAudioOutputFramebuffer.hpp"
#include "pluginterfaces/vst/vsttypes.h"

class NoteBank {
public:
    enum PolyphonyLaw {
        poly_stop,
        poly_drop
    };

    NoteBank();

    void handlePitchTracking(double frequency, double pitchbend);

    void clearVoiceBank();
    
    void setPolyLaw(PolyphonyLaw law);
    
    void setPolyCount(short count);

    void noteOn(Steinberg::int16 pitch, double pitchbend, Steinberg::int32 note_id);
    
    void noteOn(double fundamental, double pitchbend, Steinberg::int32 note_id);
    
    void noteOff(Steinberg::int16 pitch, Steinberg::int32 note_id);

    void setPitchbend(double pitchbend);

    void setPitchbendRange(short pitchbend_range);

    [[nodiscard]] double getPitchbendRange() const;

    void updateTime(int sampleRate);

    class iterator {
    public:
        iterator();
        iterator(NoteBank* note_bank, std::size_t i);
        bool operator==(const iterator& rhs) const;
        bool operator!=(const iterator& rhs) const;
        NoteInfo& operator*() const;
        NoteInfo* operator->() const;
        iterator& operator++();
        iterator operator++(int);

        using difference_type = std::ptrdiff_t;
        using value_type = NoteInfo;
        using pointer = NoteInfo*;
        using reference = NoteInfo&;
        using iterator_category = std::forward_iterator_tag;

    private:
        NoteBank* note_bank;
        std::size_t i;
    };

    class const_iterator {
    public:
        const_iterator();
        const_iterator(const NoteBank* note_bank, std::size_t i);
        bool operator==(const const_iterator& rhs) const;
        bool operator!=(const const_iterator& rhs) const;
        const NoteInfo& operator*() const;
        const NoteInfo* operator->() const;
        const_iterator& operator++();
        const_iterator operator++(int);

        using difference_type = std::ptrdiff_t;
        using value_type = NoteInfo;
        using pointer = NoteInfo*;
        using reference = NoteInfo&;
        using iterator_category = std::forward_iterator_tag;

    private:
        const NoteBank* note_bank;
        std::size_t i;
    };

    [[nodiscard]] iterator begin();
    [[nodiscard]] iterator end();
    [[nodiscard]] const_iterator cbegin() const;
    [[nodiscard]] const_iterator cend() const;

private:

    [[nodiscard]] double pitch_to_fundamental(Steinberg::int16 pitch) const;
    [[nodiscard]] double pitchbend_to_pitchbend_factor(double pitchbend) const;

    static const std::size_t MaxVoices = 64;
    //TODO use different structure?
    std::vector<NoteInfo> voices;
    short polyphony_count = 8;

    PolyphonyLaw poly_law = poly_stop;
    static const double twlthRt; //= std::pow(2, 1.0/12);
    static const double a_neg2;  //= 6.875;
    
    short pitchbend_range = 7;
};

