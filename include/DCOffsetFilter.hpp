#ifndef DCOffsetFilter_hpp
#define DCOffsetFilter_hpp

#include <q/utility/ring_buffer.hpp>
#include <cmath>


class DCOffsetFilter{
public:
    DCOffsetFilter();
    double filterNewSample(const double new_sample);
    void updateSampleRate(const int sample_rate);

private:
    double running_sum = 0.0;
    const int TARGET_FREQ = 20; //Hz
    cycfi::q::ring_buffer<double> buffer;

};

#endif /* DCOffsetFilter_hpp */
