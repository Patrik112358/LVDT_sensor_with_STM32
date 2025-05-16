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
// Sets the 1st secondary windings amplitude
void UI_SetSec1Amplitude(float amplitude);
// Sets the 2nd secondary windings amplitude
void UI_SetSec2Amplitude(float amplitude);
// Sets the position calculated from ratio
void UI_SetPosition(float position);
// Sets the ratio of sec1 and sec2 amplitudes
void UI_SetRatio(float ratio);

// Waits for user button press
UI_params_t UI_GetParams(void);


#endif /* USER_INTERFACE_H */
