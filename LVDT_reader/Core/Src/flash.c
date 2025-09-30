#include "flash.h"
#include <stdint.h>
#include <stdio.h>
#include "stm32g474xx.h"
#include "stm32g4xx.h"
// #include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_flash.h"
#include "stm32g4xx_it.h"

typedef uint64_t flash_datatype;
#define DATA_SIZE sizeof(flash_datatype)

uint32_t flash_objects_start_address = FLASH_BASE + 0x10000; // 64kB offset

void store_flash_memory(uint32_t memory_address, uint8_t *data, uint16_t data_length)
{
  uint8_t                double_word_data[DATA_SIZE];
  FLASH_EraseInitTypeDef flash_erase_struct = { 0 };
  HAL_FLASH_Unlock();
  // defining the members of a struct
  flash_erase_struct.TypeErase = FLASH_TYPEERASE_PAGES;
  // defining an onset number page to be erased
  flash_erase_struct.Page = (memory_address - FLASH_BASE) / FLASH_PAGE_SIZE;
  // number of pages to remove
  flash_erase_struct.NbPages = 1 + data_length / FLASH_PAGE_SIZE;
  // identify the flash bank
  if (memory_address >= FLASH_BASE + FLASH_BANK_SIZE && memory_address < FLASH_BASE + 2 * FLASH_BANK_SIZE) {
    flash_erase_struct.Banks = FLASH_BANK_2;
  } else if (memory_address >= FLASH_BASE && memory_address < FLASH_BASE + FLASH_BANK_SIZE) {
    flash_erase_struct.Banks = FLASH_BANK_1;
  } else {
    printf("illegal memory address \n");
    UsageFault_Handler();
  }
  uint32_t error_status = 0;

  // erase the pages, this step is mandatory
  HAL_FLASHEx_Erase(&flash_erase_struct, &error_status);
  int i = 0;
  // using while loop, convey all data to the flash memory
  while (i <= data_length) {
    double_word_data[i % DATA_SIZE] = data[i];
    i++;
    if (i % DATA_SIZE == 0) {
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, memory_address + i - DATA_SIZE, *((uint64_t *)double_word_data));
    }
  }
  // convey data if something left
  if (i % DATA_SIZE != 0) {
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, memory_address + i - i % DATA_SIZE,
        *((flash_datatype *)double_word_data));
  }
  // lock the memory
  HAL_FLASH_Lock();
}

void read_flash_memory(uint32_t memory_address, uint8_t *data, uint16_t data_length)
{
  for (int i = 0; i < data_length; i++) { *(data + i) = (*(uint8_t *)(memory_address + i)); }
}


int read_flash_object_LVDT_reader_params_v1(FLASH_store_object_v1_t *params)
{
  FLASH_store_object_v1_t flash_params = { 0 };
  read_flash_memory(flash_objects_start_address, (uint8_t *)&flash_params, sizeof(flash_params));
  if (flash_params.object_version != 1) {
    return -1; // Invalid version
  }
  params->object_version = flash_params.object_version;
  params->length_coefficient = flash_params.length_coefficient;
  params->averaging_buffer_size = flash_params.averaging_buffer_size;
  params->uart_send_period_n_measurements = flash_params.uart_send_period_n_measurements;
  return 0;
}

int write_flash_object_LVDT_reader_params_v1(const FLASH_store_object_v1_t *params)
{
  FLASH_store_object_v1_t flash_params = {
    .object_version = params->object_version,
    .length_coefficient = params->length_coefficient,
    .averaging_buffer_size = params->averaging_buffer_size,
    .uart_send_period_n_measurements = params->uart_send_period_n_measurements,
  };
  store_flash_memory(flash_objects_start_address, (uint8_t *)&flash_params, sizeof(flash_params));
  return 0;
}
