// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <platform.h>

#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS Plus headers */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_Sockets.h"

#include "wifi_test.h"

#include "sl_wfx_iot_wifi.h"

typedef struct {
    rtos_spi_master_device_t *wifi_device_ctx;
    rtos_gpio_t *gpio_ctx;
} wifi_test_devs_t;

static void wifi_test_task(wifi_test_devs_t *wifi_devs)
{
    const rtos_gpio_port_id_t wifi_wup_rst_port = rtos_gpio_port(WIFI_WUP_RST_N);
    const rtos_gpio_port_id_t wifi_irq_port = rtos_gpio_port(WIFI_WIRQ);

    rtos_spi_master_device_t *wifi_device_ctx = wifi_devs->wifi_device_ctx;
    rtos_gpio_t *gpio_ctx = wifi_devs->gpio_ctx;

    vTaskDelay(pdMS_TO_TICKS(100));

    sl_wfx_host_set_hif(wifi_device_ctx,
                        gpio_ctx,
                        wifi_irq_port, 0,
                        wifi_wup_rst_port, 0,
                        wifi_wup_rst_port, 1);

    WIFI_On();

    const uint8_t ucIPAddress[ 4 ] = { IPconfig_IP_ADDR_OCTET_0, IPconfig_IP_ADDR_OCTET_1, IPconfig_IP_ADDR_OCTET_2, IPconfig_IP_ADDR_OCTET_3 };
    const uint8_t ucNetMask[ 4 ] = { IPconfig_NET_MASK_OCTET_0, IPconfig_NET_MASK_OCTET_1, IPconfig_NET_MASK_OCTET_2, IPconfig_NET_MASK_OCTET_3 };
    const uint8_t ucGatewayAddress[ 4 ] = { IPconfig_GATEWAY_OCTET_0, IPconfig_GATEWAY_OCTET_1, IPconfig_GATEWAY_OCTET_2, IPconfig_GATEWAY_OCTET_3 };
    const uint8_t ucDNSServerAddress[ 4 ] = { IPconfig_DNS_SERVER_OCTET_0, IPconfig_DNS_SERVER_OCTET_1, IPconfig_DNS_SERVER_OCTET_2, IPconfig_DNS_SERVER_OCTET_3 };
    uint8_t ucMACAddress[ 6 ] = { 0, 0, 0, 0, 0, 0 };
    FreeRTOS_IPInit(ucIPAddress,
                    ucNetMask,
                    ucGatewayAddress,
                    ucDNSServerAddress,
                    ucMACAddress);

    WIFIScanResult_t scan_results[10];

    if (WIFI_Scan(scan_results, 10) == eWiFiSuccess) {
        WIFIScanResult_t *scan_result = scan_results;

        for (int i = 0; i < 10; i++) {
            uint8_t no_bssid[wificonfigMAX_BSSID_LEN] = {0};

            if (memcmp(scan_result->ucBSSID, no_bssid, wificonfigMAX_BSSID_LEN) == 0) {
                break;
            }

            #define HWADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
            #define HWADDR_ARG(hwaddr) (hwaddr)[0], (hwaddr)[1], (hwaddr)[2], (hwaddr)[3], (hwaddr)[4], (hwaddr)[5]
            rtos_printf("Scan result %d:\n", i);
            rtos_printf("\tBSSID: " HWADDR_FMT "\n", i, HWADDR_ARG(scan_result->ucBSSID));
            rtos_printf("\tSSID: %s\n", scan_result->cSSID);
            rtos_printf("\tSecurity: %s\n", scan_result->xSecurity == eWiFiSecurityOpen ? "open" : scan_result->xSecurity == eWiFiSecurityWEP ? "WEP" : "WPA");
            rtos_printf("\tChannel: %d\n", (int) scan_result->cChannel);
            rtos_printf("\tStrength: %d dBm\n\n", (int) scan_result->cRSSI);

            scan_result++;
        }
    }

    WIFINetworkParams_t network_params = {
            .pcSSID = "xxxxx",             /**< SSID of the Wi-Fi network to join. */
            .ucSSIDLength = 15,            /**< SSID length not including NULL termination. */
            .pcPassword = "xxxxx",         /**< Password needed to join the AP. */
            .ucPasswordLength = 8,         /**< Password length not including NULL termination. */
            .xSecurity = eWiFiSecurityWPA, /**< Wi-Fi Security. @see WIFISecurity_t. */
            .cChannel = 0                  /**< Channel number. */
    };
    WIFIReturnCode_t connected;

    connected = WIFI_ConnectAP(&network_params);
    rtos_printf("WIFI_ConnectAP() returned %x\n", connected);
    if (connected == eWiFiSuccess) {
        uint8_t ip[4];

        while (WIFI_GetIP(ip) != eWiFiSuccess) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));

        WIFI_GetHostIP("google.com", ip);
        rtos_printf("google.com has IP address %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
        WIFI_Ping(ip, 3, 1000);
    }

    vTaskDelete(NULL);
}

void wifi_test_start(rtos_spi_master_device_t *wifi_device_ctx, rtos_gpio_t *gpio_ctx)
{
    static wifi_test_devs_t wifi_devs;

    wifi_devs.wifi_device_ctx = wifi_device_ctx;
    wifi_devs.gpio_ctx = gpio_ctx;

    xTaskCreate((TaskFunction_t) wifi_test_task,
                "wifi_test_task",
                RTOS_THREAD_STACK_SIZE(wifi_test_task),
                &wifi_devs,
                configMAX_PRIORITIES/2,
                NULL);
}
