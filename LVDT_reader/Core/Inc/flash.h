#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

void store_flash_memory(uint32_t memory_address, uint8_t* data, uint16_t data_length);
void read_flash_memory(uint32_t memory_address, uint8_t* data, uint16_t data_length);

typedef struct {
  uint8_t object_version; /* version = 1 */
  float   length_coefficient; /* Coefficient for converting the raw position to mm */
  uint8_t averaging_buffer_size; /* Size of the averaging buffer */
  uint8_t uart_send_period_n_measurements; /* Number of measurements between UART sends */
} FLASH_store_object_v1_t;

int read_flash_object_LVDT_reader_params_v1(FLASH_store_object_v1_t* params);
int write_flash_object_LVDT_reader_params_v1(const FLASH_store_object_v1_t* params);


#endif // FLASH_H
