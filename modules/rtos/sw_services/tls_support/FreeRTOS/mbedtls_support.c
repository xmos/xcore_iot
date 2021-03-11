// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT MBEDTLS_SUPPORT
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "FreeRTOS_Sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/error.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/platform_time.h"
#include "mbedtls/threading.h"

/* Library headers */
#include "random.h"
#include "tls_support.h"
#include "fs_support.h"
#include "ff.h"

#ifndef MBEDTLS_PLATFORM_MEMORY
#error MBEDTLS_PLATFORM_MEMORY must be enabled
#endif

static mbedtls_entropy_context entrp_ctx;
static mbedtls_ctr_drbg_context drbg_ctx;

static random_generator_t ring_oscillator = 0;

static int platform_ready = 0;

int tls_platform_ready( void )
{
	return platform_ready;
}

#ifdef MBEDTLS_DEBUG_C
void default_mbedtls_debug( void *ctx, int level,
							const char *file, int line,
							const char *str )
{
    const char *p, *basename;

    /* Extract basename from file */
    for( p = basename = file; *p != '\0'; p++ )
        if( *p == '/' || *p == '\\' )
            basename = p + 1;

    rtos_printf( "%s:%04d: |%d| %s", basename, line, level, str );
}
#endif


mbedtls_entropy_context* get_shared_entropy_ctx( void )
{
	return &entrp_ctx;
}

mbedtls_ctr_drbg_context* get_shared_drbg_ctx( void )
{
	return &drbg_ctx;
}

int mbedtls_hardware_poll( void* data, unsigned char* output, size_t len, size_t* olen )
{
	int retval = 0;

	if( ring_oscillator == 0 )
	{
		retval = MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
	}
	else
	{
		for( int i=0; i<len; i++ )
		{
			output[i] = random_get_random_number( &ring_oscillator );
		}
		(*olen) = len;
	}

	return retval;
}

static void freertos_mutex_init( mbedtls_threading_mutex_t* mutex )
{
    mutex->mutex = xSemaphoreCreateMutex();
    mutex->is_valid = ( char )( mutex->mutex != NULL );
}

static void freertos_mutex_free( mbedtls_threading_mutex_t* mutex )
{
    if( mutex->is_valid == 1 )
    {
        vSemaphoreDelete( mutex->mutex );
        mutex->is_valid = 0;
    }
}

static int freertos_mutex_lock( mbedtls_threading_mutex_t* mutex )
{
    int retval = 0;

    if( mutex->is_valid == 0 )
    {
    	retval = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;
    }
    else
    {
        if( ( xSemaphoreTake( mutex->mutex, portMAX_DELAY ) ) == pdFALSE )
        {
        	retval = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
        }
    }

    return retval;
}

static int freertos_mutex_unlock( mbedtls_threading_mutex_t* mutex )
{
    int retval = 0;

    if( mutex->is_valid == 0 )
    {
    	retval = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;
    }
    else
    {
        if( ( xSemaphoreGive( mutex->mutex ) ) == pdFALSE )
        {
        	retval = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
        }
    }

    return retval;
}

void tls_ctx_init( tls_ctx_t* ctx )
{
    memset( ctx, 0, sizeof( tls_ctx_t ) );
}

void tls_platform_init( void )
{
	ring_oscillator = random_create_generator_from_hw_seed();

    mbedtls_threading_set_alt( freertos_mutex_init,
							   freertos_mutex_free,
							   freertos_mutex_lock,
							   freertos_mutex_unlock );

    mbedtls_ctr_drbg_init( &drbg_ctx );
    mbedtls_entropy_init( &entrp_ctx );

    if( ( mbedtls_ctr_drbg_seed( &drbg_ctx,
								 mbedtls_entropy_func,
								 &entrp_ctx,
								 (const unsigned char *) drbg_seed_string,
								 strlen( drbg_seed_string ) ) )
    		!= 0 )
    {
        configASSERT(0); /* mbedtls_ctr_drbg_seed failed */
    }

    platform_ready = 1;
}

void tls_platform_free( void )
{
	mbedtls_ctr_drbg_free( &drbg_ctx );
	mbedtls_entropy_free( &entrp_ctx );
	platform_ready = 0;
}

int tls_send( void* ctx, const unsigned char* buf, size_t len)
{
	tls_ctx_t* tls_ctx = ( tls_ctx_t* ) ctx;
	return FreeRTOS_send( tls_ctx->socket, buf, len, tls_ctx->flags );
}

int tls_recv( void* ctx, unsigned char* buf, size_t len)
{
	tls_ctx_t* tls_ctx = ( tls_ctx_t* ) ctx;
	return FreeRTOS_recv( tls_ctx->socket, buf, len, tls_ctx->flags );
}


extern int rtos_ff_get_file(const char* filename, FIL* outfile, unsigned int* len );


int get_cert( mbedtls_x509_crt* cert, const char* filepath )
{
	int retval = pdFAIL;
	FIL prvfile;
	unsigned int prvfile_len = 0;
	unsigned char * data;
	unsigned bytes_read;
	FRESULT result;

	do
	{
		/* Check that a valid pointer was passed */
		if( cert == NULL )
		{
			break;
		}

		if( rtos_ff_get_file( filepath, &prvfile, &prvfile_len ) == pdFAIL )
		{
			rtos_printf("Get cert file failed\n");
			break;
		}

		/* 0x00 must be at the end of data to parsed as a PEM certificate
		 * by mbedtls, so malloc an extra byte
		 * See mbedtls_x509_crt_parse_file */
		data = pvPortMalloc( sizeof( unsigned char ) * ( prvfile_len + 1) );

		if( data != NULL )
		{
			data[ prvfile_len ] = 0x00;

			result = f_read( &prvfile, data, prvfile_len, &bytes_read );
			rtos_printf("cert %d bytes\n%s\n", prvfile_len, data);

			if( bytes_read == prvfile_len )
			{
				int ret;
				if( ( ret = mbedtls_x509_crt_parse( cert, ( const unsigned char* ) data, prvfile_len + 1 ) ) < 0 )
				{
					rtos_printf("failed mbedtls_x509_crt_parse ret:-0x%x\n", ( unsigned int )-ret );
				}
				else
				{
					retval = pdPASS;
				}
			}
			else
			{
				rtos_printf("failed to read cert file\n");
			}
			mbedtls_platform_zeroize( data, prvfile_len );
			vPortFree( data );
			f_close( &prvfile );
			break;
		}
		else
		{
			rtos_printf("failed to allocate buffer for cert\n");
		}

		f_close( &prvfile );
	} while(0);

	return retval;
}

int get_key( mbedtls_pk_context* key, const char* filepath )
{
	int retval = pdFAIL;
	FIL prvfile;
	unsigned int prvfile_len = 0;
	unsigned char * data;
	unsigned bytes_read;
	FRESULT result;

	do
	{
		/* Check that a valid pointer was passed */
		if( key == NULL )
		{
			break;
		}

		if( rtos_ff_get_file( filepath, &prvfile, &prvfile_len ) == pdFAIL )
		{
			rtos_printf("Get key file failed\n");
			break;
		}

		/* 0x00 must be at the end of data to parsed as a PEM certificate
		 * by mbedtls, so malloc an extra byte.
		 * See mbedtls_x509_crt_parse_file
		 */
		data = pvPortMalloc( sizeof( unsigned char ) * ( prvfile_len + 1) );

		if( data != NULL )
		{
			data[ prvfile_len ] = 0x00;

			result = f_read( &prvfile, data, prvfile_len, &bytes_read );
			rtos_printf("key %d bytes\n%s\n", prvfile_len, data);

			if( bytes_read == prvfile_len )
			{
				int ret;
				if( ( ret = mbedtls_pk_parse_key( key, ( const unsigned char* ) data, prvfile_len + 1, NULL, 0 ) ) < 0 )
				{
					rtos_printf("failed mbedtls_pk_parse_key ret:-0x%x\n", ( unsigned int )-ret );
				}
				else
				{
					retval = pdPASS;
				}
			}
			else
			{
				rtos_printf("failed to read key file\n");
			}
			mbedtls_platform_zeroize( data, prvfile_len );
			vPortFree( data );
			f_close( &prvfile );
			break;
		}
		else
		{
			rtos_printf("failed to allocate buffer for key\n");
		}

		f_close( &prvfile );
	} while(0);

	return retval;
}

int get_ca_cert( mbedtls_x509_crt* ca_cert )
{
	int retval = pdFAIL;

	if( ca_cert != NULL )
	{
		retval = get_cert( ca_cert, ca_chain_filepath );
	}
	return retval;
}

int get_device_cert( mbedtls_x509_crt* cert )
{
	int retval = pdFAIL;

	if( cert != NULL )
	{
		retval = get_cert( cert, cert_filepath );
	}
	return retval;
}

int get_device_prvkey( mbedtls_pk_context* prvkey )
{
	int retval = pdFAIL;

	if( prvkey != NULL )
	{
		retval = get_key( prvkey, prvkey_filepath );
	}
	return retval;
}
