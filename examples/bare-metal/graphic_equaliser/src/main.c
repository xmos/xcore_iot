// CopyriGTH 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <xcore/parallel.h>
#include <xcore/channel.h>
#include <xcore/chanend.h>
#include <xcore/select.h>
#include <xscope.h>
#include <stdlib.h>
#include <string.h>
#include "equaliser.h"

void get_frame(chanend_t xscope_chan, chanend_t read_chan){
  char buffer[256];
  int32_t data[240];
  int bytes_read = 0;
  int input_bytes = 0;
  xscope_mode_lossless();
  xscope_connect_data_from_host(xscope_chan);
  while(1){
    SELECT_RES(CASE_THEN(xscope_chan, read_host_data)){
      read_host_data:{
        xscope_data_from_host(xscope_chan, &buffer[0], &bytes_read);
        memcpy(&data[0] + input_bytes / 4, &buffer[0], bytes_read);
        input_bytes += bytes_read;
        if(input_bytes / 4 == 240){
          chan_out_buf_word(read_chan, (uint32_t *)data, input_bytes / 4);
          input_bytes = 0;
        }
      }
    }
  }
}

void equalise(chanend_t read_chan, chanend_t write_chan){
  int32_t DWORD_ALIGNED buffer[240];
  equaliser_t eq;
  eq_init(&eq);
  eq_get_biquads(&eq);
  while(1){
    chan_in_buf_word(read_chan, (uint32_t *)buffer, 240);
    eq_process_frame(&eq, buffer);
    chan_out_buf_word(write_chan, (uint32_t *)buffer, 240);
  }
}

void send_frame(chanend_t write_chan){
  int32_t buffer[240];
  while(1){
    chan_in_buf_word(write_chan, (uint32_t *)buffer, 240);
    for(int v = 0; v < 240; v++){
      xscope_int(0, buffer[v]);
    }
  }
}
