#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdint.h>
#include "main.h"
#include "ssd1306.h"

// typedef struct {
//     float primary_drive_frequency;
//     float sec1_amplitude;
//     float sec2_amplitude;
//     float position;
// } ScreenContents_t;
// typedef struct {} ScreenRow_t;
// typedef struct row_spec ScreenRowSpec_t;
// typedef struct {
//     struct {float val; ScreenRowSpec_t row_spec;} primary_drive_frequency;
//     float sec1_amplitude;
//     float sec2_amplitude;
//     float position;
// } ScreenContents_t;

#define MAX_AVERAGING_BUFFER_SIZE 32


typedef struct {
  uint8_t averaging_length; // Number of samples to average
  uint8_t averaging_current_idx; // Current index in the averaging buffer
  float   length_coefficient; // Coefficient to convert ratio to position
  uint8_t
      uart_send_frequency_N; // Configures N. Every Nth measurement will be sent through UART. 0 means measurement after button press;
  uint8_t uart_send_frequency_current_idx;
} UI_state_t;

typedef struct {
  const SSD1306_Font_t* font;
  uint8_t               row_length;
} UI_params_t;

// extern ScreenContents_t screen_contents;

void UI_Init(void);
// Prints prepared item buffers to screen
void UI_UpdateScreen(void);
// Updates the items' buffers with the latest values
void UI_UpdateDisplayableItems(void);
// Updates buffers, then prints them to screen
void UI_Update(void);

// Sets the primary drive frequency
void UI_SetPrimaryDriveFrequency(float frequency);
// Sets the secondary sampling frequency
void UI_SetSecondarySamplingFrequency(float frequency);
// Sets the 1st secondary windings amplitude
void UI_SetSec1Amplitude(float amplitude);
// Sets the 2nd secondary windings amplitude
void UI_SetSec2Amplitude(float amplitude);
// Sets the position calculated from ratio
void UI_SetPosition(float position);
// Sets the ratio of sec1 and sec2 amplitudes
void UI_SetRatio(float ratio);
// Sets the number of halfbuffers skipped (sampled but not processed)
void UI_SetNHalfbuffersSkipped(uint32_t n_halfbuffers);
// Sets the number of halfbuffers skipped per second
void UI_SetNHalfbuffersSkippedPerSecond(float n_halfbuffers_per_second);

int UI_SaveStateParamsToFlash(UI_state_t* ui_state);
int UI_LoadStateParamsFromFlash(UI_state_t* ui_state);

// Waits for user button press
UI_params_t UI_GetParams(void);


#endif /* USER_INTERFACE_H */
