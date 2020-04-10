// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#define clock libc_clock
#include <time.h>
#undef clock

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "ff_headers.h"
#include "ff_stdio.h"

/* Library headers */
#include "soc.h"
#include <quadflashlib.h>

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "spi_master_driver.h"
#include "gpio_driver.h"
#include "sl_wfx.h"

/* App headers */
#include "sl_wfx_iot_wifi.h"
#include "network.h"
#include "dhcpd.h"

/* FreeRTOS+FAT includes. */
#include "ff_headers.h"
#include "ff_stdio.h"
#include "ff_flashdisk.h"

#include "FreeRTOS_TCP_server.h"


#define FTP_DEMO 1

extern void vCreateAndVerifyExampleFiles( const char *pcMountPath );
extern void vStdioWithCWDTest( const char *pcMountPath );
extern void vStartTFTPServerTask(unsigned short, unsigned long);
extern TCPServer_t *FreeRTOS_CreateTCPServer( const struct xSERVER_CONFIG *pxConfigs, BaseType_t xCount );

#define mainFLASH_DISK_SECTOR_SIZE    512UL /* Currently fixed! */
#define mainFLASH_DISK_SECTORS        ( ( 1UL * 1024UL * 1024UL ) / mainFLASH_DISK_SECTOR_SIZE )
#define mainFLASH_DISK_IO_MANAGER_CACHE_SIZE   ( 2UL * mainFLASH_DISK_SECTOR_SIZE )
#define mainFLASH_DISK_NAME           "/flash"


static void indent(int level)
{
	for(int i = 0; i < level; i++) {
		rtos_printf("  ");
	}
}

static void ls_recursive( const char *pcDirectoryToScan, int level)
{
	FF_FindData_t *pxFindStruct;
	const char  *pcAttrib;
	const char  *pcWritableFile = "writable file";
	const char  *pcReadOnlyFile = "read only file";
	const char  *pcDirectory = "directory";

    /* FF_FindData_t can be large, so it is best to allocate the structure
    dynamically, rather than declare it as a stack variable. */
    pxFindStruct = ( FF_FindData_t * ) pvPortMalloc( sizeof( FF_FindData_t ) );

    /* FF_FindData_t must be cleared to 0. */
    memset( pxFindStruct, 0x00, sizeof( FF_FindData_t ) );

    indent(level);
    rtos_printf("Listing %s\n", pcDirectoryToScan);

    /* The first parameter to ff_findfist() is the directory being searched.  Do
    not add wildcards to the end of the directory name. */
    if( ff_findfirst( pcDirectoryToScan, pxFindStruct ) == 0 )
    {
        do
        {
            /* Point pcAttrib to a string that describes the file. */
            if( ( pxFindStruct->ucAttributes & FF_FAT_ATTR_DIR ) != 0 )
            {
                pcAttrib = pcDirectory;
                if (pxFindStruct->pcFileName[0] == '.') {
                	continue;
                }
            }
            else if( pxFindStruct->ucAttributes & FF_FAT_ATTR_READONLY )
            {
                pcAttrib = pcReadOnlyFile;
            }
            else
            {
                pcAttrib = pcWritableFile;
            }

            /* Print the files name, size, and attribute string. */
            indent(level);
            rtos_printf( "%s [%s] [size=%d]\n", pxFindStruct->pcFileName,
                                                pcAttrib,
                                                pxFindStruct->ulFileSize );

            if( ( pxFindStruct->ucAttributes & FF_FAT_ATTR_DIR ) != 0 )
            {
            	char *dirname = pvPortMalloc(strlen(pcDirectoryToScan) + 1 + strlen(pxFindStruct->pcFileName) + 1);
            	dirname[0] = '\0';
            	strcat(dirname, pcDirectoryToScan);
            	strcat(dirname, "/");
            	strcat(dirname, pxFindStruct->pcFileName);
            	ls_recursive(dirname, level + 1);
                vPortFree(dirname);
            }

        } while( ff_findnext( pxFindStruct ) == 0 );
    }

    /* Free the allocated FF_FindData_t structure. */
    vPortFree( pxFindStruct );
}


static FF_FILE *wf200_fw_file;
static uint32_t wf200_fw_size;

uint32_t sl_wfx_app_fw_size(void)
{
	if (wf200_fw_file == NULL) {
		rtos_printf("Opening WF200 firmware file\n");
		wf200_fw_file = ff_fopen("/flash/firmware/wf200.sec", "r");
	}

	if (wf200_fw_file != NULL) {
		wf200_fw_size = ff_filelength(wf200_fw_file);
	} else {
		wf200_fw_size = 0;
	}

	return wf200_fw_size;
}

sl_status_t sl_wfx_app_fw_read(uint8_t *data, uint32_t index, uint32_t size)
{
	uint32_t items_read = 0;

	if (wf200_fw_file != NULL) {
		items_read = ff_fread(data, size, 1, wf200_fw_file);
	}

	if (items_read == 0 || index + size >= wf200_fw_size) {
		if (wf200_fw_file != NULL) {
			ff_fclose(wf200_fw_file);
			wf200_fw_file = NULL;
			wf200_fw_size = 0;
			rtos_printf("Closed WF200 firmware file\n");
		}
	}

	if (items_read == 1) {
		return SL_STATUS_OK;
	} else {
		return SL_STATUS_FAIL;
	}
}

static char *security_name(WIFISecurity_t s)
{
    switch (s) {
    case eWiFiSecurityOpen:
        return "Open";
    case eWiFiSecurityWEP:
        return "WEP";
    case eWiFiSecurityWPA:
        return "WPA";
    case eWiFiSecurityWPA2:
        return "WPA2";
    case eWiFiSecurityWPA2_ent:
        return "WPA2 Enterprise";
    default:
        return "Unsupported";
    }
}

static void ftp_test(void *arg)
{
	TCPServer_t *pxTCPServer = NULL;
	static const struct xSERVER_CONFIG xServerConfiguration[] =
	{
				/* Server type,		port number,	backlog, 	root dir. */
				{ eSERVER_FTP,  	21, 			12, 		"" }
	};
	pxTCPServer = FreeRTOS_CreateTCPServer( xServerConfiguration, sizeof( xServerConfiguration ) / sizeof( xServerConfiguration[ 0 ] ) );
	configASSERT( pxTCPServer );

	vStartTFTPServerTask( 2000, 15 );

	for( ;; )
	{
		FreeRTOS_TCPServerWork( pxTCPServer, pdMS_TO_TICKS(100) );
	}
}

static void wf200_test(void *arg)
{
    WIFIReturnCode_t ret;
    WIFIScanResult_t scan_results[20];
    WIFINetworkParams_t pxNetworkParams;
	static uint32_t ip;
	char a[16];

    rtos_printf("Hello from wf200 test\n ");

    spi_master_driver_init(
            BITSTREAM_SPI_DEVICE_A,  /* Initializing SPI device A */
            2,                       /* Use 2 DMA buffers for the scatter/gather */
            0);                      /* This device's interrupts should happen on core 0 */

    gpio_driver_init(
            BITSTREAM_GPIO_DEVICE_A,
            NULL,
            0);

    ret = WIFI_On();

    rtos_printf("WIFI_On() returned %x\n", ret);

    if (ret != eWiFiSuccess) {
    	vTaskDelete(NULL);
    }

    initalize_FreeRTOS_IP();

    ret = WIFI_Scan(scan_results, 20);

    if (ret == eWiFiSuccess) {
        for (int i = 0; i < 20; i++) {
            uint8_t no_bssid[wificonfigMAX_BSSID_LEN] = {0};
            if (memcmp(scan_results[i].ucBSSID, no_bssid, wificonfigMAX_BSSID_LEN) == 0) {
                break;
            }
            rtos_printf("%d: %s\n", i, scan_results[i].cSSID);
            rtos_printf("\tChannel: %d\n", (int) scan_results[i].cChannel);
            rtos_printf("\tStrength: %d dBm\n", (int) scan_results[i].cRSSI);
            rtos_printf("\t%s\n", security_name(scan_results[i].xSecurity));
        }
    }

	pxNetworkParams.pcSSID = "xxxxxxxxx";
	pxNetworkParams.ucSSIDLength = strlen(pxNetworkParams.pcSSID);
	pxNetworkParams.pcPassword = "xxxxxxxxx";
	pxNetworkParams.ucPasswordLength = strlen(pxNetworkParams.pcPassword);
	pxNetworkParams.xSecurity = eWiFiSecurityWPA;
	pxNetworkParams.cChannel = 0;

	do {
		ret = WIFI_ConnectAP(&pxNetworkParams);
		rtos_printf("WIFI_ConnectAP() returned %x\n", ret);
	} while (ret != eWiFiSuccess);

	vTaskDelay(pdMS_TO_TICKS(5000));

	WIFI_GetIP( (void *) &ip );
	FreeRTOS_inet_ntoa(ip, a);
	rtos_printf("My IP is %s\n", a);

#if FTP_DEMO
    xTaskCreate(ftp_test, "ftp_test", 4000, NULL, 15, NULL);
#endif

	WIFI_GetHostIP("google.com", (void *) &ip );
	FreeRTOS_inet_ntoa(ip, a);
	rtos_printf("google.com is %s\n", a);

	rtos_printf("Pinging google.com now!\n");
	WIFI_Ping( (void *) &ip, 5, 1000 );

	for(;;)
	{
		int free_stack_words;
		vTaskDelay(pdMS_TO_TICKS(5000));
		free_stack_words = uxTaskGetStackHighWaterMark(NULL);
		rtos_printf("wf200_test free stack words: %d\n", free_stack_words);
		rtos_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
	}
}

static void prvCreateDiskAndExampleFiles( void* arg )
{
FF_Disk_t *pxDisk;

	rtos_printf("Flash can hold %d bytes\n", fl_getFlashSize() );

    /* Create the Flash disk. */
    pxDisk = FF_FlashDiskInit( mainFLASH_DISK_NAME, 0, mainFLASH_DISK_SECTORS, mainFLASH_DISK_IO_MANAGER_CACHE_SIZE );
    configASSERT( pxDisk );

    /* Start the wifi test task now that the filesystem has been mounted
     * since it will load the WF200 firmware from the filesystem */
    xTaskCreate(wf200_test, "wf200_test", 800, NULL, 15, NULL);

    /* Print out information on the disk. */
    FF_FlashDiskShowPartition( pxDisk );

    rtos_printf("Removing test directory\n");
    ff_deltree( mainFLASH_DISK_NAME "/test" );
    rtos_printf("Creating test directory\n");
    ff_mkdir( mainFLASH_DISK_NAME "/test" );

    /* Create a few example files on the disk */
    vCreateAndVerifyExampleFiles( mainFLASH_DISK_NAME "/test" );

    FF_FlashDiskShowPartition( pxDisk );

#if !(FTP_DEMO)
	vStdioWithCWDTest( mainFLASH_DISK_NAME "/test" );
#endif
	rtos_printf("\n\nSTDIO with CWD tests complete!\n\n");

    rtos_printf("\n");
    ls_recursive("/flash", 0);


    for(;;)
    {
    	int free_stack_words;
		vTaskDelay(pdMS_TO_TICKS(5000));
		free_stack_words = uxTaskGetStackHighWaterMark(NULL);
		rtos_printf("prvCreateDiskAndExampleFiles free stack words: %d\n", free_stack_words);
        FF_FlashDiskShowPartition( pxDisk );
    }
}

void soc_tile0_main(
        int tile)
{
    rtos_printf("Hello from tile %d\n", tile);

	xTaskCreate(prvCreateDiskAndExampleFiles, "fs_test", 700, NULL, 15, NULL);

    vTaskStartScheduler();
}


void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
    configASSERT(0);
}
