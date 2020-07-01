// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>
#include <stdio.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* Library headers */
#include "soc.h"

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

/* FATFS include */
#include "ff.h"

#define SAVE_WIFI_NETWORKS 0

#define WF200_TASK_STACK_SIZE  1000//800
#define FAT_TEST_STACK_SIZE    700


#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
#if XCOREAI_EXPLORER
__attribute__((section(".ExtMem_data")))
#endif
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif


static FIL wf200_fw_file;
static uint32_t wf200_fw_size;

uint32_t sl_wfx_app_fw_size(void)
{
	FRESULT result = FR_OK;

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
	UINT bytes_read = 0;

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

static void wf200_test(void *arg)
{
    WIFIReturnCode_t ret;
    WIFIScanResult_t scan_results[20];
    WIFINetworkParams_t network_params;
    WIFINetworkProfile_t network_profile;
	static uint32_t ip;
	int i;
	char a[16];

    rtos_printf("Hello from wf200 test\n ");

    gpio_driver_init(
            BITSTREAM_GPIO_DEVICE_A,
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

#if SAVE_WIFI_NETWORKS
    uint16_t profile_number;

    /* Ensure the wifi directory exists */
    f_mkdir("/flash/wifi");

    while (WIFI_NetworkDelete(0) == eWiFiSuccess) {
        rtos_printf("Deleted WiFi profile\n");
    }

    strcpy(network_profile.cSSID, "SSID1");
    strcpy(network_profile.cPassword, "PASSWORD1");
    network_profile.ucSSIDLength = strlen(network_profile.cSSID);
    network_profile.ucPasswordLength = strlen(network_profile.cPassword);
    network_profile.xSecurity = eWiFiSecurityWPA;

    ret = WIFI_NetworkAdd(&network_profile, &profile_number);

    if (ret == eWiFiSuccess) {
        rtos_printf("Added WiFi profile %d\n", profile_number);
    } else {
        rtos_printf("Failed to add a WiFi profile\n");
    }

    strcpy(network_profile.cSSID, "SSID2");
    strcpy(network_profile.cPassword, "PASSWORD2");
    network_profile.ucSSIDLength = strlen(network_profile.cSSID);
    network_profile.ucPasswordLength = strlen(network_profile.cPassword);
    network_profile.xSecurity = eWiFiSecurityWPA;

    ret = WIFI_NetworkAdd(&network_profile, &profile_number);

    if (ret == eWiFiSuccess) {
        rtos_printf("Added WiFi profile %d\n", profile_number);
    } else {
        rtos_printf("Failed to add a WiFi profile\n");
    }

#endif

    i = 0;
    int more = 1;
    WIFIReturnCode_t connected = eWiFiFailure;
    do {
        if (WIFI_NetworkGet(&network_profile, i) == eWiFiSuccess) {
            size_t tmp_len;

            network_params.pcSSID = network_profile.cSSID;

            tmp_len = strnlen(network_profile.cSSID, wificonfigMAX_SSID_LEN);
            if (tmp_len < network_profile.ucSSIDLength) {
                network_params.ucSSIDLength = tmp_len;
            } else {
                network_params.ucSSIDLength = network_profile.ucSSIDLength;
            }

            network_params.pcPassword = network_profile.cPassword;

            tmp_len = strnlen(network_profile.cPassword, wificonfigMAX_PASSPHRASE_LEN);
            if (tmp_len < network_profile.ucPasswordLength) {
                network_params.ucPasswordLength = tmp_len;
            } else {
                network_params.ucPasswordLength = network_profile.ucPasswordLength;
            }

            network_params.xSecurity = network_profile.xSecurity;
            network_params.cChannel = 0;

            for (int j = 0; connected != eWiFiSuccess && j < 3; j++) {
                connected = WIFI_ConnectAP(&network_params);
                rtos_printf("WIFI_ConnectAP() returned %x\n", connected);
            }

            /* try the next profile if this one did not connect */
            i++;
        } else {
            more = 0;
        }
    } while (more && connected != eWiFiSuccess);

    if (connected != eWiFiSuccess) {
        rtos_printf("Could not connect to %d networks\n", i);
        vTaskDelete(NULL);
    } else {
        rtos_printf("Connected to network %d\n", i - 1);
    }

	vTaskDelay(pdMS_TO_TICKS(5000));

	while (WIFI_GetIP( (void *) &ip ) != eWiFiSuccess) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	FreeRTOS_inet_ntoa(ip, a);
	rtos_printf("My IP is %s\n", a);

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
	}
}

static void prvCreateDiskAndExampleFiles( void* arg )
{
    FATFS *fs;
    fs = pvPortMalloc(sizeof(FATFS));
    f_mount(fs, "", 0);

    /* Start the wifi test task now that the filesystem has been mounted
     * since it will load the WF200 firmware from the filesystem */
    xTaskCreate(wf200_test, "wf200_test", WF200_TASK_STACK_SIZE, NULL, 16, NULL);

    for(;;)
    {
    	int free_stack_words;
		vTaskDelay(pdMS_TO_TICKS(5000));
		free_stack_words = uxTaskGetStackHighWaterMark(NULL);
		rtos_printf("prvCreateDiskAndExampleFiles free stack words: %d\n", free_stack_words);
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
