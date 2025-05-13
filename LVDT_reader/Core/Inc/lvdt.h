#ifndef LVDT_H
#define LVDT_H

#include <stdint.h>
#include <stm32g4xx_ll_dac.h>
#include "main.h"

#define ADC_BUFFER_SIZE 512
#define DAC_BUFFER_SIZE 32

extern const uint16_t DAC_buffer_sine[DAC_BUFFER_SIZE];
extern uint32_t       ADC_buffer[ADC_BUFFER_SIZE];

/* Definitions of environment analog values */
/* Value of analog reference voltage (Vref+), connected to analog voltage   */
/* supply Vdda (unit: mV).                                                  */
#define VDDA_APPLI                         ((uint32_t)3300)

/* Definitions of data related to this example */
/* Full-scale digital value with a resolution of 12 bits (voltage range     */
/* determined by analog voltage references Vref+ and Vref-,                 */
/* refer to reference manual).                                              */
#define DIGITAL_SCALE_12BITS               (__LL_DAC_DIGITAL_SCALE(LL_DAC_RESOLUTION_12B))

/* Definitions of waveform generation values */
/* Waveform generation: parameters of waveform */
/* Waveform amplitude (unit: mV) */
#define WAVEFORM_AMPLITUDE                 (VDDA_APPLI)
/* Waveform amplitude (unit: Hz) */
#define WAVEFORM_FREQUENCY                 ((uint32_t)1000)
/* Size of array containing DAC waveform samples */
#define WAVEFORM_SAMPLES_SIZE              (sizeof(DAC_buffer_sine) / sizeof(typeof(DAC_buffer_sine[0])))

/* Waveform generation: parameters of timer (used as DAC trigger) */
/* Timer frequency (unit: Hz). With a timer 16 bits and time base           */
/* freq min 1Hz, range is min=1Hz, max=32kHz.                               */
#define WAVEFORM_TIMER_FREQUENCY           (WAVEFORM_FREQUENCY * WAVEFORM_SAMPLES_SIZE)
/* Timer minimum frequency (unit: Hz), used to calculate frequency range.   */
/* With a timer 16 bits, maximum frequency will be 32000 times this value.  */
#define WAVEFORM_TIMER_FREQUENCY_RANGE_MIN ((uint32_t)1)
/* Timer prescaler maximum value (0xFFFF for a timer 16 bits) */
#define WAVEFORM_TIMER_PRESCALER_MAX_VALUE ((uint32_t)0xFFFF - 1)



void LVDT_Init(void);
void LVDT_Start(void);
void LVDT_Stop(void);

void LVDT_Start_DAC(void);
void LVDT_Stop_DAC(void);

#endif /* LVDT_H */
