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
#include "wifi.h"

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

void initalize_FreeRTOS_IP( void )
{
    FreeRTOS_IPInit(
        ucIPAddress,
        ucNetMask,
        ucGatewayAddress,
        ucDNSServerAddress,
        ucMACAddress );
}

#define SOFT_AP_SSID "xcore.ai"
#define SOFT_AP_PASSWORD ""

int wifi_conn_mgr_event_cb(int event, char *ssid, char *password)
{
    switch (event) {
    case WIFI_CONN_MGR_EVENT_STARTUP:
        rtos_printf("Directing WiFi manager to go into station mode\n");

        return WIFI_CONN_MGR_MODE_STATION;

    case WIFI_CONN_MGR_EVENT_CONNECT_FAILED:
        rtos_printf("Directing WiFi manager to start a soft AP\n");

        strcpy(ssid, SOFT_AP_SSID);
        strcpy(password, SOFT_AP_PASSWORD);
        return WIFI_CONN_MGR_MODE_SOFT_AP;

    case WIFI_CONN_MGR_EVENT_CONNECTED:
        rtos_printf("Connected to %s\n", ssid);

        return WIFI_CONN_MGR_MODE_STATION; /* this is ignored */

    case WIFI_CONN_MGR_EVENT_DISCONNECTED:
        if (ssid[0] != '\0') {
            rtos_printf("Disconnected from %s\n", ssid);
        } else {
            rtos_printf("Disconnected from AP\n");
        }

        return WIFI_CONN_MGR_MODE_STATION;

    case WIFI_CONN_MGR_EVENT_SOFT_AP_STARTED:
        rtos_printf("Soft AP %s started\n", ssid);

        return WIFI_CONN_MGR_MODE_SOFT_AP; /* this is ignored */

    case WIFI_CONN_MGR_EVENT_SOFT_AP_STOPPED:
        rtos_printf("Soft AP %s stopped. Going into station mode\n", ssid);

        return WIFI_CONN_MGR_MODE_STATION;

    default:
        return WIFI_CONN_MGR_MODE_STATION;
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

    initalize_FreeRTOS_IP();

    wifi_conn_mgr_start();
}
