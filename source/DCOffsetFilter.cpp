#include "../include/DCOffsetFilter.hpp"

DCOffsetFilter::DCOffsetFilter() : buffer(44100/TARGET_FREQ) {
    
}

//Takes a single sample, and returns how much this sample should be
//reduced by, based on previously added samples
double DCOffsetFilter::filterNewSample(double new_sample){
    //Add the new sample to the bufer, and return the updated average
    running_sum += new_sample;
    running_sum -= buffer.back();
    buffer.push(new_sample);
    
    return running_sum / buffer.size();
}

//Ensures the filter is holding the correct number of samples to operate
//properly. This function can cause temporary audio glitches, but that
//should be expected whenever the sample rate changes
void DCOffsetFilter::updateSampleRate(const int sample_rate){
    //If the internal buffer is not of the required size,
    //discard the old buffer and create a new one of the required size
    const int REQD_SIZE = std::ceil(sample_rate / TARGET_FREQ);
    
    if (buffer.size() != REQD_SIZE){
        buffer = cycfi::q::ring_buffer<double> (REQD_SIZE);
        running_sum = 0.0;
    }
}
