#ifndef HarmonicFlipper_hpp
#define HarmonicFlipper_hpp

#include "HarmonicFlipInfo.hpp"
#include "NoteBank.hpp"
#include <array>

class HarmonicFlipper{
    
public:
    [[nodiscard]] short process(const NoteBank& voices);
    enum HarmonicStatus{Inactive = 0, Active = 1, Inverted = 2};
    void setStatus(short harmonic, HarmonicStatus status);
    HarmonicStatus getStatus(short harmonic) const;
    static const short numHarmonics = 16;
    static const short numSubharmonics = 7;
    void setMidiTranspose(double numSteps);
    
private:
    std::array<HarmonicFlipInfo, numHarmonics> harmonics;
    std::array<HarmonicFlipInfo, numSubharmonics> subharmonics;
    void validateHarmonic(short harmonic) const;
    double transposeMultiplier = 1;
    
};



#endif /* HarmonicFlipper_hpp */
