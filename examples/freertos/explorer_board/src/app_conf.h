// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile Communication Configuration */
#define appconfI2C_MASTER_RPC_PORT 10
#define appconfI2C_MASTER_RPC_PRIORITY (configMAX_PRIORITIES/2)

#define appconfGPIO_T0_RPC_PORT 11
#define appconfGPIO_T1_RPC_PORT 12
#define appconfGPIO_RPC_PRIORITY (configMAX_PRIORITIES/2)

/* I/O and interrupt cores for Tile 0 */
#define appconfI2C_IO_CORE                      3 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfI2C_INTERRUPT_CORE               0 /* Must be kept off I/O cores. */

/* I/O and interrupt cores for Tile 1 */
#define appconfPDM_MIC_IO_CORE                  1 /* Must be kept off I/O cores. Must be kept off core 0 with the RTOS tick ISR */
#define appconfI2S_IO_CORE                      2 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfPDM_MIC_INTERRUPT_CORE           3 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */
#define appconfI2S_INTERRUPT_CORE               4 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */

/* Audio Pipeline Configuration */
#define appconfAUDIO_CLOCK_FREQUENCY            MIC_ARRAY_CONFIG_MCLK_FREQ
#define appconfPDM_CLOCK_FREQUENCY              MIC_ARRAY_CONFIG_PDM_FREQ
#define appconfPIPELINE_AUDIO_SAMPLE_RATE       16000
#define appconfAUDIO_PIPELINE_STAGE_ONE_GAIN    256
#define appconfAUDIO_FRAME_LENGTH            	MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME
#define appconfPRINT_AUDIO_FRAME_POWER          0


/* GPIO Configuration */
#define appconfGPIO_VOLUME_RAPID_FIRE_MS       	100

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )
#define appconfAUDIO_PIPELINE_TASK_PRIORITY    	( configMAX_PRIORITIES - 4 )
#define appconfGPIO_TASK_PRIORITY              	( configMAX_PRIORITIES - 2 )
#define appconfFILESYSTEM_DEMO_TASK_PRIORITY    ( configMAX_PRIORITIES - 2 )
#define appconfMEM_ANALYSIS_TASK_PRIORITY		( configMAX_PRIORITIES - 1 )
#define appconfSPI_MASTER_TASK_PRIORITY		    ( configMAX_PRIORITIES - 1 )
#define appconfQSPI_FLASH_TASK_PRIORITY		    ( configMAX_PRIORITIES - 1 )

#endif /* APP_CONF_H_ */
