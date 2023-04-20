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

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

// defined by compiler flags for flexibility
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                 OPT_OS_NONE
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG              0
#endif

// CFG_TUSB_DEBUG is defined by compiler in DEBUG build
// #define CFG_TUSB_DEBUG           0

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE    64
#endif

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

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN                                 TUD_AUDIO_MIC_ONE_CH_DESC_LEN
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT                                 1
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ                              64

#define CFG_TUD_AUDIO_ENABLE_EP_IN                                    1
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX                    2                                       // Driver gets this info from the descriptors - we define it here to use it to setup the descriptors and to do calculations with it below
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX                            1                                       // Driver gets this info from the descriptors - we define it here to use it to setup the descriptors and to do calculations with it below - be aware: for different number of channels you need another descriptor!

// EP and buffer sized
#define SAMPLES_PER_FRAME_NOMINAL                   (16000 / 1000)
#define SAMPLES_PER_FRAME_EXTRA                     (SAMPLES_PER_FRAME_NOMINAL + 20)

#define BYTES_PER_FRAME_NOMINAL                     (SAMPLES_PER_FRAME_NOMINAL * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)
#define BYTES_PER_FRAME_EXTRA                       (SAMPLES_PER_FRAME_EXTRA   * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)

// #define CFG_TUD_AUDIO_EPSIZE_IN                     BYTES_PER_FRAME_EXTRA
// #define CFG_TUD_AUDIO_TX_FIFO_SIZE                  BYTES_PER_FRAME_EXTRA
// #define CFG_TUD_AUDIO_EP_SZ_IN                      CFG_TUD_AUDIO_EPSIZE_IN
// #define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX           CFG_TUD_AUDIO_EP_SZ_IN                  // Maximum EP IN size for all AS alternate settings used
// #define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ        CFG_TUD_AUDIO_TX_FIFO_SIZE

// #define CFG_TUD_AUDIO_ENABLE_EP_IN                                    1
// #define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX                    2                                       // Driver gets this info from the descriptors - we define it here to use it to setup the descriptors and to do calculations with it below
// #define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX                            1                                       // Driver gets this info from the descriptors - we define it here to use it to setup the descriptors and to do calculations with it below - be aware: for different number of channels you need another descriptor!
#define CFG_TUD_AUDIO_EP_SZ_IN                                        SAMPLES_PER_FRAME_NOMINAL * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX      // 16 Samples (16 kHz) x 2 Bytes/Sample x 1 Channel
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX                             CFG_TUD_AUDIO_EP_SZ_IN                  // Maximum EP IN size for all AS alternate settings used
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ                          CFG_TUD_AUDIO_EP_SZ_IN + BYTES_PER_FRAME_EXTRA

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
