#ifndef ONEBUTTON_H
#define ONEBUTTON_H

#include <stdint.h>
#include "stm32g474xx.h"

typedef struct OnebuttonHandler OnebuttonHandler_t;
#define ONEBUTTON_PROCESSED_EVENT_BUFFER_SIZE 10

typedef struct {
  uint32_t      debounce_delay_ms; // Delay in milliseconds to debounce the button
  uint32_t      long_press_min_duration_ms; // Delay in milliseconds to detect a long press
  uint32_t      double_click_max_delay_ms; // Delay in milliseconds to detect a double click
  GPIO_TypeDef *GPIOx; // GPIO port where the button is connected
  uint16_t      GPIO_Pin; // GPIO pin where the button is connected
} OnebuttonHandlerInitParams_t; // Initialization parameters for the Onebutton handler

typedef enum {
  BUTTON_ACTION_NONE = 0,
  BUTTON_ACTION_SINGLE_CLICK,
  BUTTON_ACTION_DOUBLE_CLICK,
  BUTTON_ACTION_LONG_PRESS,
} ButtonAction_t;

typedef struct {
  uint32_t       timestamp; // Timestamp of the event from HAL_GetTick()
  ButtonAction_t action; // Action of the button
} ButtonEvent_t; // Button event structure, records single click, double click and long press


// Function prototypes for button handling

/** Constructs a new Onebutton event processor.
 * @brief  Initializes the Onebutton handler with the given parameters.
 * @param  handler Pointer to the Onebutton handler structure.
 * @param  params Pointer to the initialization parameters.
 * @retval 0 on success, -1 on error.
 * @note   This function DOES NOT INITIALIZE the GPIO pin for the button
*/
int Onebutton_Init(OnebuttonHandler_t *handler, OnebuttonHandlerInitParams_t *params);

/**
  * @brief  Gets the default initialization parameters for the Onebutton handler.
  * @retval OnebuttonHandlerInitParams_t structure filled with default values.
  */
OnebuttonHandlerInitParams_t Onebutton_GetDefaultInitParams(void);

/**
  * @brief  Processes all button state changes into button events.
  * @param  handler Pointer to the Onebutton handler structure.
  * @retval 0 on success, -1 on "nothing to process".
  * @note   This function should be called periodically to process button state changes.
  *         It will update the internal state of the button and generate events based on the button
  *         state changes. Ideally, you should first try to consume out all events by using Onebutton_GetEvent(...) first,
  *         in order to avoid filling the internal event queue.
 */
int Onebutton_Process(OnebuttonHandler_t *handler);

/**
  * @brief  Gets the next button event from the event queue.
  * @param  handler Pointer to the Onebutton handler structure.
  * @param  event Pointer to the ButtonEvent_t structure to fill with the event data.
  * @retval 0 on success, -1 on empty queue.
  * @note   This function will remove the event from the queue.
 */
int Onebutton_GetEvent(OnebuttonHandler_t *handler, ButtonEvent_t *event);

/**
  * @brief  Checks if there are any events in the event queue.
  * @param  handler Pointer to the Onebutton handler structure.
  * @retval 1 if there are events, 0 if the queue is empty.
 */
int Onebutton_HasEvents(OnebuttonHandler_t *handler);

/**
  * @brief  Interrupt handler for the Onebutton handler.
  * @param  handler Pointer to the Onebutton handler structure.
  * @note   This function should be called in the GPIO interrupt handler for the button pin.
  *         It will handle the button press and release events and update the internal state of the button.
  *         It is expected that the GPIO pin is configured to trigger an interrupt on both rising and falling edges.
  *         This function determines whether the button is pressed or released using HAL_GPIO_ReadPin(),
  *         however it does NOT investigate reason for the interrupt -- it is the responsibility of the caller
  *         to ensure that the reason for the interrupt is a button press or release for the button configured in the `handler`.
 */
void Onebutton_InterruptHandler(OnebuttonHandler_t *handler);


// Rest of this header should've been opaque, but without dynamic memoory it'd quickly become unnecessary mess
#define ONEBUTTON_RAW_EVENT_BUFFER_SIZE (4 * ONEBUTTON_PROCESSED_EVENT_BUFFER_SIZE)

typedef struct {
  uint8_t       read_idx; // Head of the queue
  uint8_t       count; // Number of entries
  uint8_t       size; // Maximum number of entries of the queue
  ButtonEvent_t events[ONEBUTTON_PROCESSED_EVENT_BUFFER_SIZE]; // Array of button events
} ButtonEventQueue_t; // Circular queue for button events

typedef enum {
  RAW_BUTTON_ACTION_NONE = 0,
  RAW_BUTTON_ACTION_PRESS,
  RAW_BUTTON_ACTION_RELEASE,
} RawButtonAction_t;

typedef struct {
  uint32_t          timestamp; // Timestamp of the event from HAL_GetTick()
  RawButtonAction_t action; // Action of the button
} RawButtonEvent_t; // Raw button event structure, records all presses and releases of the button

typedef struct {
  uint32_t         last_valid_raw_event_time; // Timestamp of the last not-ignored raw event
  uint8_t          read_idx; // Head of the queue
  uint8_t          count; // Number of entries
  uint8_t          size; // Maximum number of entries of the queue
  RawButtonEvent_t events[ONEBUTTON_RAW_EVENT_BUFFER_SIZE]; // Array of raw button events
} RawButtonEventQueue_t; // Circular queue for raw button events

struct OnebuttonHandler {
  OnebuttonHandlerInitParams_t params; // Initialization parameters for the Onebutton handler
  GPIO_TypeDef                *GPIOx; // GPIO port where the button is connected
  uint16_t                     GPIO_Pin; // GPIO pin where the button is connected
  uint32_t                     last_press_time; // Timestamp of the last button press
  uint32_t                     last_release_time; // Timestamp of the last button release
  ButtonEventQueue_t           processed_events; // Queue for processed button events
  RawButtonEventQueue_t        raw_events; // Queue for raw button events
};

#endif // ONEBUTTON_H
