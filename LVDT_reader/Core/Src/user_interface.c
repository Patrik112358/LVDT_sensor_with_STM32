#include "user_interface.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/_types.h>
#include "debugtools.h"
#include "flash.h"
#include "main.h"
#include "onebutton.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "usart.h"

#define SMALLEST_FONT Font_6x8
#define WIDTH_CHARS   (SSD1306_WIDTH / 6) // [21] 6 is the width of the smallest font
#define HEIGHT_CHARS  (SSD1306_HEIGHT / 8) // [8] 8 is the height of the smallest font

const UIWireframe_PathbarItemcounter_t ui_wireframe_pathbar_itemcounter = { 
  .path_bar = {  { 0, 0 }, { 95, 8 }, },
  .page_counter = { { 98, 0 }, { 127, 8 } },
  // Divider line
  .lines = {
      { { 96, 0 }, { 96, 8 } }, // Vertical divider between path and counter
      { { 0, 9 }, { 127, 9 } }, // Horizontal divider
      SSD1306_SENTINEL_LINE_INITIALIZER,
  },
 };

const UIWireframe_CarouselScreen_t ui_wireframe_carousel_screen = { 
  .item_left = { { 0, 10 }, { 40, 33 } },
  .item_middle = { { 43, 10 }, { 84, 33 } },
  .item_right = { { 87, 10 }, { 127, 33 } },
  .item_detail = { { 0, 36 }, { 127, 64 } },
  .highlight_rect_left = {{0,10},{40,34}},
  .highlight_rect_middle = {{41,10},{84,34}},
  .highlight_rect_right = {{86,10},{127,34}},
  // Divider lines
  .lines = {
      { { 0, 35 }, { 127, 35 } }, // hl, detail separator
      { { 41, 10 }, { 41, 34 } }, // vl, left
      { { 85, 10 }, { 85, 34 } }, // vl, right
      SSD1306_SENTINEL_LINE_INITIALIZER,
  },
 };

extern const UI_MenuItem_t mi_root;
UI_Menu_t                  ui_menu = { .root = &mi_root, .current_item = &mi_root };

extern const UI_MenuItem_t mi_run;
extern const UI_MenuItem_t mi_length_coefficient_calibration;
extern const UI_MenuItem_t mi_offset_zeroing;
extern const UI_MenuItem_t mi_value_averaging;
extern const UI_MenuItem_t mi_UART_value_reporting;

const UI_MenuItem_t mi_root = {
  .short_label_menu_path = "Menu",
  .update_screen_fn = UIMenu_DisplayFolderCarousel,
  .item_carousel_label = "# MENU #",
  .detailed_description = "ROOT of MENU",
  .detailed_description_fn = NULL,
  .parent_item = NULL,
  .first_child_item = &mi_run,
  // .highlighted_child_item = &mi_run,
  .next_sibling = NULL,
  .short_btn_press = UIMenu_Carousel_NextItem,
  .long_btn_press = UIMenu_Carousel_SelectItem,
};

const UI_MenuItem_t mi_run = {
  /** TODO this one should have smthing like `.procedure_to_run` instead of
  `.update_screen_fn`, `.short_btn_press`, `.long_btn_press`, as it does not 
  have its own screen (handled by menu) nor menu-like btn interactions. Or maybe find a way
  to make it into a menu-type of thing.
   */
  .short_label_menu_path = "Run",
  .update_screen_fn = NULL,
  .item_carousel_label = "#  >  #\n# RUN #\n#  >  #",
  .detailed_description = "Start the operation\nof LVDT sensor",
  .detailed_description_fn = NULL,
  .parent_item = &mi_root,
  .first_child_item = NULL,
  // .highlighted_child_item = NULL,
  .next_sibling = &mi_length_coefficient_calibration,
  .short_btn_press = NULL,
  .long_btn_press = NULL,
};

const UI_MenuItem_t mi_length_coefficient_calibration = {
  .short_label_menu_path = "LenCoefCal",
  .update_screen_fn = NULL,
  .item_carousel_label = "Length\ncoeff.\ncalib.\n",
  .detailed_description = "Calibrate the length\ncoefficient or load\nthe default value.",
  .detailed_description_fn = NULL,
  .parent_item = &mi_root,
  .first_child_item = NULL,
  // .highlighted_child_item = NULL,
  .next_sibling = &mi_offset_zeroing,
  .short_btn_press = NULL,
  .long_btn_press = NULL,
};

const UI_MenuItem_t mi_offset_zeroing = {
  .short_label_menu_path = "OffsetZero",
  .update_screen_fn = NULL,
  .item_carousel_label = "Offset\nzeroing\n",
  .detailed_description = "You can calibrate\noffset zeroing and\nturn it on/off.",
  .detailed_description_fn = NULL,
  .parent_item = &mi_root,
  .first_child_item = NULL,
  // .highlighted_child_item = NULL,
  .next_sibling = &mi_value_averaging,
  .short_btn_press = NULL,
  .long_btn_press = NULL,
};

const UI_MenuItem_t mi_value_averaging = {
  .short_label_menu_path = "ValAvg",
  .update_screen_fn = NULL,
  .item_carousel_label = "Value\naverag-\ning\n",
  .detailed_description = "Select how many meas-\nurements to average\nbefore updating value.",
  .detailed_description_fn = NULL,
  .parent_item = &mi_root,
  .first_child_item = NULL,
  // .highlighted_child_item = NULL,
  .next_sibling = &mi_UART_value_reporting,
  .short_btn_press = NULL,
  .long_btn_press = NULL,
};

const UI_MenuItem_t mi_UART_value_reporting = {
  .short_label_menu_path = "UARTreport",
  .update_screen_fn = NULL,
  .item_carousel_label = "UART\nvalue\nsending",
  .detailed_description = "How often to send\nvalues through UART.",
  .detailed_description_fn = NULL,
  .parent_item = &mi_root,
  .first_child_item = NULL,
  // .highlighted_child_item = NULL,
  .next_sibling = NULL,
  .short_btn_press = NULL,
  .long_btn_press = NULL,
};

void UIMenu_Run(void)
{
  bool should_update_screen = true;
  while (1) {
    // TODO: rewrite to always process all processable ob_events (for quicker interactions)
    if (should_update_screen) {
      ssd1306_Fill(Black);
      ui_menu.current_item->update_screen_fn(ui_menu.current_item);
      ssd1306_UpdateScreen();
      should_update_screen = false;
    }
    DEBUG_PRINT("UI_Menu rendered\n");
    ButtonEvent_t button_event;
    int           ob_get_event_ret = Onebutton_GetEvent(ui_menu.onebutton_handle, &button_event);
    if (0 == ob_get_event_ret) {
      should_update_screen = true;
      if (button_event.action == BUTTON_ACTION_SINGLE_CLICK) {
        if (ui_menu.current_item->short_btn_press) { ui_menu.current_item->short_btn_press(); }
      } else if (button_event.action == BUTTON_ACTION_LONG_PRESS) {
        if (ui_menu.current_item->long_btn_press) { ui_menu.current_item->long_btn_press(); }
      }
    } else if (-1 == ob_get_event_ret) {
      // No event to process
    }
    int ob_process_ret = Onebutton_Process(ui_menu.onebutton_handle);
  }
}


void UIMenu_Carousel_NextItem(void)
{
  if (NULL == ui_menu.current_item || NULL == ui_menu.current_item->first_child_item) {
    WARN_PRINT("No current item or no child items to navigate in carousel.\n");
    return;
  }
  ui_menu.highlighted_child_item = ui_menu.highlighted_child_item->next_sibling;
  if (NULL == ui_menu.highlighted_child_item) {
    INFO_PRINT("No highlighted child item in carousel, setting to first child.\n");
    ui_menu.highlighted_child_item = ui_menu.current_item->first_child_item;
    return;
  }
}

void UIMenu_Carousel_SelectItem(void)
{
  if (NULL == ui_menu.current_item || NULL == ui_menu.current_item->first_child_item) {
    WARN_PRINT("No current item or no child items to select in carousel.\n");
    return;
  }
  if (NULL == ui_menu.highlighted_child_item) {
    WARN_PRINT("No highlighted child item in carousel to select.\n");
    return;
  }
  ui_menu.current_item = ui_menu.highlighted_child_item;
  ui_menu.highlighted_child_item = ui_menu.current_item->first_child_item;
  if (NULL == ui_menu.highlighted_child_item) {
    INFO_PRINT("Selected item has no child items, highlighted_child_item set to NULL.\n");
  }
}

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
  X("position    ##.### mm", "%+ 6.3f", float, position)                                                               \
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

float sec1_amplitude_buffer[MAX_AVERAGING_BUFFER_SIZE] = { 0 };
float sec2_amplitude_buffer[MAX_AVERAGING_BUFFER_SIZE] = { 0 };
float ratio_buffer[MAX_AVERAGING_BUFFER_SIZE] = { 0 };


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
  // (DisplayableItem_t*)&all_displayable_items.ratio,
  (DisplayableItem_t*)&all_displayable_items.position,
  (DisplayableItem_t*)&all_displayable_items.n_halfbuffers_skipped_per_second,
  // (DisplayableItem_t*)&all_displayable_items.n_halfbuffers_skipped,
};


UI_params_t ui_params = { 0 };
UI_state_t  ui_state = {
   .length_coefficient = 1.0f / 0.4312f,
   .averaging_length = 1,
};

void UI_Init(OnebuttonHandler_t* onebutton_handle)
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
  // UI_UpdateScreen();
  if (0 != UI_LoadStateParamsFromFlash(&ui_state)) { INFO_PRINT("UI state params load from flash failed.\n"); }
  ui_menu.onebutton_handle = onebutton_handle;
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
  ui_state.averaging_current_idx++;
  ui_state.averaging_current_idx %= ui_state.averaging_length;
  if (ui_state.averaging_current_idx == 0) {
    UI_UpdateScreen();
    if (ubButtonPress == 1) {
      UI_SendToUART();
      ubButtonPress = 0;
    }
  }
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
  amplitude = amplitude / 1000000.0f;
  sec1_amplitude_buffer[ui_state.averaging_current_idx] = amplitude;
  if (ui_state.averaging_length - 1 <= ui_state.averaging_current_idx) {
    float sum = 0;
    for (unsigned i = 0; i < ui_state.averaging_length; i++) { sum += sec1_amplitude_buffer[i]; }
    amplitude = sum / ui_state.averaging_length;
    memset(sec1_amplitude_buffer, 0, ui_state.averaging_length * sizeof(sec1_amplitude_buffer[0]));
    all_displayable_items.sec1_amplitude.val = amplitude;
  }
}

void UI_SetSec2Amplitude(float amplitude)
{
  amplitude = amplitude / 1000000.0f;
  sec2_amplitude_buffer[ui_state.averaging_current_idx] = amplitude;
  if (ui_state.averaging_length - 1 <= ui_state.averaging_current_idx) {
    float sum = 0;
    for (unsigned i = 0; i < ui_state.averaging_length; i++) { sum += sec2_amplitude_buffer[i]; }
    amplitude = sum / ui_state.averaging_length;
    memset(sec2_amplitude_buffer, 0, ui_state.averaging_length * sizeof(sec2_amplitude_buffer[0]));
    all_displayable_items.sec2_amplitude.val = amplitude;
  }
}

void UI_SetPosition(float position)
{
  all_displayable_items.position.val = position;
}

void UI_SetRatio(float ratio)
{
  ratio = ratio * 10.0f;
  ratio_buffer[ui_state.averaging_current_idx] = ratio;
  if (ui_state.averaging_length - 1 <= ui_state.averaging_current_idx) {
    float sum = 0;
    for (unsigned i = 0; i < ui_state.averaging_length; i++) { sum += ratio_buffer[i]; }
    ratio = sum / ui_state.averaging_length;
    memset(ratio_buffer, 0, ui_state.averaging_length * sizeof(ratio_buffer[0]));
    all_displayable_items.ratio.val = ratio;
  }
  float position = ratio * ui_state.length_coefficient;
  UI_SetPosition(position);
}

void UI_SetNHalfbuffersSkipped(uint32_t n_halfbuffers)
{
  all_displayable_items.n_halfbuffers_skipped.val = n_halfbuffers;
}

void UI_SetNHalfbuffersSkippedPerSecond(float n_halfbuffers_per_second)
{
  all_displayable_items.n_halfbuffers_skipped_per_second.val = n_halfbuffers_per_second;
}


int UI_SaveStateParamsToFlash(UI_state_t* ui_state)
{
  FLASH_store_object_v1_t flash_params = {
    .object_version = 1,
    .length_coefficient = ui_state->length_coefficient,
    .averaging_buffer_size = ui_state->averaging_length,
    .uart_send_period_n_measurements = ui_state->uart_send_frequency_N,
  };
  return write_flash_object_LVDT_reader_params_v1(&flash_params);
}

int UI_LoadStateParamsFromFlash(UI_state_t* ui_state)
{
  FLASH_store_object_v1_t flash_params = { .averaging_buffer_size = 1, .uart_send_period_n_measurements = 0 };
  int                     ret = read_flash_object_LVDT_reader_params_v1(&flash_params);
  if (ret < 0) {
    DEBUG_PRINT("Failed to read UI state params from flash\n");
    return ret;
  }
  // ui_state->length_coefficient = flash_params.length_coefficient;
  // ui_state->averaging_length = flash_params.averaging_buffer_size;
  // ui_state->uart_send_frequency_N = flash_params.uart_send_period_n_measurements;
  *ui_state = (UI_state_t){
    // averaging
    .averaging_length = flash_params.averaging_buffer_size,
    .averaging_current_idx = 0, // Reset the current index
    // UART reporting
    .uart_send_frequency_N = flash_params.uart_send_period_n_measurements,
    .uart_send_frequency_current_idx = 0, // Reset the UART send frequency index
    // measurement interpretation
    .length_coefficient = flash_params.length_coefficient,
  };
  return 0;
}

void UIMenu_DisplayPathbarItemcount(void)
{
  char path[18] = { 0 };
  memset(path, ' ', sizeof(path) - 1);
  const UI_MenuItem_t* current_item = ui_menu.current_item;
  { // Build & print path
    int path_pos = sizeof(path) - 1;
    while (current_item != NULL) {
      int current_path_fragment_length = strlen(current_item->short_label_menu_path) + 1;
      path_pos -= current_path_fragment_length;
      snprintf(&path[path_pos], current_path_fragment_length, "%s>", current_item->short_label_menu_path);
      current_item = current_item->parent_item;
    }
    path[sizeof(path) - 2] = '\0';
    char* path_start = path;
    while (*path_start == ' ') { path_start++; }
    ssd1306_WriteMultilineStringIntoBox_Box(path_start, ui_wireframe_pathbar_itemcounter.path_bar, SMALLEST_FONT, White,
        true);
  }
  { // Count and show item counter
    unsigned             total_items = 0;
    unsigned             highlighted_item_index = 0; // 1-based index
    const UI_MenuItem_t* current_item = ui_menu.current_item->first_child_item;
    while (current_item != NULL) {
      total_items++;
      if (current_item == ui_menu.highlighted_child_item) { highlighted_item_index = total_items; }
      current_item = current_item->next_sibling;
    }
    char item_counter[6] = { 0 };
    memset(item_counter, ' ', sizeof(item_counter) - 1);
    if (0 != highlighted_item_index) {
      snprintf(item_counter, 4, "%2d/", highlighted_item_index <= 99 ? highlighted_item_index : 99);
    } else {
      snprintf(item_counter, 4, "--/");
    }
    if (0 != total_items) {
      snprintf(&item_counter[3], 3, "%2d", total_items <= 99 ? total_items : 99);
    } else {
      snprintf(item_counter, 3, "--");
    }
    ssd1306_WriteMultilineStringIntoBox_Box(item_counter, ui_wireframe_pathbar_itemcounter.page_counter, SMALLEST_FONT,
        White, true);
  }
  ssd1306_MultipleLines(ui_wireframe_pathbar_itemcounter.lines, White);
  return;
}

void UIMenu_DisplayFolderCarousel(const UI_MenuItem_t* self)
{
  UIMenu_DisplayPathbarItemcount();
  if (NULL == ui_menu.current_item || NULL == ui_menu.current_item->first_child_item) {
    WARN_PRINT("No current item or no child items to display in carousel.\n");
    // return;
  }
  unsigned total_items = 1;
  unsigned highlighted_item_index = 1;

  if (NULL == ui_menu.highlighted_child_item) {
    ui_menu.highlighted_child_item = ui_menu.current_item->first_child_item;
  }
  const UI_MenuItem_t* const highlighted_item = ui_menu.highlighted_child_item;

  const UI_MenuItem_t* left = NULL;
  const UI_MenuItem_t* middle = NULL;
  const UI_MenuItem_t* right = NULL;

  right = ui_menu.current_item->first_child_item;
  while (highlighted_item != right && NULL != right) {
    left = middle;
    middle = right;
    right = right->next_sibling;
    total_items++;
    highlighted_item_index++;
  }

  { // find total number of items
    const UI_MenuItem_t* last_child = right;
    while (last_child->next_sibling != NULL) {
      last_child = last_child->next_sibling;
      total_items++;
    }
  }

  if (highlighted_item_index < total_items) {
    for (unsigned i = 0; i < 1 + (1 == highlighted_item_index); i++) {
      left = middle;
      middle = right;
      right = right != NULL ? right->next_sibling : NULL;
    }
  }

  // Display the carousel of child items
  // UIMenu_DisplaySingleItemCarouselLabel(0, 11, left ? left->item_carousel_label : NULL);
  // UIMenu_DisplaySingleItemCarouselLabel(43, 11, middle ? middle->item_carousel_label : NULL);
  // UIMenu_DisplaySingleItemCarouselLabel(87, 11, right ? right->item_carousel_label : NULL);
  ssd1306_WriteMultilineStringIntoBox_Box(left ? left->item_carousel_label : NULL,
      ui_wireframe_carousel_screen.item_left, SMALLEST_FONT, White, true);
  ssd1306_WriteMultilineStringIntoBox_Box(middle ? middle->item_carousel_label : NULL,
      ui_wireframe_carousel_screen.item_middle, SMALLEST_FONT, White, true);
  ssd1306_WriteMultilineStringIntoBox_Box(right ? right->item_carousel_label : NULL,
      ui_wireframe_carousel_screen.item_right, SMALLEST_FONT, White, true);

  if (highlighted_item == left) { ssd1306_InvertRectangle_Box(ui_wireframe_carousel_screen.item_left); }
  if (highlighted_item == middle) { ssd1306_InvertRectangle_Box(ui_wireframe_carousel_screen.item_middle); }
  if (highlighted_item == right) { ssd1306_InvertRectangle_Box(ui_wireframe_carousel_screen.highlight_rect_right); }
  ssd1306_MultipleLines(ui_wireframe_carousel_screen.lines, White);
}

void UIMenu_DisplaySingleItemCarouselLabel(unsigned x, unsigned y, const char* str)
{
  if (NULL == str) { return; }
  // Display the multiline string at the specified coordinates
  // char line[8] = { 0 };
  // for (unsigned row = 0; row < 3 && NULL != str && '\0' != *str; row++) {
  //   for (unsigned column = 0; column < 7 && '\0' != *str && '\n' != *str; column++) {
  //     line[column] = *str;
  //     str++;
  //   }
  //   while ('\0' != *str && '\n' != *str) { str++; } // Consume rest of the line (beyond 7ch limit)
  //   if ('\n' == *str) { str++; } // Skip the newline character
  //   line[7] = '\0'; // Null-terminate the string
  //   ssd1306_SetCursor(x, y + 8 * row);
  //   ssd1306_WriteString(line, Font_6x8, White);
  // }
}
