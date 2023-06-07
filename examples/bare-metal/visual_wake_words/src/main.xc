// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xccompat.h>
#include <xscope.h>

extern "C" {
void app_init(chanend flash_server);
void app_data(unsigned char *data, size_t size);
void flash_server(chanend c);
}


unsafe {
  void process_xscope(chanend c_xscope_data_in) {
    int bytes_read = 0;
    unsigned char buffer[256];

    xscope_connect_data_from_host(c_xscope_data_in);
    xscope_mode_lossless();
    while (1) {
      select {
        case xscope_data_from_host(c_xscope_data_in, buffer, bytes_read):
          app_data(buffer, bytes_read);
          break;
      }
    }
  }
}

int main(void) {
  chan c_xscope_data_in;
  chan c_flash_server;

  par {
    xscope_host_data(c_xscope_data_in);
    on tile[0] : {
      app_init(c_flash_server);
      process_xscope(c_xscope_data_in);
    }
    on tile[0] : flash_server(c_flash_server);
  }

  return 0;
}
