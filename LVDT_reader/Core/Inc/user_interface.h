#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdint.h>
#include <sys/_types.h>
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

typedef struct UI_MenuItem UI_MenuItem_t;
struct UI_MenuItem {
  // Menu will build path from this, only last 16chars of path are visible
  const char* const short_label_menu_path;
  // Self-presentation function of this item.
  // Should redraw screen from (0,10) to (127,63).
  void (*update_screen_fn)(const UI_MenuItem_t* self);

  // Used by parent.
  // Item's label that fits into 7x3 chars rectangle.
  const char* const item_carousel_label;
  // Used by parent.
  // Item's longer description that fits into 21x3 chars rectangle.
  // If null, return of `detailed_description_fn` will be used.
  const char* const detailed_description;
  // Used by parent.
  // Function to provide `detailed_description` or draw graphics.
  // Has to directly call to screen drawing functions to update screen buffer
  // (but should not call for screen refresh).
  // Should take care to only update the region between (0,36) and (128,64) pixels.
  // The preceding line (pixels (0,35) to (128,35)) is filled in.
  void (*detailed_description_fn)(void);

  // Parent item
  const UI_MenuItem_t* const parent_item;
  // Child items (submenus, actions, settable-items)
  const UI_MenuItem_t* const first_child_item;
  // Next sibling item
  const UI_MenuItem_t* const next_sibling;

  // Gets called on short button press
  void (*short_btn_press)(void);
  // Gets called on long button press
  void (*long_btn_press)(void);
};

typedef struct {
  SSD1306_BOX path_bar;
  SSD1306_BOX page_counter;
  // SSD1306_LINE lines[];
  SSD1306_LINE_LIST lines;
} UIWireframe_PathbarItemcounter_t;
extern const UIWireframe_PathbarItemcounter_t ui_wireframe_pathbar_itemcounter;

typedef struct {
  SSD1306_BOX  item_left;
  SSD1306_BOX  item_middle;
  SSD1306_BOX  item_right;
  SSD1306_BOX  item_detail;
  SSD1306_BOX  highlight_rect_left;
  SSD1306_BOX  highlight_rect_middle;
  SSD1306_BOX  highlight_rect_right;
  SSD1306_LINE lines[];
} UIWireframe_CarouselScreen_t;
extern const UIWireframe_CarouselScreen_t ui_wireframe_carousel_screen;

typedef struct {
  const UI_MenuItem_t* const root;
  const UI_MenuItem_t*       current_item;
  const UI_MenuItem_t*       highlighted_child_item; // Selected child item of `current_item`
  OnebuttonHandler_t*        onebutton_handle;
} UI_Menu_t;

extern UI_Menu_t ui_menu;

void UIMenu_Carousel_NextItem(void);
void UIMenu_Carousel_SelectItem(void);
void UIMenu_Run(void);

void UIMenu_DisplayPathbarItemcount(void);
// Used as `UI_MenuItem.update_screen_fn`.
// Displays child items as a carousel with movable selection.
void UIMenu_DisplayFolderCarousel(const UI_MenuItem_t* self);
// Displays a single item label (7x3 characters) in the carousel.
void UIMenu_DisplaySingleItemCarouselLabel(unsigned x, unsigned y, const char* str);

// extern ScreenContents_t screen_contents;

void UI_Init(OnebuttonHandler_t* handler);
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
