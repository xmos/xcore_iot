// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Hacks to build temporarily */
#include <stdint.h>
typedef struct {
    int32_t ch_a;
    int32_t ch_b;
} vtb_ch_pair_t;
#define VFE_PROC_FRAME_LENGTH_LOG2 9
#define VFE_FRAME_ADVANCE          240
#define VFE_CHANNEL_PAIRS          1
/* End of hacks */

#define appconfAUDIO_CLOCK_FREQUENCY        24576000
#define appconfPDM_CLOCK_FREQUENCY          3072000
#define appconfAUDIO_PIPELINE_SAMPLE_RATE   16000
#define appconfAUDIO_PIPELINE_FRAME_ADVANCE VFE_FRAME_ADVANCE

#define appconfUSB_AUDIO_PORT          0
#define appconfGPIO_T0_RPC_PORT        1
#define appconfGPIO_T1_RPC_PORT        2
#define appconfDEVICE_CONTROL_USB_PORT 3
#define appconfDEVICE_CONTROL_I2C_PORT 4
#define appconfSPI_AUDIO_PORT          5
#define appconfWW_SAMPLES_PORT         6


#ifndef appconfI2S_ENABLED
#if XVF3610_Q60A || XCOREAI_EXPLORER
#define appconfI2S_ENABLED         1
#else
#define appconfI2S_ENABLED         0
#endif
#endif

#ifndef appconfUSB_ENABLED
#define appconfUSB_ENABLED         1
#endif

#ifndef appconfWW_ENABLED
#define appconfWW_ENABLED          0
#endif

#ifndef appconfUSB_AUDIO_SAMPLE_RATE
#define appconfUSB_AUDIO_SAMPLE_RATE appconfAUDIO_PIPELINE_SAMPLE_RATE
//#define appconfUSB_AUDIO_SAMPLE_RATE 48000
#endif

#ifndef appconfI2C_CTRL_ENABLED
#if XCOREAI_EXPLORER
/*
 * When this is enabled on the XVF3610_Q60A board, the board
 * cannot function as an I2C master and will not configure the
 * DAC. In this case the DAC should be configured externally.
 * MCLK will also default to be external if this is set on
 * the XVF3610_Q60A board.
 */
#define appconfI2C_CTRL_ENABLED    1
#else
#define appconfI2C_CTRL_ENABLED    0
#endif
#endif

#ifndef appconfSPI_OUTPUT_ENABLED
#define appconfSPI_OUTPUT_ENABLED  0
#endif

#ifndef appconfI2S_AUDIO_SAMPLE_RATE
#define appconfI2S_AUDIO_SAMPLE_RATE appconfAUDIO_PIPELINE_SAMPLE_RATE
//#define appconfI2S_AUDIO_SAMPLE_RATE 48000
#endif

#ifndef appconfEXTERNAL_MCLK
#if XVF3610_Q60A && appconfI2C_CTRL_ENABLED
#define appconfEXTERNAL_MCLK       1
#else
#define appconfEXTERNAL_MCLK       0
#endif
#endif

/*
 * This option sends all 6 16 KHz channels (two channels of processed audio,
 * stereo reference audio, and stereo microphone audio) out over a single
 * 48 KHz I2S line.
 */
#ifndef appconfI2S_TDM_ENABLED
#define appconfI2S_TDM_ENABLED     0
#endif

#define appconfI2S_MODE_MASTER     0
#define appconfI2S_MODE_SLAVE      1
#ifndef appconfI2S_MODE
#define appconfI2S_MODE            appconfI2S_MODE_MASTER
#endif

#define appconfAEC_REF_USB         0
#define appconfAEC_REF_I2S         1
#ifndef appconfAEC_REF_DEFAULT
#define appconfAEC_REF_DEFAULT     appconfAEC_REF_I2S
#endif

#define appconfMIC_SRC_MICS        0
#define appconfMIC_SRC_USB         1
#ifndef appconfMIC_SRC_DEFAULT
#define appconfMIC_SRC_DEFAULT     appconfMIC_SRC_MICS
#endif

#define appconfUSB_AUDIO_RELEASE   0
#define appconfUSB_AUDIO_TESTING   1
#ifndef appconfUSB_AUDIO_MODE
#define appconfUSB_AUDIO_MODE      appconfUSB_AUDIO_TESTING
#endif

#define appconfSPI_AUDIO_RELEASE   0
#define appconfSPI_AUDIO_TESTING   1
#ifndef appconfSPI_AUDIO_MODE
#define appconfSPI_AUDIO_MODE      appconfSPI_AUDIO_TESTING
#endif

#include "app_conf_check.h"

/* Application control Config */
#define appconf_CONTROL_I2C_DEVICE_ADDR 0x42
#define appconf_CONTROL_SERVICER_COUNT  4

/* WW Config */
#define appconfWW_FRAMES_PER_INFERENCE          (160)

/* I/O and interrupt cores for Tile 0 */
/* Note, USB and SPI are mutually exclusive */
#define appconfXUD_IO_CORE                      1 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfSPI_IO_CORE                      1 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfUSB_INTERRUPT_CORE               2 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */
#define appconfUSB_SOF_INTERRUPT_CORE           3 /* Must be kept off I/O cores. Best kept off cores with other ISRs. */
#define appconfSPI_INTERRUPT_CORE               2 /* Must be kept off I/O cores. */

/* I/O and interrupt cores for Tile 1 */
#define appconfPDM_MIC_IO_CORE                  1 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfI2S_IO_CORE                      2 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfI2C_IO_CORE                      3 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfPDM_MIC_INTERRUPT_CORE           4 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */
#define appconfI2S_INTERRUPT_CORE               5 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */
#define appconfI2C_INTERRUPT_CORE               0 /* Must be kept off I/O cores. */

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY              (configMAX_PRIORITIES/2 + 5)
#define appconfGPIO_RPC_HOST_PRIORITY             (configMAX_PRIORITIES/2 + 2)
#define appconfGPIO_TASK_PRIORITY                 (configMAX_PRIORITIES/2 + 2)
#define appconfI2C_TASK_PRIORITY                  (configMAX_PRIORITIES/2 + 2)
#define appconfDEVICE_CONTROL_USB_CLIENT_PRIORITY (configMAX_PRIORITIES/2 + 2)
#define appconfDEVICE_CONTROL_I2C_CLIENT_PRIORITY (configMAX_PRIORITIES/2 + 2)
#define appconfUSB_MGR_TASK_PRIORITY              (configMAX_PRIORITIES/2 + 1)
#define appconfUSB_AUDIO_TASK_PRIORITY            (configMAX_PRIORITIES/2 + 1)
#define appconfSPI_TASK_PRIORITY                  (configMAX_PRIORITIES/2 + 1)
#define appconfQSPI_FLASH_TASK_PRIORITY           (configMAX_PRIORITIES/2 + 0)
#define appconfWW_TASK_PRIORITY                   (configMAX_PRIORITIES/2 - 1)

#endif /* APP_CONF_H_ */
