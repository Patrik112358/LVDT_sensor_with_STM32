#include "user_interface.h"
#include <stdint.h>
#include <stdio.h>
#include "debugtools.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "ssd1306_tests.h"
#include "usart.h"

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
  X("secSampFrq####.###kHz", "% 7.3f", float, secondary_sampling_frequency)                                            \
  X("sec1 amp    #.#### V ", "% 6.4f", float, sec1_amplitude)                                                          \
  X("sec2 amp    #.#### V ", "% 6.4f", float, sec2_amplitude)                                                          \
  X("ratio     ####.### - ", "%+ 8.3f", float, ratio)                                                                  \
  X("position     ###.# mm", "%05.1f", float, position)                                                                \
  X("bufs skip'd ###### - ", "% 6u", uint32_t, n_halfbuffers_skipped)                                                  \
  X("bufs skip'd###.### /s", "% 7.3f", float, n_halfbuffers_skipped_per_second)                                        \
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
  (DisplayableItem_t*)&all_displayable_items.sec1_amplitude, (DisplayableItem_t*)&all_displayable_items.sec2_amplitude,
  (DisplayableItem_t*)&all_displayable_items.ratio,
  // (DisplayableItem_t*)&all_displayable_items.position,
  (DisplayableItem_t*)&all_displayable_items.n_halfbuffers_skipped_per_second,
  // (DisplayableItem_t*)&all_displayable_items.n_halfbuffers_skipped,
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
  const unsigned last_row_top_y = 6 * SMALLEST_FONT.height;
  const unsigned last_row_bottom_y = SSD1306_HEIGHT - 1;
  const unsigned last_row_midline_start_y = last_row_top_y + (last_row_bottom_y - last_row_top_y) / 2;
  const unsigned last_row_midline_end_y = last_row_midline_start_y + 1;
  const unsigned last_row_left_x = 0;
  const unsigned last_row_right_x = SSD1306_WIDTH - 1;
  ssd1306_FillRectangle(last_row_left_x, last_row_top_y, last_row_right_x, last_row_bottom_y, Black);
  ssd1306_FillRectangle(last_row_left_x, last_row_midline_start_y, last_row_right_x, last_row_midline_end_y, White);
  const unsigned slider_width = 12;
  const unsigned slider_half_height = SMALLEST_FONT.height - 1;
  // const unsigned slider_height = 4;
  const unsigned slider_top_y = last_row_midline_start_y - slider_half_height;
  const unsigned slider_bottom_y = last_row_midline_end_y + slider_half_height;

  const float position = all_displayable_items.ratio.val_float;
  const float min_position = -8.0f;
  const float max_position = 8.0f;
  const float position_range = max_position - min_position;
  const float position_normalized = (position - min_position) / position_range;
  const float slider_horizontal_position_middle = position_normalized * SSD1306_WIDTH;
  int         slider_left_x = (int)(slider_horizontal_position_middle - (float)slider_width / 2) / 1;
  int         slider_right_x = (int)(slider_horizontal_position_middle + (float)slider_width / 2) / 1;
  if (slider_left_x < 0) { slider_left_x = 0; }
  if (slider_right_x >= SSD1306_WIDTH) { slider_right_x = SSD1306_WIDTH - 1; }

  ssd1306_FillRectangle(slider_left_x, slider_top_y, slider_right_x, slider_bottom_y, White);

  int slider_half_width = slider_width / 2;
  int slider_protrude_left_x = slider_left_x - slider_half_width;
  int slider_protrude_right_x = slider_right_x + slider_half_width;
  if (slider_protrude_left_x < 0) { slider_protrude_left_x = 0; }
  if (slider_protrude_right_x >= SSD1306_WIDTH) { slider_protrude_right_x = SSD1306_WIDTH - 1; }
  const unsigned slider_quarter_height = slider_half_height / 2;
  const unsigned slider_protrude_top_y = last_row_midline_start_y - slider_quarter_height;
  const unsigned slider_protrude_bottom_y = last_row_midline_end_y + slider_quarter_height;
  if (slider_protrude_left_x < slider_left_x) {
    ssd1306_FillRectangle(slider_protrude_left_x, slider_protrude_top_y, slider_left_x, slider_protrude_bottom_y,
        White);
  }
  if (slider_protrude_right_x > slider_right_x) {
    ssd1306_FillRectangle(slider_right_x, slider_protrude_top_y, slider_protrude_right_x, slider_protrude_bottom_y,
        White);
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

void UI_SendToUART(void)
{
#define FMT_STR "%.6f,%.6f,%.6f\n"
#define VARS                                                                                                           \
  all_displayable_items.sec1_amplitude.val, all_displayable_items.sec2_amplitude.val, all_displayable_items.ratio.val
  int ret = snprintf(NULL, 0, FMT_STR, VARS);
  if (ret < 0) {
    WARN_PRINT("snprintf failed");
    return;
  }
  char buffer[ret + 1];
  ret = snprintf(buffer, sizeof(buffer), FMT_STR, VARS);
  if (ret < 0) {
    WARN_PRINT("snprintf failed");
    return;
  }
#undef FMT_STR
#undef VARS
  HAL_UART_Transmit(&huart1, (uint8_t*)buffer, ret, HAL_MAX_DELAY);
}

void UI_Update(void)
{
  UI_UpdateDisplayableItems();
  UI_UpdateScreen();
  UI_SendToUART();
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
  all_displayable_items.sec1_amplitude.val = amplitude / 1000000.0f;
  // all_displayable_items.sec1_amplitude.val = amplitude;
}

void UI_SetSec2Amplitude(float amplitude)
{
  all_displayable_items.sec2_amplitude.val = amplitude / 1000000.0f;
  // all_displayable_items.sec2_amplitude.val = amplitude;
}

void UI_SetPosition(float position)
{
  all_displayable_items.position.val = position;
}

void UI_SetRatio(float ratio)
{
  all_displayable_items.ratio.val = ratio * 10;
}

void UI_SetNHalfbuffersSkipped(uint32_t n_halfbuffers)
{
  all_displayable_items.n_halfbuffers_skipped.val = n_halfbuffers;
}

void UI_SetNHalfbuffersSkippedPerSecond(float n_halfbuffers_per_second)
{
  all_displayable_items.n_halfbuffers_skipped_per_second.val = n_halfbuffers_per_second;
}
