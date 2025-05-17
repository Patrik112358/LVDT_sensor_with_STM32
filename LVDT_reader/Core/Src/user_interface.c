#include "user_interface.h"
#include <stdint.h>
#include <stdio.h>
#include "debugtools.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "ssd1306_tests.h"

#define SMALLEST_FONT Font_6x8
#define WIDTH_CHARS   (SSD1306_WIDTH / 6) // [21] 6 is the width of the smallest font
#define HEIGHT_CHARS  (SSD1306_HEIGHT / 8) // [8] 8 is the height of the smallest font
const char empty_row[WIDTH_CHARS + 1] = "                     "; // +1 for null terminator
// char row[LONGEST_POSSIBLE_ROW_LENGTH + 1]; // +1 for null terminator
// char screen[WIDTH_CHARS * HEIGHT_CHARS + 1] = // +1 for null terminator
//     //   *********************
// "                     "
// "prim freq   ##.### Hz"
// "sec1 amp     #.### V "
// "sec2 amp     #.### V "
// "ratio      ###.### - "
// "position     ###.# mm"
// "                     "
// "                     "
//     "\0";
//   *********************

typedef struct {
  char       row[WIDTH_CHARS + 1]; // +1 for null terminator
  const char original_row[WIDTH_CHARS + 1]; // +1 for null terminator
  const char fmt[10];
  char*      updatable_text_ptr;
} ScreenRowSpec_t;


// Define the X-macro for screen content rows in the requested order
// Format: X(row_text, format_string, data_type, field_name)
#define DISPLAYABLE_ITEMS(X)                                                                                           \
  /* =====================                                                                                          */ \
  X("primDrvFrq ###.###kHz", "% 7.3f", float, primary_drive_frequency)                                                 \
  X("secSampFrq ###.###kHz", "% 7.3f", float, secondary_sampling_frequency)                                            \
  X("sec1 amp   ###.### kV", "% 7.3f", float, sec1_amplitude)                                                          \
  X("sec2 amp   ###.### kV", "% 7.3f", float, sec2_amplitude)                                                          \
  X("ratio      ###.### - ", "%07.3f", float, ratio)                                                                   \
  X("position     ###.# mm", "%05.1f", double, position)                                                               \
  X("bufs skip'd ###### - ", "% 6u", uint32_t, n_halfbuffers_skipped)                                                  \
  /* =====================                                                                                          */

typedef enum {
#define CREATE_ENUM(row_text, fmt, type, name) DisplayableItemEnum_##name,
  DISPLAYABLE_ITEMS(CREATE_ENUM)
#undef CREATE_ENUM
      DisplayableItemEnum_CNT,
} DisplayableItemEnum_t;


typedef struct {
  ScreenRowSpec_t row_spec;
  union {
    float    val_float;
    double   val_double;
    uint32_t val_uint32_t;
    char     val[sizeof(double)];
  };
} DisplayableItem_t;


#define DEFINE_ROW_OF_TYPE(type)                                                                                       \
  typedef struct {                                                                                                     \
    ScreenRowSpec_t row_spec;                                                                                          \
    union {                                                                                                            \
      float    val_float;                                                                                              \
      double   val_double;                                                                                             \
      uint32_t val_uint32_t;                                                                                           \
      type     val;                                                                                                    \
    };                                                                                                                 \
  } DisplayableItem_##type##_t;

DEFINE_ROW_OF_TYPE(float)
BUILD_BUG_ON(sizeof(DisplayableItem_t) != sizeof(DisplayableItem_float_t));
DEFINE_ROW_OF_TYPE(double)
BUILD_BUG_ON(sizeof(DisplayableItem_t) != sizeof(DisplayableItem_double_t));
DEFINE_ROW_OF_TYPE(uint32_t)


// Define the struct fields using the X-macro
typedef union {
  struct {
#define DEFINE_ROW(row_text, fmt, type, name) DisplayableItem_##type##_t name;
    DISPLAYABLE_ITEMS(DEFINE_ROW)
#undef DEFINE_ROW
  };
  DisplayableItem_t arr[DisplayableItemEnum_CNT];
} AllDisplayableItems_t;

// Initialize the structure using the X-macro
AllDisplayableItems_t all_displayable_items = {
#define INIT_ROW(row_text, format, type, name)                                                                         \
  .name = { .row_spec = { .row = row_text, .original_row = row_text, .fmt = format }, .val = (type) - 0.0f },
  DISPLAYABLE_ITEMS(INIT_ROW)
#undef INIT_ROW
};

DisplayableItem_t* screen_rows[HEIGHT_CHARS] = {
  (DisplayableItem_t*)&all_displayable_items.primary_drive_frequency,
  (DisplayableItem_t*)&all_displayable_items.secondary_sampling_frequency,
  (DisplayableItem_t*)&all_displayable_items.sec1_amplitude,
  (DisplayableItem_t*)&all_displayable_items.sec2_amplitude,
  (DisplayableItem_t*)&all_displayable_items.ratio,
  (DisplayableItem_t*)&all_displayable_items.position,
  NULL,
  (DisplayableItem_t*)&all_displayable_items.n_halfbuffers_skipped,
};


UI_params_t ui_params = { 0 };

void UI_Init(void)
{
  for (unsigned item_idx = 0; item_idx < (sizeof(all_displayable_items.arr) / sizeof(all_displayable_items.arr[0]));
      item_idx++) {
    DisplayableItem_t* item = &all_displayable_items.arr[item_idx];
    // item->row_spec.updatable_text_ptr = item->row_spec.row;
    char** update_position_ptr = &item->row_spec.updatable_text_ptr;
    *update_position_ptr = item->row_spec.row;
    while (**update_position_ptr != '#') { // find the first '#' character
      (*update_position_ptr)++;
      if (**update_position_ptr == '\0' // don't run beyond string terminator
          || *update_position_ptr == (item->row_spec.row + sizeof((ScreenRowSpec_t){ 0 }.row)) // don't run OOB
      ) {
        break;
      }
    }
    if ('#' != **update_position_ptr) {
      *update_position_ptr = NULL; // no update position found
    }
  }
  ui_params.font = &Font_6x8;
  ui_params.row_length = SSD1306_WIDTH / ui_params.font->width;
  ssd1306_Init();
  // ssd1306_TestFonts1_screen_info_PRINT();
  UI_UpdateScreen();
}


// Update the screen contents based on the current state of the system
// This function should be called periodically to refresh the display
void UI_UpdateScreen(void)
{
  for (int row_idx = 0; row_idx < HEIGHT_CHARS; row_idx++) {
    DisplayableItem_t* row = screen_rows[row_idx];
    ssd1306_SetCursor(1, row_idx * ui_params.font->height);
    if (NULL == row) {
      ssd1306_WriteString(empty_row, *ui_params.font, White);
      continue;
    }
    ssd1306_WriteString(row->row_spec.row, *ui_params.font, White);
  }
  ssd1306_UpdateScreen();
}

void UI_UpdateDisplayableItems(void)
{
  // for (int row_idx = 0; row_idx < HEIGHT_CHARS; row_idx++) {
  //   DisplayableItem_t* row = screen_rows[row_idx];
  //   if (NULL == row) { continue; }
  //   // Update the row contents based on the current state of the system
  //   const unsigned max_chars = sizeof(row->row_spec.row) - (row->row_spec.updatable_text_ptr - row->row_spec.row);
  //   int            ret = snprintf(row->row_spec.updatable_text_ptr, max_chars, row->row_spec.fmt, row->val);
  //   if (ret < 0 || ret >= (int)max_chars) {
  //     WARN_PRINT("snprintf failed");
  //   } else {
  //     row->row_spec.updatable_text_ptr[ret] = ' ';
  //   }
  // }
  unsigned           max_chars = 0;
  int                ret = 0;
  DisplayableItem_t* row = NULL;

#define UPDATE_DISPLAYABLE_ITEMS(row_text, format_string, data_type, field_name)                                       \
  row = (DisplayableItem_t*)&all_displayable_items.field_name;                                                         \
  max_chars = sizeof(row->row_spec.row) - (row->row_spec.updatable_text_ptr - row->row_spec.row);                      \
  ret = snprintf(row->row_spec.updatable_text_ptr, max_chars, row->row_spec.fmt, row->val_##data_type);                \
  if (ret < 0 || ret >= (int)max_chars) {                                                                              \
    if (0) DEBUG_PRINT("snprintf failed when updating item `" #field_name "`\n");                                      \
  } else {                                                                                                             \
    row->row_spec.updatable_text_ptr[ret] =                                                                            \
        row->row_spec.original_row[row->row_spec.updatable_text_ptr - row->row_spec.row + ret];                        \
  }

  DISPLAYABLE_ITEMS(UPDATE_DISPLAYABLE_ITEMS)
#undef UPDATE_DISPLAYABLE_ITEMS
}

void UI_Update(void)
{
  UI_UpdateDisplayableItems();
  UI_UpdateScreen();
}


void UI_SetPrimaryDriveFrequency(float frequency)
{
  all_displayable_items.primary_drive_frequency.val = frequency / 1000.0f;
}

void UI_SetSecondarySamplingFrequency(float frequency)
{
  all_displayable_items.secondary_sampling_frequency.val = frequency / 1000.0f;
}

void UI_SetSec1Amplitude(float amplitude)
{
  all_displayable_items.sec1_amplitude.val = amplitude / 1000.0f;
}

void UI_SetSec2Amplitude(float amplitude)
{
  all_displayable_items.sec2_amplitude.val = amplitude / 1000.0f;
}

void UI_SetPosition(float position)
{
  all_displayable_items.position.val = position;
}

void UI_SetRatio(float ratio)
{
  all_displayable_items.ratio.val = ratio;
}

void UI_SetNHalfbuffersSkipped(uint32_t n_halfbuffers)
{
  all_displayable_items.n_halfbuffers_skipped.val = n_halfbuffers;
}
