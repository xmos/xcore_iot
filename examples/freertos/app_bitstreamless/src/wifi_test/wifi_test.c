// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <platform.h>

#include "FreeRTOS.h"
#include "task.h"

#include "wifi_test.h"

#include "sl_wfx_iot_wifi.h"

typedef struct {
    rtos_spi_master_device_t *wifi_device_ctx;
    rtos_gpio_t *gpio_ctx;
} wifi_test_devs_t;

static void wifi_test_task(wifi_test_devs_t *wifi_devs)
{
    const rtos_gpio_port_id_t wifi_wup_rst_port = rtos_gpio_port(WIFI_WUP_RST_N);

    rtos_spi_master_device_t *wifi_device_ctx = wifi_devs->wifi_device_ctx;
    rtos_gpio_t *gpio_ctx = wifi_devs->gpio_ctx;

//    rtos_gpio_port_enable(gpio_ctx, wifi_wup_rst_port);
//    rtos_gpio_port_out(gpio_ctx, wifi_wup_rst_port, 3); /* set bits 0 (WUP) and 1 (RST) high. */

    vTaskDelay(pdMS_TO_TICKS(100));

    uint8_t config_read_cmd[2] = {0x80, 0x02}; /* read 2 16-bit words from address 0 */
    uint8_t config_write_cmd[2] = {0x00, 0x02}; /* read 2 16-bit words from address 0 */
    uint8_t config_reg[4];

    // byte_number: 03_02_01_00
    // ar index #:  00 01 02 03
    // on wire:     01 00 03 02
    //              02 03 00 01

//    for (int i = 0; i < 2; i++) {
//        rtos_spi_master_transaction_start(wifi_device_ctx);
//        rtos_spi_master_transfer(wifi_device_ctx, config_read_cmd, NULL, 2);
//        rtos_spi_master_transfer(wifi_device_ctx, NULL, config_reg, 4);
//        rtos_spi_master_transaction_end(wifi_device_ctx);
//        rtos_printf("Config reg: 0x%02x%02x%02x%02x\n", config_reg[2], config_reg[3], config_reg[0], config_reg[1]);
//    }
//
//    rtos_spi_master_transaction_start(wifi_device_ctx);
//    rtos_spi_master_transfer(wifi_device_ctx, config_write_cmd, NULL, 2);
//    config_reg[3] = 0x04;
//    rtos_spi_master_transfer(wifi_device_ctx, config_reg, NULL, 4);
//    rtos_spi_master_transaction_end(wifi_device_ctx);
//
//    for (int i = 0; i < 2; i++) {
//        rtos_spi_master_transaction_start(wifi_device_ctx);
//        rtos_spi_master_transfer(wifi_device_ctx, config_read_cmd, NULL, 2);
//        rtos_spi_master_transfer(wifi_device_ctx, NULL, config_reg, 4);
//        rtos_spi_master_transaction_end(wifi_device_ctx);
//        rtos_printf("Config reg: 0x%02x%02x%02x%02x\n", config_reg[2], config_reg[3], config_reg[0], config_reg[1]);
//    }

    sl_wfx_host_set_hif(wifi_device_ctx,
                        gpio_ctx,
                        rtos_gpio_port_1I, 0, /* IRQ */
                        rtos_gpio_port_4E, 0, /* WUP */
                        rtos_gpio_port_4E, 1 ); /* RST */

    WIFI_On();

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
            rtos_printf("\tSecurity: %s\n", scan_result->xSecurity == eWiFiSecurityOpen ? "open" :
                    scan_result->xSecurity == eWiFiSecurityWEP ? "WEP" : "WPA");
            rtos_printf("\tChannel: %d\n", (int) scan_result->cChannel);
            rtos_printf("\tStrength: %d dBm\n\n", (int) scan_result->cRSSI);

            scan_result++;
        }
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
