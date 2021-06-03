// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile Communication Configuration */
#define INTERTILE_RPC_PORT 10
#define INTERTILE_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES-1)

#define INTERTILE_TEST_SYNC_PORT 11
#define INTERTILE_TEST_SYNC_TASK_PRIORITY (configMAX_PRIORITIES-1)

#define I2C_MASTER_RPC_PORT 12
#define I2C_MASTER_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

#define GPIO_RPC_PORT 13
#define GPIO_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

#define QSPI_FLASH_RPC_PORT 14
#define QSPI_FLASH_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

#define I2C_SLAVE_ISR_CORE   4
#define I2C_SLAVE_CORE_MASK  (1 << 1)
#define I2C_SLAVE_ADDR       0x7A

/* Audio Pipeline Configuration */
#define appconfAUDIO_CLOCK_FREQUENCY            24576000
#define appconfPDM_CLOCK_FREQUENCY              3072000
#define appconfPIPELINE_AUDIO_SAMPLE_RATE       48000
#define appconfAUDIO_PIPELINE_STAGE_ONE_GAIN    42
#define appconfAUDIO_FRAME_LENGTH            	256
#define appconfPRINT_AUDIO_FRAME_POWER          0

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )

#endif /* APP_CONF_H_ */
