#include "onebutton.h"
#include <stdint.h>
#include "debugtools.h"
#include "stm32g4xx_hal.h"

#define ONEBUTTON_RAW_EVENT_BUFFER_SIZE (4 * ONEBUTTON_PROCESSED_EVENT_BUFFER_SIZE)

typedef struct {
  ButtonEvent_t events[ONEBUTTON_PROCESSED_EVENT_BUFFER_SIZE]; // Array of button events
  uint8_t       head; // Head of the queue
  uint8_t       tail; // Tail of the queue
  uint8_t       size; // Max size of the queue
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
  uint8_t          head; // Head of the queue
  uint8_t          tail; // Tail of the queue
  uint8_t          size; // Max size of the queue
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

int Onebutton_Init(OnebuttonHandler_t *handler, OnebuttonHandlerInitParams_t *params)
{
  if (!handler || !params) {
    return -1; // Invalid parameters
  }
  handler->params = *params;
  handler->GPIOx = params->GPIOx;
  handler->GPIO_Pin = params->GPIO_Pin;
  handler->last_press_time = 0;
  handler->last_release_time = 0;

  // Initialize the processed events queue
  handler->processed_events.head = 0;
  handler->processed_events.tail = 0;
  handler->processed_events.size = ONEBUTTON_PROCESSED_EVENT_BUFFER_SIZE;

  // Initialize the raw events queue
  handler->raw_events.head = 0;
  handler->raw_events.tail = 0;
  handler->raw_events.size = ONEBUTTON_RAW_EVENT_BUFFER_SIZE;

  return 0;
}

OnebuttonHandlerInitParams_t Onebutton_GetDefaultInitParams(void)
{
  OnebuttonHandlerInitParams_t params = {
    .debounce_delay_ms = 50,
    .long_press_min_duration_ms = 1000,
    .double_click_max_delay_ms = 500,
    .GPIOx = NULL,
    .GPIO_Pin = 0,
  };
  return params;
}

// Processes all processeable raw button events and pushes them to the processed queue.
int ProcessAllRawButtonEvents(OnebuttonHandler_t *handler);

// Pops the next processed button event from the queue. Returns 0 on success, -1 if the queue is empty.
int ButtonEventQueue_Pop(ButtonEventQueue_t *queue, ButtonEvent_t *event);

// Pushes a new processed button event to the queue. Returns 0 on success, -1 if the queue is full.
int ButtonEventQueue_Push(ButtonEventQueue_t *queue, ButtonEvent_t event);

static inline int ButtonEventQueue_IsEmpty(ButtonEventQueue_t *queue)
{
  return (queue->head == queue->tail);
}

static inline int ButtonEventQueue_IsFull(ButtonEventQueue_t *queue)
{
  return ((queue->head + 1) % queue->size == queue->tail);
}

// Pops the next raw button event from the queue. Returns 0 on success, -1 if the queue is empty.
int ButtonRawPressQueue_Pop(RawButtonEventQueue_t *queue, RawButtonEvent_t *event);

// Peeks the next raw button event from the queue without removing it. Returns 0 on success, -1 if the queue is empty.
int ButtonRawPressQueue_Peek(RawButtonEventQueue_t *queue, RawButtonEvent_t *event);

// Pushes a new raw button event to the queue. Returns 0 on success, -1 if the queue is full.
int ButtonRawPressQueue_Push(RawButtonEventQueue_t *queue, RawButtonEvent_t event, uint8_t debounce_delay_ms);

static inline int ButtonRawPressQueue_IsEmpty(RawButtonEventQueue_t *queue)
{
  return (queue->head == queue->tail);
}

static inline int ButtonRawPressQueue_IsFull(RawButtonEventQueue_t *queue)
{
  return ((queue->head + 1) % ONEBUTTON_RAW_EVENT_BUFFER_SIZE == queue->tail);
}

void Onebutton_InterruptHandler(OnebuttonHandler_t *handler)
{
  uint32_t         current_time = HAL_GetTick();
  GPIO_PinState    state = HAL_GPIO_ReadPin(handler->GPIOx, handler->GPIO_Pin);
  RawButtonEvent_t raw_event = { .timestamp = current_time, .action = RAW_BUTTON_ACTION_NONE };
  if (GPIO_PIN_SET == state) {
    raw_event.action = RAW_BUTTON_ACTION_PRESS;
  } else if (GPIO_PIN_RESET == state) {
    raw_event.action = RAW_BUTTON_ACTION_RELEASE;
  }
  ButtonRawPressQueue_Push(&handler->raw_events, raw_event, handler->params.debounce_delay_ms);
  return;
}

int Onebutton_HasEvents(OnebuttonHandler_t *handler)
{
  return !ButtonEventQueue_IsEmpty(&handler->processed_events);
}

int Onebutton_GetEvent(OnebuttonHandler_t *handler, ButtonEvent_t *event)
{
  // if (ButtonEventQueue_IsEmpty(&handler->processed_events)) {
  //   return -1; // No events in the queue
  // }
  return ButtonEventQueue_Pop(&handler->processed_events, event);
}

int Onebutton_Process(OnebuttonHandler_t *handler)
{
  if (ButtonRawPressQueue_IsEmpty(&handler->raw_events)) {
    return -1; // No raw events to process
  }

  // Process all raw button events and push them to the processed queue
  return ProcessAllRawButtonEvents(handler);
}

int ProcessOneRawButtonEvent(OnebuttonHandler_t *handler, ButtonEvent_t *processed_event)
{
  if (1 == ButtonRawPressQueue_IsEmpty(&handler->raw_events)) {
    return -1; // No raw events to process
  }
  RawButtonEvent_t raw_events_for_this_action[4] = { 0 };
  uint8_t          raw_event_count = 0;
  *processed_event = (ButtonEvent_t){ 0 };

  for (int idx = 0; idx < 2; idx++) {
    do {
      if (-1 == ButtonRawPressQueue_Pop(&handler->raw_events, &raw_events_for_this_action[idx])) {
        if (0 == idx) {
          return -1;
        } else {
          break;
        }
      }
    } while (
        (0 == idx ? RAW_BUTTON_ACTION_PRESS : RAW_BUTTON_ACTION_RELEASE) != raw_events_for_this_action[idx].action);
    raw_event_count++;
  }
  processed_event->timestamp = raw_events_for_this_action[0].timestamp;

  const uint8_t current_time = HAL_GetTick();

  // Determine processed event type
  if (current_time >= raw_events_for_this_action[0].timestamp + handler->params.long_press_min_duration_ms) {
    // Enough time for long press has passed
    processed_event->action = BUTTON_ACTION_LONG_PRESS;
  } else {
    RawButtonEvent_t raw_event_peek = { 0 };
    if (0 != ButtonRawPressQueue_Peek(&handler->raw_events, &raw_event_peek) // No next raw event
        || raw_event_peek.action != RAW_BUTTON_ACTION_PRESS // Next event is not a press
        || raw_event_peek.timestamp
               > raw_events_for_this_action[1].timestamp
                     + handler->params.double_click_max_delay_ms // Next event is too far in the future
    ) {
      processed_event->action = BUTTON_ACTION_SINGLE_CLICK;
    } else {
      for (int idx = 2; idx < 4; idx++) {
        do {
          if (0 != ButtonRawPressQueue_Pop(&handler->raw_events, &raw_events_for_this_action[idx]) && 2 == idx) {
            processed_event->action = BUTTON_ACTION_SINGLE_CLICK;
            WARN_PRINT("Unexpected fail while popping raw event for double click processing.\n");
            return 0;
          }
        } while (
            (2 == idx ? RAW_BUTTON_ACTION_PRESS : RAW_BUTTON_ACTION_RELEASE) != raw_events_for_this_action[idx].action);
        raw_event_count++;
      }
      processed_event->action = BUTTON_ACTION_DOUBLE_CLICK;
      // if (0 != ButtonRawPressQueue_Pop(&handler->raw_events, &raw_events_for_this_action[2])) {
      //   WARN_PRINT("Unexpected fail while popping raw event for double click processing.\n");
      // }
    }
  }

  return 0;
}

int ProcessAllRawButtonEvents(OnebuttonHandler_t *handler)
{
  ButtonEvent_t processed_event = { 0 };
  int           ret = 0;

  while (0 == (ret = ProcessOneRawButtonEvent(handler, &processed_event))) {
    if (0 != ButtonEventQueue_Push(&handler->processed_events, processed_event)) {
      WARN_PRINT("Failed to push processed event to the queue.\n");
      return -1; // Failed to push processed event to the queue
    }
  }

  if (-1 == ret) {
    return -1; // No more raw events to process
  }

  return 0; // Successfully processed all raw events
}

int ButtonEventQueue_Pop(ButtonEventQueue_t *queue, ButtonEvent_t *event)
{
  if (ButtonEventQueue_IsEmpty(queue)) {
    return -1; // Queue is empty
  }

  *event = queue->events[queue->head];
  queue->head = (queue->head + 1) % queue->size;

  return 0;
}

int ButtonEventQueue_Push(ButtonEventQueue_t *queue, ButtonEvent_t event)
{
  if (ButtonEventQueue_IsFull(queue)) {
    return -1; // Queue is full
  }

  queue->events[queue->tail] = event;
  queue->tail = (queue->tail + 1) % queue->size;

  return 0;
}

int ButtonRawPressQueue_Pop(RawButtonEventQueue_t *queue, RawButtonEvent_t *event)
{
  if (ButtonRawPressQueue_IsEmpty(queue)) {
    return -1; // Queue is empty
  }

  *event = queue->events[queue->head];
  queue->head = (queue->head + 1) % queue->size;

  return 0;
}

int ButtonRawPressQueue_Peek(RawButtonEventQueue_t *queue, RawButtonEvent_t *event)
{
  if (ButtonRawPressQueue_IsEmpty(queue)) {
    return -1; // Queue is empty
  }

  *event = queue->events[queue->head];
  return 0; // Successfully peeked the event
}

int ButtonRawPressQueue_Push(RawButtonEventQueue_t *queue, RawButtonEvent_t event, uint8_t debounce_delay_ms)
{
  if (ButtonRawPressQueue_IsFull(queue)) {
    return -1; // Queue is full
  }

  if (0 < debounce_delay_ms) {
    RawButtonEvent_t last_event = queue->events[(queue->tail + queue->size - 1) % queue->size];
    if (event.timestamp < last_event.timestamp + debounce_delay_ms) {
      // Ignore the event if it is too close to the last event
      return 0;
    }
  }

  queue->events[queue->tail] = event;
  queue->tail = (queue->tail + 1) % queue->size;

  return 0;
}
