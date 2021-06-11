// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xcore_device_memory.h"

#include <stddef.h>
#include <string.h>

#include <xcore/assert.h>
#include <xcore/port.h>
#include <xcore/swmem_fill.h>
#include <xmos_flash.h>

#define WORDS_TO_BYTES(w) ((w) * sizeof(uint32_t))
#define BYTES_TO_WORDS(b) (((b) + sizeof(uint32_t) - 1) / sizeof(uint32_t))

#define WORD_TO_BYTE_ADDRESS(w) WORDS_TO_BYTES(w)
#define BYTE_TO_WORD_ADDRESS(b) ((b) / sizeof(uint32_t))

#ifdef USE_SWMEM

flash_ports_t flash_ports_0 = {PORT_SQI_CS, PORT_SQI_SCLK, PORT_SQI_SIO,
                               XS1_CLKBLK_5};

flash_clock_config_t flash_clock_config = {
    flash_clock_reference,
    0,
    1,
    flash_clock_input_edge_plusone,
    flash_port_pad_delay_1,
};

flash_qe_config_t flash_qe_config_0 = {flash_qe_location_status_reg_0,
                                       flash_qe_bit_6};

flash_handle_t flash_handle;

swmem_fill_t swmem_fill_handle;

void swmem_fill(fill_slot_t address) {
  swmem_fill_buffer_t buf;
  unsigned int *buf_ptr = (unsigned int *)buf;

  flash_read_quad(&flash_handle,
                  BYTE_TO_WORD_ADDRESS(address - (void *)XS1_SWMEM_BASE),
                  buf_ptr, SWMEM_FILL_SIZE_WORDS);

  swmem_fill_populate_from_buffer(swmem_fill_handle, address, buf);
}

void swmem_setup() {
  flash_connect(&flash_handle, &flash_ports_0, flash_clock_config,
                flash_qe_config_0);

  swmem_fill_handle = swmem_fill_get();
}

void swmem_teardown() {
  swmem_fill_free(swmem_fill_handle);
  flash_disconnect(&flash_handle);
}

void swmem_handler(void *ignored) {
  fill_slot_t address = 0;
  while (1) {
    address = swmem_fill_in_address(swmem_fill_handle);
    swmem_fill(address);
  }
}

size_t swmem_load(void *dest, const void *src, size_t size) {
  xassert(IS_SWMEM(src));

  flash_read_quad(&flash_handle,
                  BYTE_TO_WORD_ADDRESS(((uintptr_t)src - XS1_SWMEM_BASE)),
                  (unsigned int *)dest, BYTES_TO_WORDS(size));
  return size;
}

#endif /* USE_SWMEM */
