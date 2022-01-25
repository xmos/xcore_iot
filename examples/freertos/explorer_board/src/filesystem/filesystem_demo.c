// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "filesystem/filesystem_demo.h"
#include "fs_support.h"

#define DEMO_FILEPATH		"/flash/fs/demo.txt"

static void filesystem_demo(void* args)
{
    FIL test_file;
    FRESULT result;
    uint32_t demo_file_size = -1;
    uint32_t bytes_read = 0;
    uint8_t *file_contents_buf = NULL;

    result = f_open(&test_file, DEMO_FILEPATH, FA_READ);
    if (result == FR_OK)
    {
        rtos_printf("Found file %s\n", DEMO_FILEPATH);
        demo_file_size = f_size(&test_file);

        file_contents_buf = pvPortMalloc(sizeof(uint8_t)*demo_file_size);
        configASSERT(file_contents_buf != NULL);

        result = f_read(&test_file,
                        (uint8_t*)file_contents_buf,
                        demo_file_size,
                        (unsigned int*)&bytes_read);
        configASSERT(bytes_read == demo_file_size);
    } else {
        rtos_printf("Failed to open file %s\n", DEMO_FILEPATH);
    }
    if (demo_file_size != -1)
    {
        rtos_printf("Loaded file %s\n", DEMO_FILEPATH);
        f_close(&test_file);
    }

    while(1)
    {
        rtos_printf("Contents of %s are:\n%s\n", DEMO_FILEPATH, file_contents_buf);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void filesystem_demo_create( UBaseType_t priority )
{
    xTaskCreate((TaskFunction_t) filesystem_demo,
                "filesystem_demo",
                RTOS_THREAD_STACK_SIZE(filesystem_demo),
                NULL,
                priority,
                NULL);
}
