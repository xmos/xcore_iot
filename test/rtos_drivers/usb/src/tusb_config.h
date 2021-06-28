/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#include <stdint.h>
#include "app_conf.h"

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUSB_RHPORT0_MODE      (OPT_MODE_DEVICE | OPT_MODE_HIGH_SPEED)
#define CFG_TUSB_OS                OPT_OS_CUSTOM

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG             0
#endif

#define CFG_TUSB_MEM_ALIGN         __attribute__ ((aligned(4)))

#define CFG_TUSB_DEBUG_PRINTF     rtos_printf

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUD_EP_MAX            12
#define CFG_TUD_TASK_QUEUE_SZ     8
#define CFG_TUD_ENDPOINT0_SIZE    64

#define CFG_TUD_XCORE_INTERRUPT_CORE appconfUSB_INTERRUPT_CORE
#define CFG_TUD_XCORE_IO_CORE_MASK   (1 << appconfXUD_IO_CORE)

//------------- CLASS -------------//
#define CFG_TUD_CDC               0
#define CFG_TUD_MSC               0
#define CFG_TUD_HID               0
#define CFG_TUD_MIDI              0
#define CFG_TUD_AUDIO             1
#define CFG_TUD_VENDOR            0

//--------------------------------------------------------------------
// AUDIO CLASS DRIVER CONFIGURATION
//--------------------------------------------------------------------
extern const uint16_t tud_audio_desc_lengths[CFG_TUD_AUDIO];

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN                       tud_audio_desc_lengths[0]
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT                       1
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ                    64


#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX          2
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX          2

#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX                  2
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX                  2

// EP and buffer sizes
#define SAMPLES_PER_FRAME_NOMINAL                   (AUDIO_SAMPLE_RATE / 1000)

#define CFG_TUD_AUDIO_ENABLE_EP_IN                  1
#define BYTES_PER_TX_FRAME_NOMINAL                  (SAMPLES_PER_FRAME_NOMINAL * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ               BYTES_PER_TX_FRAME_NOMINAL
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX           (CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ)    // Maximum EP IN size for all AS alternate settings used
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ        CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ

#define CFG_TUD_AUDIO_ENABLE_EP_OUT                 1
#define BYTES_PER_RX_FRAME_NOMINAL                  (SAMPLES_PER_FRAME_NOMINAL * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX)
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ              BYTES_PER_RX_FRAME_NOMINAL
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX          (CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ + 2)   // Maximum EP IN size for all AS alternate settings used. Plus 2 for CRC
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ       CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ*3

#endif /* _TUSB_CONFIG_H_ */
