#ifndef GOERTZEL_H
#define GOERTZEL_H

#include <stdint.h>

/**
 * @brief Calculate the magnitude of a specific frequency component using the Goertzel algorithm
 * 
 * @param data Array of ADC samples to process
 * @param data_size Number of samples in the array
 * @param sample_rate ADC sampling rate in Hz
 * @param target_frequency Frequency to detect (Hz)
 * @return float Magnitude of the target frequency component
 */
float goertzel_magnitude(uint32_t* data, uint32_t data_size, float sample_rate, float target_frequency);

#endif // GOERTZEL_H
