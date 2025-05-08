#include "goertzel.h"
// #include <cmath>
#include <cmsis_gcc.h>

float goertzel_magnitude(uint32_t* data, uint32_t data_size, float sample_rate, float target_frequency) 
{
    float omega = 2.0f * M_PI * target_frequency / sample_rate;
    float sine = sin(omega);
    float cosine = cos(omega);
    float coeff = 2.0f * cosine;
    
    float q0 = 0.0f;
    float q1 = 0.0f;
    float q2 = 0.0f;
    
    // Process all samples
    for (uint32_t i = 0; i < data_size; i++) {
        q0 = coeff * q1 - q2 + (float)data[i];
        q2 = q1;
        q1 = q0;
    }
    
    // Calculate magnitude
    float real = q1 - q2 * cosine;
    float imag = q2 * sine;
    
    return sqrtf(real*real + imag*imag);
}
