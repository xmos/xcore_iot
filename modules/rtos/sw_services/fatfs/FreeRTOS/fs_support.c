// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xcore/assert.h>

#include "fs_support.h"

rtos_qspi_flash_t *ff_qspi_flash_ctx;

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

int rtos_ff_get_file(const char* filename, FIL* outfile, unsigned int* len )
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

void rtos_fatfs_init( rtos_qspi_flash_t *qspi_flash_ctx )
{
    FATFS *fs;

    fs = ( FATFS* )FS_SUP_MALLOC( sizeof( FATFS ) );

    if( fs == NULL )
    {
    	xassert(0);	/* Failed to allocate file system object */
    }

    ff_qspi_flash_ctx = qspi_flash_ctx;

	if( f_mount( fs, "", 0 ) != FR_OK )
	{
		FS_SUP_FREE( fs );
		xassert(0);	/* Failed to mount logical drive */
	}
}
