// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile port settings */
#define appconfEXAMPLE_DATA_PORT          16

/* Application tile specifiers */
#include "platform/driver_instances.h"

/* App configuration */
#define appconfINPUT_FILENAME  "in.wav\0"
#define appconfOUTPUT_FILENAME "out.wav\0"
#define appconfMAX_CHANNELS 1
#define appconfFRAME_ADVANCE 240
#define appconfFRAME_ELEMENT_SIZE sizeof(int32_t)
#define appconfDATA_FRAME_SIZE_BYTES   (appconfFRAME_ADVANCE * appconfFRAME_ELEMENT_SIZE)

#define appconfAPP_NOTIFY_FILEIO_DONE  0

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY              (configMAX_PRIORITIES - 2)
#define appconfXSCOPE_IO_TASK_PRIORITY            (configMAX_PRIORITIES - 1)
#define appconfDATA_PIPELINE_TASK_PRIORITY        (configMAX_PRIORITIES - 1)

#endif /* APP_CONF_H_ */
