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
#include "gpio_driver.h"
#include "sl_wfx.h"
#include "sl_wfx_host.h"

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
    soc_peripheral_t gpio_dev = arg;
    soc_peripheral_t spi_dev = soc_peripheral_app_data(gpio_dev);
    sl_wfx_context_t wfx_ctx;
    sl_status_t ret;

    rtos_printf("Hello from wf200 test... ");

    vTaskDelay(pdMS_TO_TICKS(5000));

    rtos_printf("GO!\n");

    sl_wfx_host_set_hif(spi_dev,
                        gpio_dev,
                        gpio_1I, 0,  /* header pin 9 */
                        gpio_1P, 0); /* header pin 10 */

    ret = sl_wfx_init(&wfx_ctx);
    rtos_printf("Returned %x\n", ret);

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
            dev,
            0);

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
