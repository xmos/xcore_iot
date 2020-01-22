// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

//#include "FreeRTOS_IP.h"
//#include "FreeRTOS_Sockets.h"
//#include "FreeRTOS_DHCP.h"

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "spi_master_driver.h"
#include "gpio_driver.h"

/* App headers */
#include "sl_wfx_iot_wifi.h"

#if 0
eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase,
                                            uint32_t ulIPAddress )
{
    debug_printf("DHCP phase %d\n", eDHCPPhase);
    if (eDHCPPhase == eDHCPPhasePreRequest) {
        ulIPAddress = FreeRTOS_ntohl(ulIPAddress);
        debug_printf("%d.%d.%d.%d\n", (ulIPAddress >> 24) & 0xff, (ulIPAddress >> 16) & 0xff, (ulIPAddress >> 8) & 0xff, (ulIPAddress >> 0) & 0xff);

    }

    return eDHCPContinue;
}
#endif

char *security_name(WIFISecurity_t s)
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

    rtos_printf("Hello from wf200 test\n ");

    ret = WIFI_On();

    rtos_printf("Returned %x\n", ret);

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

    WIFINetworkParams_t pxNetworkParams;

    pxNetworkParams.pcSSID = "xxxxxxxx";
    pxNetworkParams.ucSSIDLength = strlen(pxNetworkParams.pcSSID);
    pxNetworkParams.pcPassword = "xxxxxxxx";
    pxNetworkParams.ucPasswordLength = strlen(pxNetworkParams.pcPassword);
    pxNetworkParams.xSecurity = eWiFiSecurityWPA;
    pxNetworkParams.cChannel = 0;

    ret = WIFI_ConnectAP(&pxNetworkParams);
    rtos_printf("Connect returned %x\n", ret);

    while (1) {
        //rtos_printf("loop\n");

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void soc_tile0_main(
        int tile)
{
    rtos_printf("Hello from tile %d\n", tile);

    soc_peripheral_t dev;

    dev = spi_master_driver_init(
            BITSTREAM_SPI_DEVICE_A,  /* Initializing SPI device A */
            2,                       /* Use 2 DMA buffers for the scatter/gather */
            0);                      /* This device's interrupts should happen on core 0 */

    dev = gpio_driver_init(
            BITSTREAM_GPIO_DEVICE_A,
            NULL,
            0);

    xTaskCreate(wf200_test, "wf200_test", portTASK_STACK_DEPTH(wf200_test), NULL, 15, NULL);

    /* Initialize FreeRTOS IP*/
    //initalize_FreeRTOS_IP();

    vTaskStartScheduler();
}


void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
    //configASSERT(0);
}
