// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xscope.h>

extern "C" {
void app_main();
void app_data(void *data, size_t size);
}

unsafe {
  void process_xscope(chanend xscope_data_in) {
    int bytes_read = 0;
    unsigned char buffer[256];

    xscope_connect_data_from_host(xscope_data_in);
    xscope_mode_lossless();
    while (1) {
      select {
        case xscope_data_from_host(xscope_data_in, buffer, bytes_read):
          app_data(buffer, bytes_read);
          break;
      }
    }
  }
}

int main(void) {
  chan xscope_data_in;

  par {
    xscope_host_data(xscope_data_in);
    on tile[0] : {
      app_main();
      process_xscope(xscope_data_in);
    }
  }

  return 0;
}
