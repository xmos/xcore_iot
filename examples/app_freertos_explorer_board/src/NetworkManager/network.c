// Copyright (c) 2019, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* FreeRTOS Plus headers */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* Library headers */
#include "soc.h"
#include "bitstream_devices.h"
#include "spi_master_driver.h"
#include "gpio_driver.h"
#include "qspi_flash_driver.h"
#include "sl_wfx.h"
#include "sl_wfx_iot_wifi.h"

/* App headers */
#include "network.h"
#include "app_conf.h"
#include "dhcpd.h"
#include "ff.h"

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

const uint8_t ucIPAddress[ipIP_ADDRESS_LENGTH_BYTES] = {
                                ( uint8_t ) IPconfig_IP_ADDR_OCTET_0,
                                ( uint8_t ) IPconfig_IP_ADDR_OCTET_1,
                                ( uint8_t ) IPconfig_IP_ADDR_OCTET_2,
                                ( uint8_t ) IPconfig_IP_ADDR_OCTET_3 };

const uint8_t ucNetMask[ipIP_ADDRESS_LENGTH_BYTES] = {
                                ( uint8_t ) IPconfig_NET_MASK_OCTET_0,
                                ( uint8_t ) IPconfig_NET_MASK_OCTET_1,
                                ( uint8_t ) IPconfig_NET_MASK_OCTET_2,
                                ( uint8_t ) IPconfig_NET_MASK_OCTET_3 };

const uint8_t ucGatewayAddress[ipIP_ADDRESS_LENGTH_BYTES] = {
                                ( uint8_t ) IPconfig_GATEWAY_OCTET_0,
                                ( uint8_t ) IPconfig_GATEWAY_OCTET_1,
                                ( uint8_t ) IPconfig_GATEWAY_OCTET_2,
                                ( uint8_t ) IPconfig_GATEWAY_OCTET_3 };

const uint8_t ucDNSServerAddress[ipIP_ADDRESS_LENGTH_BYTES] = {
                                ( uint8_t ) IPconfig_DNS_SERVER_OCTET_0,
                                ( uint8_t ) IPconfig_DNS_SERVER_OCTET_1,
                                ( uint8_t ) IPconfig_DNS_SERVER_OCTET_2,
                                ( uint8_t ) IPconfig_DNS_SERVER_OCTET_3 };

const uint8_t ucMACAddress[ipMAC_ADDRESS_LENGTH_BYTES] = {
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_0,
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_1,
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_2,
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_3,
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_4,
                                ( uint8_t ) IPconfig_MAC_ADDR_OCTET_5 };

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

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    if (eNetworkEvent == eNetworkDown) {
        TaskHandle_t task = xTaskGetHandle("wf200_test");
        if (task != NULL) {
            rtos_printf("Network down, notifying the network task\n");
            xTaskNotifyGive(task);
        }
    }
}

static void wf200_test(void *arg)
{
    WIFIReturnCode_t ret;
    WIFIScanResult_t scan_results[20];
    WIFINetworkParams_t pxNetworkParams;

    rtos_printf("Hello from wf200 test\n ");

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

    pxNetworkParams.pcSSID = "xxxxxxxx";
    pxNetworkParams.ucSSIDLength = strlen(pxNetworkParams.pcSSID);
    pxNetworkParams.pcPassword = "xxxxxxxx";
    pxNetworkParams.ucPasswordLength = strlen(pxNetworkParams.pcPassword);
    pxNetworkParams.xSecurity = eWiFiSecurityWPA;
    pxNetworkParams.cChannel = 0;

    while (1) {
        int i;
        uint32_t ip = 0;
        char a[16];


        ret = eWiFiFailure;
        for (i = 0; ret != eWiFiSuccess && i < 10; i++) {
            ret = WIFI_ConnectAP(&pxNetworkParams);
            rtos_printf("WIFI_ConnectAP() returned %x\n", ret);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        if (ret != eWiFiSuccess) {
            /*
             * If a certain number of connection attempt fail, reset the device
             * before continuing to try.
             */
            WIFI_Reset();
            continue;
        }

        for (i = 0; WIFI_GetIP( (void *) &ip ) != eWiFiSuccess && i < 10; i++) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        FreeRTOS_inet_ntoa(ip, a);
        rtos_printf("My IP is %s\n", a);

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        rtos_printf("Network down, will attempt to reconnect\n");
    }
}

void initalize_wifi( void )
{

    soc_peripheral_t dev;

    dev = gpio_driver_init(
            BITSTREAM_GPIO_DEVICE_A,
            0);

    dev = qspi_flash_driver_init(
            BITSTREAM_QSPI_FLASH_DEVICE_A,
            0);

    xTaskCreate(wf200_test, "wf200_test", 1000/*portTASK_STACK_DEPTH(wf200_test)*/, NULL, 15, NULL);

}

void initalize_FreeRTOS_IP( void )
{
    FreeRTOS_IPInit(
        ucIPAddress,
        ucNetMask,
        ucGatewayAddress,
        ucDNSServerAddress,
        ucMACAddress );
}

