#ifndef GOERTZEL_H
#define GOERTZEL_H

#include <stdint.h>

/**
 * @brief Calculate the magnitude of a specific frequency component using the Goertzel algorithm
 * 
 * @param amplitude1 Pointer to store the magnitude of ADC1
 * @param amplitude2 Pointer to store the magnitude of ADC2
 * @param data Array of ADC samples to process (4bit padding + 12-bit R aligned ADC1 + 4bit padding + 12-bit R aligned ADC2)
 * @param data_size Number of samples in the array
 * @param sample_rate ADC sampling rate in Hz
 * @param target_frequency Frequency to detect (Hz)
 * @return int 0 on success, -1 on error
 */
int goertzel_magnitude(float *amplitude1, float *amplitude2, uint32_t *data, uint32_t data_size, float sample_rate,
    float target_frequency);

#endif // GOERTZEL_H
