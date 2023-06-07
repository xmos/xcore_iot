// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <stdint.h>
#include <stdio.h>

#include <xcore/assert.h>
#include <xcore/channel.h>
#include <xcore/channel_transaction.h>

#include "qspi_flash_fast_read.h"

// CLK_DIVIDE should be set to:
//    3 for XK_VOICE_L71
//    4 for XCORE_AI_EXPLORER
#define CLK_DIVIDE                    (4)
#define READ_CHUNK_SIZE_BYTES         (1024)
#define READ_CHUNK_SIZE_WORDS         (READ_CHUNK_SIZE_BYTES/4)

#ifndef MODEL_DATA_PARTITION_OFFSET
#define MODEL_DATA_PARTITION_OFFSET   0x20
#endif

#define MIN(X, Y)                     (((X) < (Y)) ? (X) : (Y))

typedef enum flash_server_command {
  FLASH_READ_PARAMETERS = 0, ///< Read a set of parameters.
  FLASH_READ_MODEL = 1, ///< Read a whole model.
  FLASH_READ_OPERATORS = 2, ///< Read the binary for an operator - future extension
  FLASH_READ_XIP = 3, ///< Read code to execute-in-place through L2 cache - future extension
  FLASH_SERVER_QUIT = 4,
} flash_server_command_t;

qspi_fast_flash_read_ctx_t qspi_fast_flash_read_ctx;
qspi_fast_flash_read_ctx_t *ctx = &qspi_fast_flash_read_ctx;


void flash_setup() {
    qspi_flash_fast_read_init(ctx,
        XS1_CLKBLK_1,
        XS1_PORT_1B,
        XS1_PORT_1C,
        XS1_PORT_4B,
        qspi_fast_flash_read_transfer_raw,
        CLK_DIVIDE
    );
    
    qspi_flash_fast_read_setup_resources(ctx);

    uint32_t addr = 0x00000000;
    uint32_t scratch_buf[QFFR_DEFAULT_CAL_PATTERN_BUF_SIZE_WORDS];

    if (qspi_flash_fast_read_calibrate(ctx, addr, qspi_flash_fast_read_pattern_expect_default, scratch_buf, QFFR_DEFAULT_CAL_PATTERN_BUF_SIZE_WORDS) != 0) {
        printf("Fast flash calibration failed\n"); 
    }
}

void flash_teardown() {
  xassert(ctx);
  qspi_flash_fast_read_shutdown(ctx);
}

void flash_server(unsigned chan) {

  int flash_server_alive = 1;
  int __attribute__((aligned(8))) buf[READ_CHUNK_SIZE_WORDS];

  flash_setup();

  while (flash_server_alive) {
      int src, next_src;
      size_t size, next_size, bytes_read;
      flash_server_command_t cmd;

      // get next command
      cmd = chan_in_word(chan);

      switch (cmd) {
        case FLASH_READ_PARAMETERS:
          // get the src address
          src = chan_in_word(chan);
          src = src + MODEL_DATA_PARTITION_OFFSET;
          
          // get the read size (in bytes)
          size = chan_in_word(chan);

          bytes_read = 0;
          next_src = src;
          while (bytes_read < size) {
            // compute the size of the next read
            next_size = MIN(size-bytes_read, READ_CHUNK_SIZE_BYTES);
            // perform read
            qspi_flash_fast_read(ctx, next_src, (uint8_t *)buf, next_size);
            // output buffer read
            for (int i=0; i<next_size/4; i++) {
              chanend_out_word(chan, buf[i]);
            }
            // update counters, need to assume qspi_flash_fast_read read next_size bytes
            bytes_read += next_size; 
            next_src += next_size;
          }
          chanend_out_end_token(chan);

          break;
        case FLASH_SERVER_QUIT:
          flash_server_alive = 0;
          break;
        case FLASH_READ_MODEL:
        case FLASH_READ_OPERATORS:
        case FLASH_READ_XIP:
        default:
          printf("ERROR: flash server command %d not supported\n", cmd);
          xassert(0);
      }
  }
   
  flash_teardown();
}