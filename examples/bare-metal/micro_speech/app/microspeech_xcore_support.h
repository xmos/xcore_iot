// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1.

#ifndef MICROSPEECH_XCORE_SUPPORT_H_
#define MICROSPEECH_XCORE_SUPPORT_H_

#ifdef __cplusplus
extern "C" {
#endif


#if __XC__
void mic_decoupler(streaming chanend c_ds_output, chanend c_gpio);
void tile0(chanend c_gpio);
#else
#include <xcore/chanend.h>

void mic_decoupler(chanend_t c_ds_output, chanend_t c_gpio);
void tile0(chanend_t c_gpio);

#include "fifo.h"

typedef struct microspeech_device {
    fifo_t* sample_fifo;
    int32_t* sample_buffer;
} microspeech_device_t;

microspeech_device_t* get_microspeech_device();

void increment_timestamp(int32_t increment);
int32_t get_led_status();
#endif /* __XC__ */

#ifdef __cplusplus
}
#endif

#endif /* MICROSPEECH_XCORE_SUPPORT_H_ */
