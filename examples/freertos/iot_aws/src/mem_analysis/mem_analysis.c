// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS Plus headers */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "mem_analysis.h"

static void mem_analysis( void *arg )
{
	const char* task_name = ( const char* ) arg;

	if( strcmp( task_name, "heap" ) == 0 )
	{
		for( ;; )
		{
			debug_printf("\tMinimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
			debug_printf("\tCurrent heap free: %d\n", xPortGetFreeHeapSize());

			vTaskDelay( pdMS_TO_TICKS( 1000 ) );
		}
	}

	int free_stack_words;
	TaskHandle_t task = NULL;

	for( ;; )
	{
		/* Always check to allow for tasks that can be deleted to be analyzed */
		task = xTaskGetHandle( task_name );

		if( task != NULL )
		{
			free_stack_words = uxTaskGetStackHighWaterMark( task );
			debug_printf("\t%s free stack words: %d\n", task_name, free_stack_words);
		}
		vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
}

void mem_analysis_create( const char* task_name )
{
    xTaskCreate( mem_analysis, "mem_an", portTASK_STACK_DEPTH(mem_analysis), ( void * ) task_name, appconfMEM_ANALYSIS_TASK_PRIORITY, NULL );
}
