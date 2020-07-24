// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include "fs_support.h"

#if RTOS_FREERTOS
#include "FreeRTOS.h"

#define FS_SUP_SUCCESS		pdPASS
#define FS_SUP_FAIL			pdFAIL

#define FS_SUP_MALLOC		pvPortMalloc
#define FS_SUP_FREE			vPortFree
#endif

#ifndef FS_SUP_SUCCESS
#define FS_SUP_SUCCESS		(1)
#endif

#ifndef FS_SUP_FAIL
#define FS_SUP_FAIL			(0)
#endif

#ifndef FS_SUP_MALLOC
#define FS_SUP_MALLOC		malloc
#endif

#ifndef FS_SUP_FREE
#define FS_SUP_FREE			free
#endif

int get_file(const char* filename, FIL* outfile, unsigned int* len )
{
	int retval = FS_SUP_FAIL;

	if( ( outfile != NULL ) && ( filename != NULL ) )
	{
		FRESULT result;
		result = f_open( outfile, filename, FA_READ );

		if( result == FR_OK )
		{
			if( len != NULL )
			{
				*len = f_size( outfile );
			}

			retval = FS_SUP_SUCCESS;
		}
	}

	return retval;
}

void filesystem_init( void )
{
    FATFS *fs;

    fs = ( FATFS* )FS_SUP_MALLOC( sizeof( FATFS ) );

    if( fs == NULL )
    {
    	configASSERT(0);	/* Failed to allocate file system object */
    }

	if( f_mount( fs, "", 0 ) != FR_OK )
	{
		FS_SUP_FREE( fs );
    	configASSERT(0);	/* Failed to mount logical drive */
	}
}
