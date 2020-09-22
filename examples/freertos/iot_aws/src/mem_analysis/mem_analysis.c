// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include "app_conf.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS Plus headers */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* Library headers */
#include "soc.h"

/* App headers */
#include "mem_analysis.h"

static void mem_analysis( void *arg )
{
	const char* task_name = ( const char* ) arg;

	if( strcmp( task_name, "heap" ) == 0 )
	{
		for( ;; )
		{
			debug_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
			debug_printf("Current heap free: %d\n", xPortGetFreeHeapSize());

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
			debug_printf("%s free stack words: %d\n", task_name, free_stack_words);
		}
		vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
}

void mem_analysis_create( const char* task_name )
{
    xTaskCreate( mem_analysis, "mem_an", portTASK_STACK_DEPTH(mem_analysis), ( void * ) task_name, configMAX_PRIORITIES, NULL );
}
