// Copyright (c) 2019, XMOS Ltd, All rights reserved

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
#include "sl_wfx.h"

/* App headers */
//#include "foo.h"

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

static void wf200_test(void *arg)
{
    soc_peripheral_t spi_dev = arg;
    sl_wfx_context_t wfx_ctx;
    sl_status_t ret;

    void sl_wfx_host_set_spi_device(soc_peripheral_t dev);

    rtos_printf("Hello from wf200 test... ");

    vTaskDelay(pdMS_TO_TICKS(5000));

    rtos_printf("GO!\n");

    sl_wfx_host_set_spi_device(spi_dev);
    ret = sl_wfx_init(&wfx_ctx);
    rtos_printf("Returned %d\n", ret);

    while (1) {
        rtos_printf("loop\n");

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

    xTaskCreate(wf200_test, "wf200_test", portTASK_STACK_DEPTH(wf200_test), dev, 15, NULL);

    /* Initialize FreeRTOS IP*/
    //initalize_FreeRTOS_IP();

    vTaskStartScheduler();
}


void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
    //configASSERT(0);
}
