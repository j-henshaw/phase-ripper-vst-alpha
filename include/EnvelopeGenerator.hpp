#ifndef EnvelopeGenerator_hpp
#define EnvelopeGenerator_hpp

#include <vector>
#include <pair>

class EnvelopeGenerator {
public:

    EnvelopeGenerator();
    
    double nextGain();
    void noteOn(int velocity);
    void noteOff();
    
    void moveLoopL(int destination);
    void moveLoopR(int destination);
    void addStage();
    void removeStage();
    int getNumStages();
    void setStageEndpoint(int stage, pair<double> endpoint);
    void setStageSplinePoint(int stage, pair<double> spline);
    void setVelocitySensitivity(double sensitivity);
    
    void setSampleRate(int sample_rate);
    
    

private:
    
    std::vector<double> envelope;
    std::vector<double> stage_targets;
    std::vector<double> stage_splines;
    std::vector<double> stage_lengths;
    double sampleLength; //seconds
    int loopStart; //stage
    int loopEnd; //stage
    int numStages;
    double velocityToGainAmt;
    double velocityMultiplier;
    int currentPosition; //index
    
    double calcVelocityMultiplier(const int velocity);
    void updateLoopEndpoint(int& endpoint, int stage);
    void reconcileLoopMarkers();
    void redrawEnvelopeStage(int stage);
    
    

};


#endif /* EnvelopeGenerator_hpp */

