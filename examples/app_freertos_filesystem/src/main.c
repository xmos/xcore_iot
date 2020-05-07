// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#define USE_FREERTOS_PLUS_FAT 0
#define USE_FATFS 1

#define clock libc_clock
#include <time.h>
#undef clock

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* Library headers */
#include "soc.h"
//#include <quadflashlib.h>

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "spi_master_driver.h"
#include "gpio_driver.h"
#include "qspi_flash_driver.h"
#include "sl_wfx.h"

/* App headers */
#include "sl_wfx_iot_wifi.h"
#include "network.h"
#include "dhcpd.h"

/* FreeRTOS+FAT includes. */
#if USE_FREERTOS_PLUS_FAT
#include "ff_headers.h"
#include "ff_stdio.h"
#include "ff_flashdisk.h"
#elif USE_FATFS
#include "ff.h"
#endif

//#include "FreeRTOS_TCP_server.h"

#define WF200_TASK_STACK_SIZE  1000//800
#define FTP_TASK_STACK_SIZE    500
#define TFTPD_TASK_STACK_SIZE  1000//100
#define FAT_TEST_STACK_SIZE    700

#define FTP_DEMO 0

//extern void vCreateAndVerifyExampleFiles( const char *pcMountPath );
//extern void vStdioWithCWDTest( const char *pcMountPath );
//extern void vStartTFTPServerTask(unsigned short, unsigned long);
//extern TCPServer_t *FreeRTOS_CreateTCPServer( const struct xSERVER_CONFIG *pxConfigs, BaseType_t xCount );

#define mainFLASH_DISK_SECTOR_SIZE    4096UL /* Currently fixed! */
#define mainFLASH_DISK_SECTORS        ( ( 1UL * 1024UL * 1024UL ) / mainFLASH_DISK_SECTOR_SIZE )
#define mainFLASH_DISK_IO_MANAGER_CACHE_SIZE   ( 2UL * mainFLASH_DISK_SECTOR_SIZE )
#define mainFLASH_DISK_NAME           "/flash"


static void indent(int level)
{
	for(int i = 0; i < level; i++) {
		rtos_printf("  ");
	}
}

#if USE_FREERTOS_PLUS_FAT && 0
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
#endif

#if USE_FREERTOS_PLUS_FAT
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

	rtos_printf("wf200 fw size is %d\n", wf200_fw_size);
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
		rtos_printf("items_read: %d\n", items_read);
		return SL_STATUS_FAIL;
	}
}
#elif USE_FATFS
static FIL wf200_fw_file;
static uint32_t wf200_fw_size;

uint32_t sl_wfx_app_fw_size(void)
{
	FRESULT result;

	if (wf200_fw_file.obj.fs == NULL) {
		rtos_printf("Opening WF200 firmware file\n");
		result = f_open(&wf200_fw_file, "/flash/firmware/wf200.sec", FA_READ);
	}

	if (result == FR_OK) {
		wf200_fw_size = f_size(&wf200_fw_file);
	} else {
		wf200_fw_size = 0;
	}

	rtos_printf("wf200 fw size is %d\n", wf200_fw_size);
	return wf200_fw_size;
}

sl_status_t sl_wfx_app_fw_read(uint8_t *data, uint32_t index, uint32_t size)
{
	FRESULT result;
	uint32_t bytes_read = 0;

	if (wf200_fw_file.obj.fs != NULL) {
		result = f_read(&wf200_fw_file, data, size, &bytes_read);
	}

	if (bytes_read == 0 || index + size >= wf200_fw_size) {
		if (wf200_fw_file.obj.fs != NULL) {
			f_close(&wf200_fw_file);
			wf200_fw_size = 0;
			rtos_printf("Closed WF200 firmware file\n");
		}
	}

	if (bytes_read == size) {
		return SL_STATUS_OK;
	} else {
		rtos_printf("items_read: %d\n", bytes_read);
		return SL_STATUS_FAIL;
	}
}
#endif

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
#if 0
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

	vStartTFTPServerTask( TFTPD_TASK_STACK_SIZE, 15 );

	for( ;; )
	{
		FreeRTOS_TCPServerWork( pxTCPServer, pdMS_TO_TICKS(100) );
	}
}
#endif

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
    } else {
    	rtos_printf("WIFI_Scan() failed %d\n", ret);
    }

	pxNetworkParams.pcSSID = "24g.apodize.com";
	pxNetworkParams.ucSSIDLength = strlen(pxNetworkParams.pcSSID);
	pxNetworkParams.pcPassword = "katie123";
	pxNetworkParams.ucPasswordLength = strlen(pxNetworkParams.pcPassword);
	pxNetworkParams.xSecurity = eWiFiSecurityWPA;
	pxNetworkParams.cChannel = 0;

	do {
		ret = WIFI_ConnectAP(&pxNetworkParams);
		rtos_printf("WIFI_ConnectAP() returned %x\n", ret);
	} while (ret != eWiFiSuccess);

	vTaskDelay(pdMS_TO_TICKS(5000));

	while (WIFI_GetIP( (void *) &ip ) != eWiFiSuccess) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	FreeRTOS_inet_ntoa(ip, a);
	rtos_printf("My IP is %s\n", a);

#if FTP_DEMO
    xTaskCreate(ftp_test, "ftp_test", FTP_TASK_STACK_SIZE, NULL, 15, NULL);
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

		rtos_printf("Minimum heap free: %d\n", xPortGetMinimumEverFreeHeapSize());
		rtos_printf("Current heap free: %d\n", xPortGetFreeHeapSize());

		free_stack_words = uxTaskGetStackHighWaterMark(NULL);
		rtos_printf("wf200_test free stack words: %d\n", free_stack_words);

		free_stack_words = uxTaskGetStackHighWaterMark(xTaskGetHandle("ftp_test"));
		rtos_printf("ftp_test free stack words: %d\n", free_stack_words);

		free_stack_words = uxTaskGetStackHighWaterMark(xTaskGetHandle("TFTPd"));
		rtos_printf("TFTPd free stack words: %d\n", free_stack_words);

	}
}

static void prvCreateDiskAndExampleFiles( void* arg )
{
#if USE_FREERTOS_PLUS_FAT
FF_Disk_t *pxDisk;

//	rtos_printf("Flash can hold %d bytes\n", fl_getFlashSize() );

    /* Create the Flash disk. */
#if SOC_QSPI_FLASH_PERIPHERAL_USED
	pxDisk = FF_FlashDiskInit( mainFLASH_DISK_NAME, 0x100000, mainFLASH_DISK_SECTORS, mainFLASH_DISK_IO_MANAGER_CACHE_SIZE );
#else
    pxDisk = FF_FlashDiskInit( mainFLASH_DISK_NAME, 0x000000, mainFLASH_DISK_SECTORS, mainFLASH_DISK_IO_MANAGER_CACHE_SIZE );
#endif
    configASSERT( pxDisk );

    /* Start the wifi test task now that the filesystem has been mounted
     * since it will load the WF200 firmware from the filesystem */
    xTaskCreate(wf200_test, "wf200_test", WF200_TASK_STACK_SIZE, NULL, 16, NULL);
#if 0
    /* Print out information on the disk. */
    FF_FlashDiskShowPartition( pxDisk );


    rtos_printf("Removing test directory\n");
    ff_deltree( mainFLASH_DISK_NAME "/test" );
    rtos_printf("Creating test directory\n");
    ff_mkdir( mainFLASH_DISK_NAME "/test" );

    /* Create a few example files on the disk */
    vCreateAndVerifyExampleFiles( mainFLASH_DISK_NAME "/test" );
#if !(FTP_DEMO)
    FF_FlashDiskShowPartition( pxDisk );

	vStdioWithCWDTest( mainFLASH_DISK_NAME "/test" );

	rtos_printf("\n\nSTDIO with CWD tests complete!\n\n");
#endif

    rtos_printf("\n");
    ls_recursive("/flash", 0);
#endif
#elif USE_FATFS

    FATFS *fs;
    fs = pvPortMalloc(sizeof(FATFS));
    f_mount(fs, "", 0);

    /* Start the wifi test task now that the filesystem has been mounted
     * since it will load the WF200 firmware from the filesystem */
    xTaskCreate(wf200_test, "wf200_test", WF200_TASK_STACK_SIZE, NULL, 16, NULL);

#endif

    for(;;)
    {
    	int free_stack_words;
		vTaskDelay(pdMS_TO_TICKS(5000));
		free_stack_words = uxTaskGetStackHighWaterMark(NULL);
		rtos_printf("prvCreateDiskAndExampleFiles free stack words: %d\n", free_stack_words);
#if USE_FREERTOS_PLUS_FAT
//        FF_FlashDiskShowPartition( pxDisk );
#endif
    }
}

void soc_tile0_main(
        int tile)
{
    rtos_printf("Hello from tile %d\n", tile);

#if SOC_QSPI_FLASH_PERIPHERAL_USED
    qspi_flash_driver_init(
            BITSTREAM_QSPI_FLASH_DEVICE_A,
            0);
#endif

	xTaskCreate(prvCreateDiskAndExampleFiles, "fs_test", FAT_TEST_STACK_SIZE, NULL, 15, NULL);

    vTaskStartScheduler();
}


void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
    configASSERT(0);
}
